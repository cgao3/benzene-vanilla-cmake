//----------------------------------------------------------------------------
/** @file WolvePlayer.cpp */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "WolveTimeControl.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

/** @page wolvetimecontrol Wolve Time Control
    We use a simple formula for determining the time to use for a
    move.
    The time remaining is divided by some constant @f$ c @f$
    (determined experimentally). Let @f$ k @f$ be the average number
    of moves in a game (also determined experimentally). Then the
    fraction of total time remaining after @f$ k @f$ moves will be @f$
    ((c-1)/c)^k @f$ on average. Because Wolve will abort early when it
    detects it cannot complete the next iteration in the time alloted,
    this is an under estimate on how much time will be left after @f$
    k @f$ moves.  Setting @f$ c @f$ so the above equation gives values
    around 0.20 results in about 2/3 of the total time being used and
    the strongest performance.
    @todo This strategy allocates more time to moves early in the
    game, which may not be the best strategy, especially while running
    the dfpn solve in parallel. Try allocating more time in the endgame.
    @todo This approach can result in lengthy searches at the begining
    that are somewhat useless. Cap early search times?
    @todo Try to estimate length of game more accurately? Say, by the
    percentage of empty cells on the board? 
*/

//----------------------------------------------------------------------------

double WolveTimeControl::TimeForMove(const Game& game, double timeLeft)
{
    const StoneBoard& brd = game.Board();
    double numMovesRemaining = 10.;
    if (brd.Width() == 9)
        // In practice, the average number of moves per game generated
        // by a search is around 8.5. The last couple moves are
        // typcially wins/losses found by previous searches, so there are
        // probably around 7 moves per game on average. Assuming there
        // are always 5 moves left uses 2/3 +- 1/6 of the time on average,
        // and seems to be the strongest (assuming 4 moves uses more time
        // but plays weaker for some reason).        
        numMovesRemaining = 5.;
    else if (brd.Width() == 11)
        // On average 15 moves/game. (9/10)^15 ~= 0.206.
        // TODO: Test!
        numMovesRemaining = 10.;
    else if (brd.Width() == 13)
        // On average 23 moves/game. (14/15)^23 ~= 0.205.
        // TODO: Test!
        numMovesRemaining = 15.;
    const double timeForMove = timeLeft / numMovesRemaining;
    LogInfo() << "timeLeft=" << timeLeft << ' '
              << "remaining=" << numMovesRemaining << ' '
              << "timeMove=" << timeForMove << '\n';
    return timeForMove;
}

//----------------------------------------------------------------------------

