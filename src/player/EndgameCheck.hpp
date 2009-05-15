//----------------------------------------------------------------------------
/** @file EndgameCheck.hpp
 */
//----------------------------------------------------------------------------

#ifndef ENDGAMECHECK_HPP
#define ENDGAMECHECK_HPP

#include "BenzenePlayer.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Handles VC endgames and prunes the moves to consider to
    the set return by PlayerUtils::MovesToConsider(). 
*/
class EndgameCheck : public BenzenePlayerFunctionality
{
public:

    /** Extends the given player. */
    EndgameCheck(BenzenePlayer* player);

    /** Destructor. */
    virtual ~EndgameCheck();

    /** If PlayerUtils::IsDeterminedState() is true, returns
        PlayerUtils::PlayInDeterminedState().  Otherwise, returns
        INVALID_POINT and prunes consider by
        PlayerUtils::MovesToConsider().
     */
    virtual HexPoint pre_search(HexBoard& brd, const Game& game_state,
				HexColor color, bitset_t& consider,
                                double max_time, double& score);

    bool Enabled() const;

    void SetEnabled(bool enable);

    /** If only a single non-losing move, will search it anyway if
        this is true. Useful when doing book evaluations, etc. */
    bool SearchSingleton() const;

    /** See SetSearchSingleton() */
    void SetSearchSingleton(bool flag);
    

private:

    bool m_enabled;

    bool m_search_singleton;
};

inline bool EndgameCheck::Enabled() const
{
    return m_enabled;
}

inline void EndgameCheck::SetEnabled(bool enable)
{
    m_enabled = enable;
}

inline bool EndgameCheck::SearchSingleton() const
{
    return m_search_singleton;
}

inline void EndgameCheck::SetSearchSingleton(bool flag)
{
    m_search_singleton = flag;
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // ENDGAMECHECK_HPP
