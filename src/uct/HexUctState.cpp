//----------------------------------------------------------------------------
/** @file HexUctState.cpp
 */
//----------------------------------------------------------------------------

#include "SgSystem.h"

#include "SgException.h"
#include "SgMove.h"

#include "BoardUtils.hpp"
#include "BitsetIterator.hpp"
#include "HexUctPolicy.hpp"
#include "HexUctUtil.hpp"
#include "PatternBoard.hpp"
#include "PlayerUtils.hpp"
#include "SequenceHash.hpp"

using namespace benzene;

/** Prints output when knowledge is computed. */
#define DEBUG_KNOWLEDGE 0

//----------------------------------------------------------------------------

namespace
{

/** Returns true if board is entirely filled. */
bool GameOver(const StoneBoard& brd)
{
    return brd.getEmpty().none();
}

/** Determines the winner of a filled-in board. */
HexColor GetWinner(const StoneBoard& brd)
{
    HexAssert(GameOver(brd));
    if (BoardUtils::ConnectedOnBitset(brd.Const(), brd.getColor(BLACK), 
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
    /** @todo Make Logger an std::ostream! */
    m_state.Dump(std::cerr);
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
      m_shared_data(0),
      m_search(sch),
      m_policy(0),
      m_treeUpdateRadius(treeUpdateRadius),
      m_playoutUpdateRadius(playoutUpdateRadius),
      m_isInPlayout(false)
{
}

HexUctState::~HexUctState()
{
    FreePolicy();
}

void HexUctState::FreePolicy()
{
    if (m_policy) 
        delete m_policy;
}
void HexUctState::SetPolicy(HexUctSearchPolicy* policy)
{
    FreePolicy();
    m_policy = policy;
}

void HexUctState::Dump(std::ostream& out) const
{
    out << "HexUctState[" << m_threadId << "] ";
    if (m_isInPlayout) out << "[playout]";
    out << "board:" << m_bd;
}

float HexUctState::Evaluate()
{
    LogFine() << "Evaluate()" << '\n';
    HexAssert(GameOver(*m_bd));
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
    ExecutePlainMove(move, m_treeUpdateRadius);
    m_tree_sequence.push_back(move);
    HexUctStoneData stones;
    if (m_shared_data->stones.get(SequenceHash::Hash(m_tree_sequence), stones))
    {
        m_bd->setColor(BLACK, stones.black);
        m_bd->setColor(WHITE, stones.white);
        m_bd->setPlayed(stones.played);
        m_bd->update();
    }
}

void HexUctState::ExecuteRolloutMove(HexPoint move)
{
    ExecutePlainMove(move, m_playoutUpdateRadius);
}

void HexUctState::ExecutePlainMove(HexPoint cell, int updateRadius)
{
    // Simply play a stone on the given cell.
    HexAssert(m_bd->isEmpty(cell));
    HexAssert(m_bd->updateRadius() == updateRadius);
    
    m_bd->playMove(m_toPlay, cell);
    if (updateRadius == 1)
	m_bd->updateRingGodel(cell);
    else
	m_bd->update(cell);
    
    m_numStonesPlayed++;
    m_lastMovePlayed = cell;
    m_new_game = false;
}

/** @todo Handle swap? */
bool HexUctState::GenerateAllMoves(std::size_t count, 
                                   std::vector<SgMoveInfo>& moves)
{
    HexAssert(m_new_game == (m_numStonesPlayed == 0));

    bitset_t moveset;
    bool have_consider_set = false;
    if (m_new_game)
    {
        moveset = m_shared_data->root_consider;
        have_consider_set = true;
    }
    else 
    {
        moveset = m_bd->getEmpty();
    }

    bool truncateChildTrees = false;
    if (count && !have_consider_set)
    {
        truncateChildTrees = true;

        m_vc_brd->SetState(*m_bd);
        m_vc_brd->ComputeAll(m_toPlay, HexBoard::DO_NOT_REMOVE_WINNING_FILLIN);

        // Fill board with winner's stones if a determined state
        if (PlayerUtils::IsDeterminedState(*m_vc_brd, m_toPlay))
        {
            moveset.reset();

            HexColor winner = m_toPlay;
            if (PlayerUtils::IsLostGame(*m_vc_brd, m_toPlay))
                winner = !m_toPlay;

            m_vc_brd->addColor(winner, m_vc_brd->getEmpty());
            // Add fillin to m_bd because Evaluate() will be called
            // immediately after SgUctSearch realizes this state has
            // no children.  This is necessary only for this tree
            // phase, subsequent tree phases will load up the fillin
            // during the ExecuteMove() needed to arrive at this
            // state.
            m_bd->addColor(winner, m_bd->getEmpty());

#if DEBUG_KNOWLEDGE
            LogInfo() << "Found win for " << winner << ": " << '\n' 
                      << *m_vc_brd << '\n';
#endif
        }
        // Otherwise, prune the moves to consider
        else
        {
            moveset &= PlayerUtils::MovesToConsider(*m_vc_brd, m_toPlay);
        }

        m_shared_data->stones.put(SequenceHash::Hash(m_tree_sequence), 
                                  HexUctStoneData(*m_vc_brd));
#if DEBUG_KNOWLEDGE
        LogInfo() << "===================================" << '\n'
                  << "Recomputed state:" << '\n' << *m_bd << '\n'
                  << "Mustplay:" << m_vc_brd->printBitset(moveset) << '\n';
#endif
    }

    moves.clear();
    for (BitsetIterator it(moveset); it; ++it)
        moves.push_back(SgMoveInfo(*it));

    return truncateChildTrees;
}

SgMove HexUctState::GeneratePlayoutMove(bool& skipRaveUpdate)
{
    skipRaveUpdate = false;
    
    if (GameOver(*m_bd))
        return SG_NULLMOVE;
        
    SgPoint move = m_policy->GenerateMove(*m_bd, m_toPlay, m_lastMovePlayed);
    HexAssert(move != SG_NULLMOVE);
    return move;
}

void HexUctState::StartSearch()
{
    LogInfo() << "StartSearch()[" << m_threadId <<"]" << '\n';
    m_shared_data = m_search.SharedData();

    // @todo Fix the interface to HexBoard so this can be constant!
    // The problem is that VCBuilder (which is inside of HexBoard)
    // expects a non-const reference to a VCBuilderParam object.
    HexBoard& brd = const_cast<HexBoard&>(m_search.Board());
    
    if (!m_bd.get() 
        || m_bd->width() != brd.width() 
        || m_bd->height() != brd.height())
    {
        m_bd.reset(new PatternBoard(brd.width(), brd.height()));
        m_vc_brd.reset(new HexBoard(brd.width(), brd.height(), 
                                    brd.ICE(), brd.Builder().Parameters()));
    }
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
    m_numStonesPlayed = 0;
    m_tree_sequence.clear();
    m_toPlay = m_shared_data->root_to_play;
    m_lastMovePlayed = m_shared_data->root_last_move_played;
    m_bd->setUpdateRadius(m_treeUpdateRadius);

    m_bd->startNewGame();
    m_bd->setColor(BLACK, m_shared_data->root_stones.black);
    m_bd->setColor(WHITE, m_shared_data->root_stones.white);
    m_bd->setPlayed(m_shared_data->root_stones.played);
    m_bd->update();
}

void HexUctState::StartPlayouts()
{
    m_isInPlayout = true;
    m_bd->setUpdateRadius(m_playoutUpdateRadius);
    
    /** Playout radius should normally be no bigger than tree radius,
	but if it is, we need to do an extra update for each playout
	during the transition from the tree phase to the playout
	phase. */
    if (m_playoutUpdateRadius > m_treeUpdateRadius)
	m_bd->update();
}

void HexUctState::StartPlayout()
{
    m_policy->InitializeForRollout(*m_bd);
}

void HexUctState::EndPlayout()
{
}

//----------------------------------------------------------------------------
