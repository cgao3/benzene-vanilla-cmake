//----------------------------------------------------------------------------
/** @file BenzenePlayer.cpp
 */
//----------------------------------------------------------------------------

#include "BenzenePlayer.hpp"
#include "PlayerUtils.hpp"
#include "BoardUtils.cpp"

//----------------------------------------------------------------------------

BenzenePlayer::BenzenePlayer()
    : HexPlayer()
{
}

BenzenePlayer::~BenzenePlayer()
{
}

//----------------------------------------------------------------------------

HexPoint BenzenePlayer::genmove(HexBoard& brd, 
                                const Game& game_state, HexColor color,
                                double time_remaining, double& score)
{
    HexPoint move = INVALID_POINT;
    bitset_t consider;

    move = init_search(brd, color, consider, time_remaining, score);
    if (move != INVALID_POINT)
        return move;

    //----------------------------------------------------------------------

    /** @bug Subtract time spent to here from time_remaining! */
    move = pre_search(brd, game_state, color, consider, time_remaining, score);
    if (move != INVALID_POINT) 
        return move;

    //----------------------------------------------------------------------

    LogInfo() << "Best move cannot be determined,"
             << " must search state." << '\n';
    /** @bug Subtract time spent to here from time_remaining! */
    move = search(brd, game_state, color, consider, time_remaining, score);

    //----------------------------------------------------------------------

    LogInfo() << "Applying post search heuristics..." << '\n';
    /** @bug Subtract time spent to here from time_remaining! */
    return post_search(move, brd, color, time_remaining, score);
}

HexPoint BenzenePlayer::init_search(HexBoard& brd, 
                                    HexColor color, 
                                    bitset_t& consider, 
                                    double time_remaining,
                                    double& score)
{
    UNUSED(time_remaining);
    
    // resign if the game is already over
    brd.absorb();
    if (brd.isGameOver()) {
        score = IMMEDIATE_LOSS;
        return RESIGN;
    }

    // compute vcs/ice and set moves to consider to all empty cells
    brd.ComputeAll(color, HexBoard::REMOVE_WINNING_FILLIN);
    consider = brd.getEmpty();
    score = 0;

    return INVALID_POINT;
}

HexPoint BenzenePlayer::pre_search(HexBoard& brd,
                                   const Game& game_state,
                                   HexColor color, 
                                   bitset_t& consider,
                                   double time_remaining,
                                   double& score)
{
    UNUSED(brd); 
    UNUSED(game_state);
    UNUSED(color);
    UNUSED(consider);
    UNUSED(time_remaining);
    UNUSED(score);
    return INVALID_POINT;
}

HexPoint BenzenePlayer::search(HexBoard& brd, 
                               const Game& game_state,
                               HexColor color, 
                               const bitset_t& consider,
                               double time_remaining, 
                               double& score)
{
    HexAssert(false); // Defense against Phil's stupidity

    UNUSED(game_state);
    UNUSED(color);
    UNUSED(consider);
    UNUSED(time_remaining);
    UNUSED(score);
    return BoardUtils::RandomEmptyCell(brd);
}
    
HexPoint BenzenePlayer::post_search(HexPoint move, 
                                    HexBoard& brd,
                                    HexColor color,
                                    double time_remaining,
                                    double& score)
{
    UNUSED(brd);
    UNUSED(color);
    UNUSED(time_remaining);
    UNUSED(score);
    return move;
}

//----------------------------------------------------------------------------

