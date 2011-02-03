//----------------------------------------------------------------------------
/** @file HexUctState.cpp

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
#include "HexUctPolicy.hpp"
#include "HexUctUtil.hpp"
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

} // namespace

//----------------------------------------------------------------------------

HexUctState::AssertionHandler::AssertionHandler(const HexUctState& state)
    : m_state(state)
{
}

void HexUctState::AssertionHandler::Run()
{
    LogSevere() << m_state.Dump() << '\n';
}

//----------------------------------------------------------------------------

HexUctState::HexUctState(const unsigned int threadId,
			 HexUctSearch& sch,
                         int treeUpdateRadius,
                         int playoutUpdateRadius)
    : SgUctThreadState(threadId, HexUctUtil::ComputeMaxNumMoves()),
      m_assertionHandler(*this),

      m_state(0),
      m_vc_brd(0),
      m_policy(0),
      m_shared_data(0),
      m_knowledge(*this),
      m_search(sch),
       m_treeUpdateRadius(treeUpdateRadius),
      m_playoutUpdateRadius(playoutUpdateRadius),
      m_isInPlayout(false)
{
}

HexUctState::~HexUctState()
{
}

void HexUctState::SetPolicy(HexUctSearchPolicy* policy)
{
    m_policy.reset(policy);
}

std::string HexUctState::Dump() const
{
    std::ostringstream os;
    os << "HexUctState[" << m_threadId << "] ";
    if (m_isInPlayout) 
        os << "[playout] ";
    os << "board:" << m_state->Position();
    return os.str();
}

SgUctValue HexUctState::Evaluate()
{
    const StoneBoard& pos = m_state->Position();
    SG_ASSERT(GameOver(pos));
    SgUctValue score = (GetWinner(pos) == m_state->ToPlay()) ? 1.0 : 0.0;
    return score;
}

void HexUctState::Execute(SgMove sgmove)
{
    ExecuteTreeMove(static_cast<HexPoint>(sgmove));
}

void HexUctState::ExecutePlayout(SgMove sgmove)
{
    ExecuteRolloutMove(static_cast<HexPoint>(sgmove));
}

void HexUctState::ExecuteTreeMove(HexPoint move)
{
    {
        HexUctPolicy* blah = dynamic_cast<HexUctPolicy*>(m_policy.get());
        if (!blah)
            abort();
        blah->AddResponse(m_state->ToPlay(), m_lastMovePlayed, move);
    }
    m_game_sequence.push_back(Move(m_state->ToPlay(), move));
    ExecutePlainMove(move, m_treeUpdateRadius);
    HexUctStoneData stones;
    if (m_shared_data->stones.Get(SequenceHash::Hash(m_game_sequence), stones))
    {
        StoneBoard& brd = m_state->Position();
        brd.StartNewGame();
        brd.SetColor(BLACK, stones.black);
        brd.SetColor(WHITE, stones.white);
        brd.SetPlayed(stones.played);
        m_pastate->Update();
    }
}

void HexUctState::ExecuteRolloutMove(HexPoint move)
{
    ExecutePlainMove(move, m_playoutUpdateRadius);
}

void HexUctState::ExecutePlainMove(HexPoint cell, int updateRadius)
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
    m_new_game = false;
}

bool HexUctState::GenerateAllMoves(SgUctValue count, 
                                   std::vector<SgUctMoveInfo>& moves,
                                   SgUctProvenType& provenType)
{
    moves.clear();

    // Handle root node as a special case
    if (m_new_game)
    {
        for (BitsetIterator it(m_shared_data->root_consider); it; ++it)
            moves.push_back(SgUctMoveInfo(*it));
        if (count == 0)
            m_knowledge.ProcessPosition(moves);
        return false;
    }

    bool truncateChildTrees = false;
    if (count == 0)
    {
        {
            HexColor winner = CheckIfWinner(m_state->Position());
            if (winner != EMPTY)
            {
                provenType = (winner == m_state->ToPlay()) 
                    ? SG_PROVEN_WIN : SG_PROVEN_LOSS;
                return false;
            }
        }
        for (BitsetIterator it(m_state->Position().GetEmpty()); it; ++it)
            moves.push_back(SgUctMoveInfo(*it));
        m_knowledge.ProcessPosition(moves);
    }
    else
    {
        // Prune moves outside of mustplay and fillin
        if (TRACK_KNOWLEDGE)
        {
            SgHashCode hash(SequenceHash::Hash(m_game_sequence));
            LogInfo() << m_threadId << ": " << hash << '\n';
        }
        truncateChildTrees = true;
        bitset_t moveset = m_state->Position().GetEmpty() 
            & ComputeKnowledge(provenType);
        for (BitsetIterator it(moveset); it; ++it)
            moves.push_back(SgUctMoveInfo(*it));
    }
    return truncateChildTrees;
}

SgMove HexUctState::GeneratePlayoutMove(bool& skipRaveUpdate)
{
    skipRaveUpdate = false;
    if (GameOver(m_state->Position()))
        return SG_NULLMOVE;
    SgPoint move = m_policy->GenerateMove(*m_pastate, m_state->ToPlay(),
                                          m_lastMovePlayed);
    SG_ASSERT(move != SG_NULLMOVE);
    return move;
}

void HexUctState::StartSearch()
{
    LogInfo() << "StartSearch()[" << m_threadId <<"]\n";
    m_shared_data = &m_search.SharedData();
    // TODO: Fix the interface to HexBoard so this can be constant!
    // The problem is that VCBuilder (which is inside of HexBoard)
    // expects a non-const reference to a VCBuilderParam object.
    HexBoard& brd = const_cast<HexBoard&>(m_search.Board());
    if (!m_state.get() 
        || m_state->Position().Width() != brd.Width() 
        || m_state->Position().Height() != brd.Height())
    {
        m_state.reset(new HexState(brd.GetPosition(), BLACK));
        m_pastate.reset(new PatternState(m_state->Position()));
        m_vc_brd.reset(new HexBoard(brd.Width(), brd.Height(), 
                                    brd.ICE(), brd.Builder().Parameters()));
    }
    m_policy->InitializeForSearch();
}

void HexUctState::TakeBackInTree(std::size_t nuMoves)
{
    SG_UNUSED(nuMoves);
}

void HexUctState::TakeBackPlayout(std::size_t nuMoves)
{
    SG_UNUSED(nuMoves);
}

SgBlackWhite HexUctState::ToPlay() const
{
    return HexUctUtil::ToSgBlackWhite(m_state->ToPlay());
}

void HexUctState::GameStart()
{
    m_new_game = true;
    m_isInPlayout = false;
    m_game_sequence = m_shared_data->game_sequence;
    m_lastMovePlayed = m_shared_data->root_last_move_played;
    m_pastate->SetUpdateRadius(m_treeUpdateRadius);
    m_state->Position().StartNewGame();
    m_state->Position().SetColor(BLACK, m_shared_data->root_stones.black);
    m_state->Position().SetColor(WHITE, m_shared_data->root_stones.white);
    m_state->Position().SetPlayed(m_shared_data->root_stones.played);
    m_state->SetToPlay(m_shared_data->root_to_play);
    m_pastate->Update();
}

void HexUctState::StartPlayouts()
{
    m_isInPlayout = true;
    m_pastate->SetUpdateRadius(m_playoutUpdateRadius);
    // Playout radius should normally be no bigger than tree radius,
    // but if it is, we need to do an extra update for each playout
    // during the transition from the tree phase to the playout phase.
    if (m_playoutUpdateRadius > m_treeUpdateRadius)
	m_pastate->Update();
}

void HexUctState::StartPlayout()
{
    m_policy->InitializeForRollout(m_state->Position());
}

void HexUctState::EndPlayout()
{
}

/** Computes moves to consider and stores fillin in the shared
    data. */
bitset_t HexUctState::ComputeKnowledge(SgUctProvenType& provenType)
{
    m_vc_brd->GetPosition().SetPosition(m_state->Position());
    m_vc_brd->ComputeAll(m_state->ToPlay());
    // Consider set will be all empty cells if state is a determined
    // state (can't compute consider set in this case and we cannot
    // delete the children as this will cause a race condition in the
    // parent class).
    // Consider set is the set of moves to consider otherwise.
    bitset_t consider;
    if (EndgameUtil::IsDeterminedState(*m_vc_brd, m_state->ToPlay()))
    {
        HexColor winner = m_state->ToPlay();
        provenType = SG_PROVEN_WIN;
        if (EndgameUtil::IsLostGame(*m_vc_brd, m_state->ToPlay()))
        {
            winner = !m_state->ToPlay();
            provenType = SG_PROVEN_LOSS;
        }
        if (DEBUG_KNOWLEDGE)
            LogInfo() << "Found win for " << winner << ":\n"
                      << *m_vc_brd << '\n';
        return m_state->Position().GetEmpty();
    }
    else
    {
        provenType = SG_NOT_PROVEN;
        consider = EndgameUtil::MovesToConsider(*m_vc_brd, m_state->ToPlay());
    }
    m_shared_data->stones.Put(SequenceHash::Hash(m_game_sequence), 
                              HexUctStoneData(m_vc_brd->GetPosition()));
    if (DEBUG_KNOWLEDGE)
        LogInfo() << "===================================\n"
                  << "Recomputed state:" << '\n' << m_state->Position() << '\n'
                  << "Consider:" << m_vc_brd->Write(consider) << '\n';
    return consider;
}
    
//----------------------------------------------------------------------------
