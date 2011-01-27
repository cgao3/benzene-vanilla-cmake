//----------------------------------------------------------------------------
/** @file WolvePlayer.cpp */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "WolveTimeControl.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

/** Determine time for move.
    Roughly determines the number of moves remaining in the game to
    estimate the time we should use for this move. To do so, the time
    remaining is divided by the average number of moves in a game
    (determined experimentally). This allots more time to moves early
    in the game, and nicely allows for unexpectedly long games.
    @todo This approach can result in lengthy searches at the begining
    that are somewhat useless. Cap early search times?
    @todo Try to estimate length of game more accurately? Say, by the
    percentage of empty cells on the board? */
double WolveTimeControl::TimeForMove(const Game& game, double timeLeft)
{
    const StoneBoard& brd = game.Board();
    double numMovesRemaining = 12.5;
    if (brd.Width() == 9)
        // In practice, the average number of moves per game generated
        // by a search is around 8.5. The last couple moves are
        // typcially wins/losses found by previous searches, so we
        // round down to about 7 important moves per game.
        numMovesRemaining = 7.;
    const double timeForMove = timeLeft / numMovesRemaining;
    LogInfo() << "timeLeft=" << timeLeft << ' '
              << "remaining=" << numMovesRemaining << ' '
              << "timeMove=" << timeForMove << '\n';
    return timeForMove;
}

//----------------------------------------------------------------------------

