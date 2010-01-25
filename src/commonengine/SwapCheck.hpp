//----------------------------------------------------------------------------
/** @file SwapCheck.hpp
 */
//----------------------------------------------------------------------------

#ifndef SWAPCHECK_HPP
#define SWAPCHECK_HPP

#include "Hex.hpp"
#include "Game.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Determines whether to swap in the current game state.

    @todo This functionality should be in the book.

    @note Initialization is NOT THREADSAFE!
*/
namespace SwapCheck
{
    /** If first move of game has been played and swap rule is being
	used, checks hardcoded swap rules and returns true if swap
	should be played. */
    bool PlaySwap(const Game& game, HexColor toPlay);
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // SWAPCHECK_HPP
