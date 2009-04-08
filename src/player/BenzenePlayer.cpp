//----------------------------------------------------------------------------
// $Id: BenzenePlayer.cpp 1994 2009-04-06 00:57:12Z broderic $
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
                                    double UNUSED(time_remaining), 
                                    double& score)
{
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

HexPoint BenzenePlayer::pre_search(HexBoard& UNUSED(brd), 
                                   const Game& UNUSED(game_state),
                                   HexColor UNUSED(color), 
                                   bitset_t& UNUSED(consider),
                                   double UNUSED(time_remaining), 
                                   double& UNUSED(score))
{
    return INVALID_POINT;
}

HexPoint BenzenePlayer::search(HexBoard& brd, 
                               const Game& UNUSED(game_state),
                               HexColor UNUSED(color), 
                               const bitset_t& UNUSED(consider),
                               double UNUSED(time_remaining), 
                               double& UNUSED(score))
{
    HexAssert(false); // Defense against Phil's stupidity
    return BoardUtils::RandomEmptyCell(brd);
}
    
HexPoint BenzenePlayer::post_search(HexPoint move, 
                                    HexBoard& UNUSED(brd), 
                                    HexColor UNUSED(color), 
                                    double UNUSED(time_remaining), 
                                    double& UNUSED(score))
{
    return move;
}

//----------------------------------------------------------------------------

