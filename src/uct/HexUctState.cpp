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

using namespace benzene;

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
			 const HexUctSearch& sch,
                         int treeUpdateRadius,
                         int playoutUpdateRadius)
    : SgUctThreadState(threadId, HexUctUtil::ComputeMaxNumMoves()),
      m_assertionHandler(*this),

      m_bd(0),
      m_initial_data(0),
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
    if (m_new_game)
    {
	// Play the first-ply cell and add its respective fill-in.
	HexAssert(m_numStonesPlayed == 0);
	HexAssert(m_toPlay == m_initial_data->root_to_play);
	
	if (m_bd->getEmpty() != m_bd->getCells())
	    m_bd->startNewGame();
	HexAssert(m_bd->getEmpty() == m_bd->getCells());
	PointToBitset::const_iterator it;
	it = m_initial_data->ply1_black_stones.find(move);
	HexAssert(it != m_initial_data->ply1_black_stones.end());
        m_bd->addColor(BLACK, it->second);
        it = m_initial_data->ply1_white_stones.find(move);
        HexAssert(it != m_initial_data->ply1_white_stones.end());
        m_bd->addColor(WHITE, it->second);
        m_bd->update();
	
        m_new_game = false;
	m_numStonesPlayed++;
	m_lastMovePlayed = move;
    }
    else
    {
	ExecutePlainMove(move, m_treeUpdateRadius);
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
}

/** @todo Handle swap? */
void HexUctState::GenerateAllMoves(std::vector<SgMove>& moves)
{
    HexAssert(m_new_game == (m_numStonesPlayed == 0));

    bitset_t moveset;
    if (m_new_game)
    {
        moveset = m_initial_data->ply1_moves_to_consider;
    }
    else if (m_numStonesPlayed == 1) 
    {
	// We're about to play the second stone...
	bitset_t opptMustplay;
	PointToBitset::const_iterator it 
	    = m_initial_data->ply2_moves_to_consider.find(m_lastMovePlayed);
	HexAssert(it != m_initial_data->ply2_moves_to_consider.end());
	opptMustplay = it->second;
        moveset = opptMustplay;
    } 
    else 
    {
        moveset = m_bd->getEmpty();
    }

    moves.clear();
    for (BitsetIterator it(moveset); it; ++it)
        moves.push_back(*it);
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
    m_initial_data = m_search.InitialData();
    const PatternBoard& brd = m_search.Board();
    
    if (!m_bd.get() 
        || m_bd->width() != brd.width() 
        || m_bd->height() != brd.height())
    {
        m_bd.reset(new PatternBoard(brd.width(), brd.height()));
    }
    m_bd->startNewGame();
    m_bd->setColor(BLACK, brd.getBlack());
    m_bd->setColor(WHITE, brd.getWhite());
    m_bd->setPlayed(brd.getPlayed());
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
    m_toPlay = m_initial_data->root_to_play;
    m_lastMovePlayed = m_initial_data->root_last_move_played;
    m_bd->setUpdateRadius(m_treeUpdateRadius);
    m_bd->startNewGame();
    
    if (!m_search.Tree().Root().HasChildren()) 
    {
	// If tree only consists of root node, then need to add root
	// position's stones to board. Otherwise we use pre-computed
	// 1-ply filled-in positions.
	HexAssert(m_bd->getEmpty() == m_bd->getCells());
	m_bd->addColor(BLACK, m_initial_data->root_black_stones);
	m_bd->addColor(WHITE, m_initial_data->root_white_stones);
        m_bd->update();
    }
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
