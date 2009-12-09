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
    virtual HexPoint PreSearch(HexBoard& brd, const Game& game_state,
                               HexColor color, bitset_t& consider,
                               double max_time, double& score);

private:
    bool m_swapLoaded;
    
    /** Contains moves to swap for each boardsize.  
        Use strings of the form "nxn" to index the map for an (n, n)
        board. */
    std::map<std::string, std::set<HexPoint> > m_swapMoves;

    BenzenePlayer* m_player;

    void LoadSwapMoves(const std::string& name);
};

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // SWAPCHECK_HPP
