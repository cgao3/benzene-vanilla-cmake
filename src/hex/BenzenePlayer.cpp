//----------------------------------------------------------------------------
/** @file BenzenePlayer.cpp */
//----------------------------------------------------------------------------

#include "BenzeneAssert.hpp"
#include "BenzenePlayer.hpp"
#include "EndgameUtil.hpp"
#include "BoardUtil.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

BenzenePlayer::BenzenePlayer()
    : HexPlayer(),
      m_search_singleton(false)
{
}

BenzenePlayer::~BenzenePlayer()
{
}

//----------------------------------------------------------------------------

/** @bug Subtract time spent to here from max_time after each step. */
HexPoint BenzenePlayer::GenMove(const HexState& state, const Game& game, 
                                HexBoard& brd, double maxTime, 
                                double& score)
{
    HexPoint move = INVALID_POINT;
    
    move = InitSearch(brd, state.ToPlay(), score);
    // Calls brd.ComputeAll(state.ToPlay())
    if (move != INVALID_POINT)
        return move;
        
    bitset_t consider;
    
    move = CheckEndgame(brd, state.ToPlay(), consider, score);
    // Have been removed from consider :
    // - the cells not in the vc-mustplay ;
    // - the cells inferior according to ComputeAll() ;
    // - the cells losing by strategy stealing argument ;
    // - the cells symetrical by rotation (if symetrical).
    if (move != INVALID_POINT) 
        return move;

    LogInfo() << "Best move cannot be determined, must search state.\n";
    return Search(state, game, brd, consider, maxTime, score);
}

/** Finds inferior cells, builds vcs. Sets moves to consider to all
    empty cells. If fillin causes terminal state, sets
    m_fillinCausedWin to true and recomputes fillin/vcs with ice
    temporarily turned off (so it can pass the players a non-empty
    consider set).
*/
HexPoint BenzenePlayer::InitSearch(HexBoard& brd, HexColor color,
				   double& score)
{
    // Resign if the game is already over
    Groups groups;
    GroupBuilder::Build(brd.GetPosition(), groups);
    if (groups.IsGameOver()) 
    {
        score = IMMEDIATE_LOSS;
        return RESIGN;
    }
    StoneBoard original(brd.GetPosition());
    brd.ComputeAll(color,INVALID_POINT); // the fillin is added to brd
    m_fillinCausedWin = false;
    m_fillinWinner = EMPTY;
    if (brd.GetGroups().IsGameOver()) 
    {
        // Fillin caused win, remove and re-compute without ice.
        m_fillinCausedWin = true;
        m_fillinWinner = brd.GetGroups().GetWinner();
        LogInfo() << "Fillin caused win! Removing...\n";
        brd.GetPosition().SetPosition(original);
        bool oldUseIce = brd.UseICE();
        brd.SetUseICE(false);
        brd.ComputeAll(color);
        brd.SetUseICE(oldUseIce);
        BenzeneAssert(!brd.GetGroups().IsGameOver());
    }
    score = 0;
    return INVALID_POINT;
}

HexPoint BenzenePlayer::CheckEndgame(HexBoard& brd, HexColor color, 
                                     bitset_t& consider, double& score)
{
    if (EndgameUtil::IsDeterminedState(brd, color, score)) 
        return EndgameUtil::PlayDeterminedState(brd, color);
    else 
    {
        consider = EndgameUtil::MovesToConsider(brd, color);
        BenzeneAssert(consider.any());
    }

    score = 0;
    if (consider.count() == 1 && !m_search_singleton) 
    {
        HexPoint move = static_cast<HexPoint>
            (BitsetUtil::FindSetBit(consider));
        LogInfo() << "Mustplay is singleton!\n";
        return move;
    }
    return INVALID_POINT;
}

//----------------------------------------------------------------------------

