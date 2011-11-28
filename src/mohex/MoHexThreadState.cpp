//----------------------------------------------------------------------------
/** @file MoHexThreadState.cpp

    @note Use SG_ASSERT so that the assertion handler is used to dump
    the state of each thread when an assertion fails.

    @bug Running with assertions and a non-zero knowledge threshold in
    lock-free mode will cause some assertions to fail. In particular,
    the way we handle terminal states (by deleting all children) can
    cause SgUctChildIterator to discover it has no children (in
    SgUctSearch::UpdateRaveValues and SgUctSearch::SelectChild) which
    it asserts is not true. It is also possible for threads to play
    into filled-in cells during the in-tree phase.
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "SgException.h"
#include "SgMove.h"

#include "BoardUtil.hpp"
#include "BitsetIterator.hpp"
#include "MoHexPlayoutPolicy.hpp"
#include "MoHexUtil.hpp"
#include "PatternState.hpp"
#include "EndgameUtil.hpp"
#include "SequenceHash.hpp"

using namespace benzene;

/** Prints output during knowledge computation. */
static const bool DEBUG_KNOWLEDGE = false;

/** Prints hash sequence before computing knowledge. 
    Use to see what threads are doing knowledge computations. */
static const bool TRACK_KNOWLEDGE = false;

//----------------------------------------------------------------------------

namespace
{

/** Returns true if board is entirely filled. */
bool GameOver(const StoneBoard& brd)
{
    return brd.GetEmpty().none();
}

/** Determines the winner of a filled-in board. */
HexColor GetWinner(const StoneBoard& brd)
{
    SG_ASSERT(GameOver(brd));
    if (BoardUtil::ConnectedOnBitset(brd.Const(), brd.GetColor(BLACK), 
                                     NORTH, SOUTH))
        return BLACK;
    return WHITE;
}

/** Returns winner if there is one. Returns EMPTY if no winner. */
HexColor CheckIfWinner(const StoneBoard& brd)
{
    if (BoardUtil::ConnectedOnBitset(brd.Const(), brd.GetColor(BLACK), 
                                     NORTH, SOUTH))
        return BLACK;
    if (BoardUtil::ConnectedOnBitset(brd.Const(), brd.GetColor(WHITE),
                                     EAST, WEST))
        return WHITE;
    return EMPTY;
}

/** Returns true if game is over and sets provenType appropriately. */
bool IsProvenState(const HexState& state, SgUctProvenType& provenType)
{
    HexColor winner = CheckIfWinner(state.Position());
    if (winner != EMPTY)
    {
        provenType = (winner == state.ToPlay())
            ? SG_PROVEN_WIN : SG_PROVEN_LOSS;
        return true;
    }
    return false;
}

/** Returns INVALID_POINT if history is empty, otherwise last move
    played to the board, ie, skips swap move. */
HexPoint LastMoveFromHistory(const MoveSequence& history)
{
    HexPoint lastMove = INVALID_POINT;
    if (!history.empty()) 
    {
	lastMove = history.back().Point();
	if (lastMove == SWAP_PIECES) 
        {
            BenzeneAssert(history.size() == 2);
            lastMove = history.front().Point();
	}
    }
    return lastMove;
}

//----------------------------------------------------------------------------

} // namespace

//----------------------------------------------------------------------------

MoHexThreadState::AssertionHandler::AssertionHandler
(const MoHexThreadState& state)
    : m_state(state)
{
}

void MoHexThreadState::AssertionHandler::Run()
{
    LogSevere() << m_state.Dump() << '\n';
}

//----------------------------------------------------------------------------

MoHexThreadState::MoHexThreadState(const unsigned int threadId,
                                   MoHexSearch& sch, int treeUpdateRadius,
                                   int playoutUpdateRadius)
    : SgUctThreadState(threadId, MoHexUtil::ComputeMaxNumMoves()),
      m_assertionHandler(*this),
      m_state(0),
      m_playoutStartState(0),
      m_vcBrd(0),
      m_policy(0),
      m_sharedData(0),
      m_priorKnowledge(*this),
      m_search(sch),
      m_treeUpdateRadius(treeUpdateRadius),
      m_playoutUpdateRadius(playoutUpdateRadius),
      m_isInPlayout(false)
{
}

MoHexThreadState::~MoHexThreadState()
{
}

void MoHexThreadState::SetPolicy(MoHexSearchPolicy* policy)
{
    m_policy.reset(policy);
}

std::string MoHexThreadState::Dump() const
{
    std::ostringstream os;
    os << "MoHexThreadState[" << m_threadId << "] ";
    if (m_isInPlayout) 
        os << "[playout] ";
    os << "board:" << m_state->Position();
    return os.str();
}

SgUctValue MoHexThreadState::Evaluate()
{
    const StoneBoard& pos = m_state->Position();
    SG_ASSERT(GameOver(pos));
    SgUctValue score = (GetWinner(pos) == m_state->ToPlay()) ? 1.0 : 0.0;
    return score;
}

void MoHexThreadState::Execute(SgMove sgmove)
{
    HexPoint move = static_cast<HexPoint>(sgmove);
    ExecuteMove(move, m_treeUpdateRadius);
    if (m_usingKnowledge)
    {
        m_gameSequence.push_back(Move(!m_state->ToPlay(), move));
        if(m_sharedData->stones.Get(SequenceHash::Hash(m_gameSequence), 
                                    m_state->Position()))
        {
            m_pastate->Update();
        }
    }
}

void MoHexThreadState::ExecutePlayout(SgMove sgmove)
{
    ExecuteMove(static_cast<HexPoint>(sgmove), m_playoutUpdateRadius);
}

void MoHexThreadState::ExecuteMove(HexPoint cell, int updateRadius)
{
    // Lock-free mode: It is possible we are playing into a filled-in
    // cell during the in-tree phase. This can occur if the thread
    // happens upon this state after fillin was published but before
    // the tree was pruned.
    //   If assertions are off, this results in a board possibly
    // containing cells of both colors and erroneous pattern state
    // info, resulting in an inaccurate playout value. In practice,
    // this does not seem to matter too much.
    //   If assertions are on, this will cause the search to abort
    // needlessly.
    // TODO: Handle case when assertions are on.
    SG_ASSERT(m_state->Position().IsEmpty(cell));
    SG_ASSERT(m_pastate->UpdateRadius() == updateRadius);
    m_state->PlayMove(cell);
    if (updateRadius == 1)
	m_pastate->UpdateRingGodel(cell);
    else
	m_pastate->Update(cell);
    m_lastMovePlayed = cell;
    m_atRoot = false;
}

bool MoHexThreadState::GenerateAllMoves(SgUctValue count, 
                                        std::vector<SgUctMoveInfo>& moves,
                                        SgUctProvenType& provenType)
{
    moves.clear();
    if (m_atRoot)
    {
        // Handle root node as a special case: using consider set
        // passed to us from MoHexPlayer.
        for (BitsetIterator it(m_sharedData->rootConsider); it; ++it)
            moves.push_back(SgUctMoveInfo(*it));
        if (count == 0)
            m_priorKnowledge.ProcessPosition(moves);
        return false;
    }
    else if (count == 0)
    {
        // First time we have been to this node. If solid winning
        // chain exists then mark as proven and abort. Otherwise, mark
        // every empty cell is a valid move.
        if (IsProvenState(*m_state, provenType))
            return false;
        for (BitsetIterator it(m_state->Position().GetEmpty()); it; ++it)
            moves.push_back(SgUctMoveInfo(*it));
        m_priorKnowledge.ProcessPosition(moves);
        return false;
    }
    else
    {
        // Re-visiting this state after a certain number of playouts.
        // If VC-win exists then mark as proven; otherwise, prune
        // moves outside of mustplay and store fillin. We must
        // truncate the child subtrees because of the fillin.
        BenzeneAssert(m_usingKnowledge);
        if (TRACK_KNOWLEDGE)
        {
            SgHashCode hash(SequenceHash::Hash(m_gameSequence));
            LogInfo() << m_threadId << ": " << hash << '\n';
        }
        bitset_t moveset = m_state->Position().GetEmpty() 
            & ComputeKnowledge(provenType);
        for (BitsetIterator it(moveset); it; ++it)
            moves.push_back(SgUctMoveInfo(*it));
        return true;
    }
    BenzeneAssert(false);
    return false;
}

SgMove MoHexThreadState::GeneratePlayoutMove(bool& skipRaveUpdate)
{
    skipRaveUpdate = false;
    if (GameOver(m_state->Position()))
        return SG_NULLMOVE;
    SgPoint move = m_policy->GenerateMove(*m_pastate, m_state->ToPlay(),
                                          m_lastMovePlayed);
    SG_ASSERT(move != SG_NULLMOVE);
    return move;
}

void MoHexThreadState::StartSearch()
{
    LogInfo() << "StartSearch()[" << m_threadId <<"]\n";
    m_usingKnowledge = !m_search.KnowledgeThreshold().empty();
    m_sharedData = &m_search.SharedData();
    // TODO: Fix the interface to HexBoard so this can be constant!
    // The problem is that VCBuilder (which is inside of HexBoard)
    // expects a non-const reference to a VCBuilderParam object.
    HexBoard& brd = const_cast<HexBoard&>(m_search.Board());
    if (!m_state.get() 
        || m_state->Position().Width() != brd.Width() 
        || m_state->Position().Height() != brd.Height())
    {
        m_state.reset(new HexState(brd.GetPosition(), BLACK));
        m_playoutStartState.reset(new HexState(brd.GetPosition(), BLACK));
        m_pastate.reset(new PatternState(m_state->Position()));
        m_playoutStartPatterns.reset
            (new PatternState(m_playoutStartState->Position()));
        m_vcBrd.reset(new HexBoard(brd.Width(), brd.Height(), 
                                   brd.ICE(), brd.Builder().Parameters()));
    }
    m_policy->InitializeForSearch();
}

void MoHexThreadState::TakeBackInTree(std::size_t nuMoves)
{
    SG_UNUSED(nuMoves);
}

void MoHexThreadState::TakeBackPlayout(std::size_t nuMoves)
{
    SG_UNUSED(nuMoves);
    if (m_search.NumberPlayouts() > 1)
    {
        m_lastMovePlayed = m_playoutStartLastMove;
        *m_state = *m_playoutStartState;
        m_pastate->CopyState(*m_playoutStartPatterns);
    }
}

SgBlackWhite MoHexThreadState::ToPlay() const
{
    return MoHexUtil::ToSgBlackWhite(m_state->ToPlay());
}

void MoHexThreadState::GameStart()
{
    m_atRoot = true;
    m_isInPlayout = false;
    m_gameSequence = m_sharedData->gameSequence;
    m_lastMovePlayed = LastMoveFromHistory(m_gameSequence);
    *m_state = m_sharedData->rootState;
    m_pastate->SetUpdateRadius(m_treeUpdateRadius);
    m_pastate->Update();
}

void MoHexThreadState::StartPlayouts()
{
    m_isInPlayout = true;
    m_pastate->SetUpdateRadius(m_playoutUpdateRadius);
    // Playout radius should normally be no bigger than tree radius,
    // but if it is, we need to do an extra update for each playout
    // during the transition from the tree phase to the playout phase.
    if (m_playoutUpdateRadius > m_treeUpdateRadius)
	m_pastate->Update();
    // If doing more than one playout make a backup of this state
    if (m_search.NumberPlayouts() > 1)
    {
        m_playoutStartLastMove = m_lastMovePlayed;
        *m_playoutStartState = *m_state;
        m_playoutStartPatterns->CopyState(*m_pastate);
    }
}

void MoHexThreadState::StartPlayout()
{
    m_policy->InitializeForPlayout(m_state->Position());
}

void MoHexThreadState::EndPlayout()
{
}

/** Computes moves to consider and stores fillin in the shared
    data. Sets provenType if state is determined by VCs. */
bitset_t MoHexThreadState::ComputeKnowledge(SgUctProvenType& provenType)
{
    m_vcBrd->GetPosition().SetPosition(m_state->Position());
    m_vcBrd->ComputeAll(m_state->ToPlay());
    // Consider set will be all empty cells if state is a determined
    // state (can't compute consider set in this case and we cannot
    // delete the children as this will cause a race condition in the
    // parent class).
    // Consider set is the set of moves to consider otherwise.
    bitset_t consider;
    if (EndgameUtil::IsDeterminedState(*m_vcBrd, m_state->ToPlay()))
    {
        HexColor winner = m_state->ToPlay();
        provenType = SG_PROVEN_WIN;
        if (EndgameUtil::IsLostGame(*m_vcBrd, m_state->ToPlay()))
        {
            winner = !m_state->ToPlay();
            provenType = SG_PROVEN_LOSS;
        }
        if (DEBUG_KNOWLEDGE)
            LogInfo() << "Found win for " << winner << ":\n"
                      << *m_vcBrd << '\n';
        return m_state->Position().GetEmpty();
    }
    else
    {
        provenType = SG_NOT_PROVEN;
        consider = EndgameUtil::MovesToConsider(*m_vcBrd, m_state->ToPlay());
    }
    m_sharedData->stones.Add(SequenceHash::Hash(m_gameSequence), 
                             m_vcBrd->GetPosition());
    if (DEBUG_KNOWLEDGE)
        LogInfo() << "===================================\n"
                  << "Recomputed state:" << '\n' << m_state->Position() << '\n'
                  << "Consider:" << m_vcBrd->Write(consider) << '\n';
    return consider;
}
    
//----------------------------------------------------------------------------
