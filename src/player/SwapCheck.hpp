//----------------------------------------------------------------------------
/** @file SwapCheck.hpp
 */
//----------------------------------------------------------------------------

#ifndef SWAPCHECK_HPP
#define SWAPCHECK_HPP

#include "BenzenePlayer.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Checks swap before search. */
class SwapCheck : public BenzenePlayerFunctionality
{
public:

    /** Adds pre-check for swap rule decision to the given player. */
    SwapCheck(BenzenePlayer* player);

    /** Destructor. */
    virtual ~SwapCheck();

    /** If first move of game has been played and swap rule is being used,
	determines whether or not to swap.
	Note: when does not swap, assumes player will search for a valid
	cell (i.e. non-swap) response.
    */
    virtual HexPoint pre_search(HexBoard& brd, const Game& game_state,
				HexColor color, bitset_t& consider,
                                double max_time, double& score);

private:
    BenzenePlayer* m_player;
};

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // SWAPCHECK_HPP
