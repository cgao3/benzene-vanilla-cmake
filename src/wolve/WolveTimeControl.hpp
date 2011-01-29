//----------------------------------------------------------------------------
/** @file WolveTimeControl.hpp */
//----------------------------------------------------------------------------

#ifndef WOLVETIMECONTROL_HPP
#define WOLVETIMECONTROL_HPP

#include "Game.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Time control. 
    See @ref wolvetimecontrol
*/
class WolveTimeControl
{
public:
    /** Determines time for move. 
        See @ref wolvetimecontrol */
    static double TimeForMove(const Game& game, double timeLeft);

private: 
};

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // WOLVETIMECONTROL_HPP
