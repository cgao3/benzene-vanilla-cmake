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

#include "BoardUtils.hpp"
#include "BitsetIterator.hpp"
#include "HexUctPolicy.hpp"
#include "HexUctUtil.hpp"
#include "PatternState.hpp"
#include "EndgameUtils.hpp"
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
    if (BoardUtils::ConnectedOnBitset(brd.Const(), brd.GetColor(BLACK), 
                                      NORTH, SOUTH))
        return BLACK;
    return WHITE;
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

HexUctState::HexUctState(std::size_t threadId,
			 HexUctSearch& sch,
                         int treeUpdateRadius,
                         int playoutUpdateRadius)
    : SgUctThreadState(threadId, HexUctUtil::ComputeMaxNumMoves()),
      m_assertionHandler(*this),

      m_bd(0),
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
    os << "board:" << *m_bd;
    return os.str();
}

float HexUctState::Evaluate()
{
    SG_ASSERT(GameOver(*m_bd));
    float score = (GetWinner(*m_bd) == m_toPlay) ? 1.0 : 0.0;
    return score;
}

void HexUctState::Execute(SgMove sgmove)
{
    ExecuteTreeMove(static_cast<HexPoint>(sgmove));
    m_toPlay = !m_toPlay;
}

void HexUctState::ExecutePlayout(SgMove sgmove)
{
    ExecuteRolloutMove(static_cast<HexPoint>(sgmove));
    m_toPlay = !m_toPlay;
}

void HexUctState::ExecuteTreeMove(HexPoint move)
{
    {
        HexUctPolicy* blah = dynamic_cast<HexUctPolicy*>(m_policy.get());
        if (!blah)
            abort();
        blah->AddResponse(m_toPlay, m_lastMovePlayed, move);
    }

    m_game_sequence.push_back(Move(m_toPlay, move));
    ExecutePlainMove(move, m_treeUpdateRadius);
    HexUctStoneData stones;
    if (m_shared_data->stones.get(SequenceHash::Hash(m_game_sequence), stones))
    {
        m_bd->StartNewGame();
        m_bd->SetColor(BLACK, stones.black);
        m_bd->SetColor(WHITE, stones.white);
        m_bd->SetPlayed(stones.played);
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
    // @todo Handle case when assertions are on.
    SG_ASSERT(m_bd->IsEmpty(cell));
    SG_ASSERT(m_pastate->UpdateRadius() == updateRadius);
    
    m_bd->PlayMove(m_toPlay, cell);
    if (updateRadius == 1)
	m_pastate->UpdateRingGodel(cell);
    else
	m_pastate->Update(cell);
    
    m_lastMovePlayed = cell;
    m_new_game = false;
}

bool HexUctState::GenerateAllMoves(std::size_t count, 
                                   std::vector<SgMoveInfo>& moves,
                                   SgProvenNodeType& provenType)
{
    moves.clear();

    // Handle root node as a special case
    if (m_new_game)
    {
        for (BitsetIterator it(m_shared_data->root_consider); it; ++it)
            moves.push_back(SgMoveInfo(*it));
        if (count == 0)
            m_knowledge.ProcessPosition(moves);
        return false;
    }

    bool truncateChildTrees = false;
    if (count == 0)
    {
        // First time at node: use empty cells and prior knowledge
        for (BitsetIterator it(m_bd->GetEmpty()); it; ++it)
            moves.push_back(SgMoveInfo(*it));
        m_knowledge.ProcessPosition(moves);
    }
    else
    {
        // Prune moves outside of mustplay and fillin
        if (TRACK_KNOWLEDGE)
        {
            hash_t hash = SequenceHash::Hash(m_game_sequence);
            LogInfo() << m_threadId << ": " 
                      << HashUtil::toString(hash) << '\n';
        }
        truncateChildTrees = true;
        bitset_t moveset = m_bd->GetEmpty() & ComputeKnowledge(provenType);
        for (BitsetIterator it(moveset); it; ++it)
            moves.push_back(SgMoveInfo(*it));
    }
    return truncateChildTrees;
}

SgMove HexUctState::GeneratePlayoutMove(bool& skipRaveUpdate)
{
    skipRaveUpdate = false;
    
    if (GameOver(*m_bd))
        return SG_NULLMOVE;
        
    SgPoint move = m_policy->GenerateMove(*m_pastate, m_toPlay,
                                          m_lastMovePlayed);
    SG_ASSERT(move != SG_NULLMOVE);
    return move;
}

void HexUctState::StartSearch()
{
    LogInfo() << "StartSearch()[" << m_threadId <<"]" << '\n';
    m_shared_data = &m_search.SharedData();

    // @todo Fix the interface to HexBoard so this can be constant!
    // The problem is that VCBuilder (which is inside of HexBoard)
    // expects a non-const reference to a VCBuilderParam object.
    HexBoard& brd = const_cast<HexBoard&>(m_search.Board());
    
    if (!m_bd.get() 
        || m_bd->Width() != brd.Width() 
        || m_bd->Height() != brd.Height())
    {
        m_bd.reset(new StoneBoard(brd.Width(), brd.Height()));
        m_pastate.reset(new PatternState(*m_bd));
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
    return HexUctUtil::ToSgBlackWhite(m_toPlay);
}

void HexUctState::GameStart()
{
    m_new_game = true;
    m_isInPlayout = false;
    m_game_sequence = m_shared_data->game_sequence;
    m_toPlay = m_shared_data->root_to_play;
    m_lastMovePlayed = m_shared_data->root_last_move_played;
    m_pastate->SetUpdateRadius(m_treeUpdateRadius);

    m_bd->StartNewGame();
    m_bd->SetColor(BLACK, m_shared_data->root_stones.black);
    m_bd->SetColor(WHITE, m_shared_data->root_stones.white);
    m_bd->SetPlayed(m_shared_data->root_stones.played);
    m_pastate->Update();
}

void HexUctState::StartPlayouts()
{
    m_isInPlayout = true;
    m_pastate->SetUpdateRadius(m_playoutUpdateRadius);
    
    /** Playout radius should normally be no bigger than tree radius,
	but if it is, we need to do an extra update for each playout
	during the transition from the tree phase to the playout
	phase. */
    if (m_playoutUpdateRadius > m_treeUpdateRadius)
	m_pastate->Update();
}

void HexUctState::StartPlayout()
{
    m_policy->InitializeForRollout(*m_bd);
}

void HexUctState::EndPlayout()
{
}

/** Computes moves to consider and stores fillin in the shared
    data. */
bitset_t HexUctState::ComputeKnowledge(SgProvenNodeType& provenType)
{
    m_vc_brd->GetPosition().StartNewGame();
    m_vc_brd->GetPosition().SetColor(BLACK, m_bd->GetPlayed(BLACK));
    m_vc_brd->GetPosition().SetColor(WHITE, m_bd->GetPlayed(WHITE));
    m_vc_brd->GetPosition().SetPlayed(m_bd->GetPlayed());
    m_vc_brd->ComputeAll(m_toPlay);

    // Consider set will be all empty cells if state is a determined
    // state (can't compute consider set in this case and we cannot
    // delete the children as this will cause a race condition in the
    // parent class).
    //
    // Consider set is the set of moves to consider otherwise.
    bitset_t consider;
    if (EndgameUtils::IsDeterminedState(*m_vc_brd, m_toPlay))
    {
        HexColor winner = m_toPlay;
        provenType = SG_PROVEN_WIN;
        if (EndgameUtils::IsLostGame(*m_vc_brd, m_toPlay))
        {
            winner = !m_toPlay;
            provenType = SG_PROVEN_LOSS;
        }
        if (DEBUG_KNOWLEDGE)
            LogInfo() << "Found win for " << winner << ": " << '\n' 
                      << *m_vc_brd << '\n';
        return m_bd->GetEmpty();
    }
    else
    {
        provenType = SG_NOT_PROVEN;
        consider = EndgameUtils::MovesToConsider(*m_vc_brd, m_toPlay);
    }

    m_shared_data->stones.put(SequenceHash::Hash(m_game_sequence), 
                              HexUctStoneData(m_vc_brd->GetPosition()));
    if (DEBUG_KNOWLEDGE)
        LogInfo() << "===================================" << '\n'
                  << "Recomputed state:" << '\n' << *m_bd << '\n'
                  << "Consider:" << m_vc_brd->Write(consider) << '\n';

    return consider;
}
    
//----------------------------------------------------------------------------
