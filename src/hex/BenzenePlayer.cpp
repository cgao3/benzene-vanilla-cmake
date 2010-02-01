//----------------------------------------------------------------------------
/** @file BenzenePlayer.cpp
 */
//----------------------------------------------------------------------------

#include "BenzenePlayer.hpp"
#include "EndgameUtils.hpp"
#include "BoardUtils.cpp"

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
HexPoint BenzenePlayer::GenMove(HexBoard& brd, const Game& game_state, 
                                HexColor color, double max_time, double& score)
{
    HexPoint move = INVALID_POINT;
    bitset_t consider;

    move = InitSearch(brd, color, consider, score);
    if (move != INVALID_POINT)
        return move;

    move = CheckEndgame(brd, color, consider, score);
    if (move != INVALID_POINT) 
        return move;

    LogInfo() << "Best move cannot be determined, must search state.\n";
    return Search(brd, game_state, color, consider, max_time, score);
}

/** Finds inferior cells, builds vcs. Sets moves to consider to all
    empty cells. If fillin causes terminal state, recomputes
    fillin/vcs with ice temporarily turned off.
    @param brd
    @param color
    @param consider
    @param score
    @return INVALID_POINT if a non-terminal state, otherwise the
    move to play in the terminal state.
*/
HexPoint BenzenePlayer::InitSearch(HexBoard& brd, HexColor color, 
                                   bitset_t& consider, double& score)
{
    // Resign if the game is already over
    Groups groups;
    GroupBuilder::Build(brd.GetState(), groups);
    if (groups.IsGameOver()) 
    {
        score = IMMEDIATE_LOSS;
        return RESIGN;
    }

    StoneBoard original(brd.GetState());
    brd.ComputeAll(color);

    // If fillin causes win, remove and re-compute without ice.
    if (brd.GetGroups().IsGameOver()) 
    {
        LogInfo() << "Captured cells caused win! Removing...\n";
        brd.GetState().SetState(original);
        bool oldUseIce = brd.UseICE();
        brd.SetUseICE(false);
        brd.ComputeAll(color);
        brd.SetUseICE(oldUseIce);
        HexAssert(!brd.GetGroups().IsGameOver());
    } 

    consider = brd.GetState().GetEmpty();
    score = 0;

    return INVALID_POINT;
}


HexPoint BenzenePlayer::CheckEndgame(HexBoard& brd, HexColor color, 
                                     bitset_t& consider, double& score)
{
    if (EndgameUtils::IsDeterminedState(brd, color, score)) 
        return EndgameUtils::PlayDeterminedState(brd, color);
    else 
    {
        consider = EndgameUtils::MovesToConsider(brd, color);
        HexAssert(consider.any());
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

