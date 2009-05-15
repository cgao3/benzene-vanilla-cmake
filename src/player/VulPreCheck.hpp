
#ifndef VULPRECHECK_HPP
#define VULPRECHECK_HPP

#include "BenzenePlayer.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Checks book before search. */
class VulPreCheck : public BenzenePlayerFunctionality
{
public:

    /** Adds pre-check for vulnerable cells to the given player. */
    VulPreCheck(BenzenePlayer* player);

    /** Destructor. */
    virtual ~VulPreCheck();

    /** Checks to see if the last move played by the opponent is
	vulnerable. If so, returns the killing move; otherwise,
	returns INVALID_POINT.
    */
    virtual HexPoint pre_search(HexBoard& brd, const Game& game_state,
				HexColor color, bitset_t& consider,
                                double max_time, double& score);

private:

    /** Member variable to track which oppt stones we've killed before.
	Helps to identify more vulnerable patterns. */
    bitset_t m_killedOpptStones;
};

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // VULPRECHECK_HPP
