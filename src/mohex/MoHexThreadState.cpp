//----------------------------------------------------------------------------
/** @file MoHexThreadState.cpp

    @note Use SG_ASSERT so that the assertion handler is used to dump
    the state of each thread when an assertion fails.

    @bug Running with assertions, and a non-zero knowledge threshold
    in lock-free mode will cause some assertions to fail: it is
    possible for threads to play into filled-in cells during the
    in-tree phase. */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "SgException.h"
#include "SgMove.h"

#include "BoardUtil.hpp"
#include "BitsetIterator.hpp"
#include "MoHexSearch.hpp"
#include "MoHexThreadState.hpp"
#include "MoHexUtil.hpp"
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
                                   MoHexSearch& sch, 
                                   MoHexSharedPolicy* sharedPolicy)
    : SgUctThreadState(threadId, MoHexUtil::ComputeMaxNumMoves()),
      m_assertionHandler(*this),
      m_state(0),
      m_playoutStartState(0),
      m_vcBrd(0),
      m_policy(sharedPolicy, sch.LocalPatterns()),
      m_sharedData(0),
      m_priorKnowledge(*this),
      m_search(sch),
      m_isInPlayout(false)
{
}

MoHexThreadState::~MoHexThreadState()
{
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
    ExecuteMove(move);
    if (m_usingKnowledge)
    {
        MoHexSharedData::StateData data;
        if(m_sharedData->stateData.Get(m_state->Hash(), data))
        {
            m_state->Position() = data.position;
        }
    }
}

void MoHexThreadState::ExecutePlayout(SgMove sgmove)
{
    m_policy.PlayMove(static_cast<HexPoint>(sgmove), m_state->ToPlay());
    ExecuteMove(static_cast<HexPoint>(sgmove));
}

void MoHexThreadState::ExecuteMove(HexPoint cell)
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
    m_state->PlayMove(cell);
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
    else if (count <= 0)
    {
        // First time we have been to this node. If solid winning
        // chain exists then mark as proven and abort. Otherwise,
        // every empty cell is a valid move.
        if (IsProvenState(*m_state, provenType))
            return false;
        for (BitsetIterator it(m_state->Position().GetEmpty()); it; ++it)
            moves.push_back(SgUctMoveInfo(*it));
        // If count is negative, then we are not actually expanding
        // this node, so do not compute prior knowledge.
        if (count == 0)
            m_priorKnowledge.ProcessPosition(moves);
        return false;
    }
    else
    {
        // Re-visiting this state after a certain number of playouts.
        // If VC-win exists then mark as proven; otherwise, prune
        // moves outside of mustplay and store fillin. We must
        // truncate the child subtrees because of the fillin if lazy
        // delete is not on.
        BenzeneAssert(m_usingKnowledge);
        bitset_t moveset = m_state->Position().GetEmpty() 
            & ComputeKnowledge(provenType);
        for (BitsetIterator it(moveset); it; ++it)
            moves.push_back(SgUctMoveInfo(*it));
        // Truncate tree only if not using lazy delete
        return !m_search.LazyDelete();
    }
    BenzeneAssert(false);
    return false;
}

SgMove MoHexThreadState::GeneratePlayoutMove(bool& skipRaveUpdate)
{
    skipRaveUpdate = false;
    if (GameOver(m_state->Position()))
        return SG_NULLMOVE;
    SgPoint move = m_policy.GenerateMove(*m_state, m_lastMovePlayed);
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
        m_vcBrd.reset(new HexBoard(brd.Width(), brd.Height(), 
                                   brd.ICE(), brd.VCBuilderParameters()));
    }
    m_policy.InitializeForSearch();
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
    }
}

SgBlackWhite MoHexThreadState::ToPlay() const
{
    return MoHexUtil::ToSgBlackWhite(m_state->ToPlay());
}

bool MoHexThreadState::IsValidMove(SgMove move)
{
    return m_state->Position().IsEmpty(static_cast<HexPoint>(move));
}

void MoHexThreadState::GameStart()
{
    m_atRoot = true;
    m_isInPlayout = false;
    m_lastMovePlayed = LastMoveFromHistory(m_sharedData->gameSequence);
    *m_state = m_sharedData->rootState;
}

void MoHexThreadState::StartPlayouts()
{
    m_isInPlayout = true;
    // If doing more than one playout make a backup of this state
    if (m_search.NumberPlayouts() > 1)
    {
        m_playoutStartLastMove = m_lastMovePlayed;
        *m_playoutStartState = *m_state;
    }
}

void MoHexThreadState::StartPlayout()
{
    m_policy.InitializeForPlayout(m_state->Position());
}

void MoHexThreadState::StartPlayout(const HexState& state,
                                    HexPoint lastMovePlayed)
{
    const StoneBoard& brd = state.Position();
    if (!m_state.get() 
        || m_state->Position().Width() != brd.Width() 
        || m_state->Position().Height() != brd.Height())
    {
        m_state.reset(new HexState(state));
    }
    *m_state = state;
    m_lastMovePlayed = lastMovePlayed;
    StartPlayout();
}

void MoHexThreadState::EndPlayout()
{
}

/** Computes moves to consider and stores fillin in the shared
    data. Sets provenType if state is determined by VCs. */
bitset_t MoHexThreadState::ComputeKnowledge(SgUctProvenType& provenType)
{
    provenType = SG_NOT_PROVEN;
    SgHashCode hash = m_state->Hash();
    MoHexSharedData::StateData data;
    if (m_sharedData->stateData.Get(hash, data))
    {
        if (TRACK_KNOWLEDGE)
            LogInfo() << "cached: " << hash << '\n';
        m_state->Position() = data.position;
        return data.consider;
    }
    if (TRACK_KNOWLEDGE)
        LogInfo() << "know: " << hash << '\n';
    m_vcBrd->GetPosition().SetPosition(m_state->Position());
    m_vcBrd->ComputeAll(m_state->ToPlay());
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
        // Set the consider set to be all empty cells: doesn't really
        // matter since we are marking it as a proven node, so the
        // search will never decend past this node again.
        return m_state->Position().GetEmpty();
    }
    data.consider = EndgameUtil::MovesToConsider(*m_vcBrd, m_state->ToPlay());
    data.position = m_vcBrd->GetPosition();
    m_sharedData->stateData.Add(m_state->Hash(), data);

    m_state->Position() = data.position;

    if (DEBUG_KNOWLEDGE)
        LogInfo() << "===================================\n"
                  << "Recomputed state:" << '\n' << data.position << '\n'
                  << "Consider:" << data.position.Write(data.consider) << '\n';
    return data.consider;
}
    
//----------------------------------------------------------------------------
