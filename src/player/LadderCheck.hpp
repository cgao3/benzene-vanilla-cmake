//----------------------------------------------------------------------------
/** @file LadderCheck.hpp
 */
//----------------------------------------------------------------------------

#ifndef LADDERCHECK_HPP
#define LADDERCHECK_HPP

#include "BenzenePlayer.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Checks for bad ladder probes and removes them from the moves to 
    consider. */
class LadderCheck : public BenzenePlayerFunctionality
{
public:

    /** Adds pre-check for vulnerable cells to the given player. */
    LadderCheck(BenzenePlayer* player);

    /** Destructor. */
    virtual ~LadderCheck();

    /** Removes bad ladder probes from the set of moves to consider. */
    virtual HexPoint pre_search(HexBoard& brd, const Game& game_state,
				HexColor color, bitset_t& consider,
                                double max_time, double& score);

    bool Enabled() const;

    void SetEnabled(bool enable);

private:

    bool m_enabled;
};

inline bool LadderCheck::Enabled() const
{
    return m_enabled;
}
    
inline void LadderCheck::SetEnabled(bool enable)
{
    m_enabled = enable;
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // LADDERCHECK_HPP
