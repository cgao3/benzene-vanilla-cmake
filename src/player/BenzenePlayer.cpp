//----------------------------------------------------------------------------
/** @file BenzenePlayer.cpp
 */
//----------------------------------------------------------------------------

#include "BenzenePlayer.hpp"
#include "PlayerUtils.hpp"
#include "BoardUtils.cpp"

using namespace benzene;

//----------------------------------------------------------------------------

BenzenePlayer::BenzenePlayer()
    : HexPlayer()
{
}

BenzenePlayer::~BenzenePlayer()
{
}

//----------------------------------------------------------------------------

/** @bug Subtract time spent to here from max_time after each step. */
HexPoint BenzenePlayer::genmove(HexBoard& brd, const Game& game_state, 
                                HexColor color, double max_time, double& score)
{
    HexPoint move = INVALID_POINT;
    bitset_t consider;

    move = init_search(brd, color, consider, score);
    if (move != INVALID_POINT)
        return move;

    //----------------------------------------------------------------------

    move = pre_search(brd, game_state, color, consider, max_time, score);
    if (move != INVALID_POINT) 
        return move;

    //----------------------------------------------------------------------

    LogInfo() << "Best move cannot be determined, must search state.\n";
    move = search(brd, game_state, color, consider, max_time, score);

    //----------------------------------------------------------------------

    LogInfo() << "Applying post search heuristics...\n";
    return post_search(move, brd, color, max_time, score);
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
HexPoint BenzenePlayer::init_search(HexBoard& brd, HexColor color, 
                                    bitset_t& consider, double& score)
{
    // Resign if the game is already over
    brd.absorb();
    if (brd.isGameOver()) 
    {
        score = IMMEDIATE_LOSS;
        return RESIGN;
    }

    StoneBoard original(brd);
    brd.ComputeAll(color);

    // If fillin causes win, remove and re-compute without ice.
    if (brd.isGameOver()) 
    {
        LogFine() << "Captured cells caused win! Removing...\n";
        brd.SetState(original);
        bool oldUseIce = brd.UseICE();
        brd.SetUseICE(false);
        brd.ComputeAll(color);
        brd.SetUseICE(oldUseIce);
        HexAssert(!brd.isGameOver());
    } 

    consider = brd.getEmpty();
    score = 0;

    return INVALID_POINT;
}

HexPoint BenzenePlayer::pre_search(HexBoard& brd, const Game& game_state,
                                   HexColor color, bitset_t& consider,
                                   double max_time, double& score)
{
    UNUSED(brd); 
    UNUSED(game_state);
    UNUSED(color);
    UNUSED(consider);
    UNUSED(max_time);
    UNUSED(score);
    return INVALID_POINT;
}

HexPoint BenzenePlayer::search(HexBoard& brd, const Game& game_state,
                               HexColor color, const bitset_t& consider,
                               double max_time, double& score)
{
    HexAssert(false); // Defense against Phil's stupidity

    UNUSED(game_state);
    UNUSED(color);
    UNUSED(consider);
    UNUSED(max_time);
    UNUSED(score);
    return BoardUtils::RandomEmptyCell(brd);
}
    
HexPoint BenzenePlayer::post_search(HexPoint move, HexBoard& brd, 
                                    HexColor color, double max_time,
                                    double& score)
{
    UNUSED(brd);
    UNUSED(color);
    UNUSED(max_time);
    UNUSED(score);
    return move;
}

//----------------------------------------------------------------------------

