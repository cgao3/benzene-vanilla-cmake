//----------------------------------------------------------------------------
/** @file HandBookCheck.hpp
 */
//----------------------------------------------------------------------------

#ifndef HANDBOOKCHECK_HPP
#define HANDBOOKCHECK_HPP

#include "BenzenePlayer.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Checks book before search. */
class HandBookCheck : public BenzenePlayerFunctionality
{
public:

    /** Adds hand-created book check to the given player. */
    HandBookCheck(BenzenePlayer* player);
    
    /** Destructor. */
    virtual ~HandBookCheck();
    
    /** Checks if any hand-created move suggestion corresponds to the
	current state. If matching suggestion is found, returns hand
	book move. Otherwise calls player's pre_search() and returns
	its computed move.
    */
    virtual HexPoint pre_search(HexBoard& brd, const Game& game_state,
				HexColor color, bitset_t& consider,
                                double max_time, double& score);

    bool Enabled() const;

    void SetEnabled(bool enable);

private:

    //----------------------------------------------------------------------
    
    void LoadHandBook();
    
    HexPoint HandBookResponse(const StoneBoard& brd, HexColor color);

    bool m_enabled;
    
    bool m_handBookLoaded;

    std::map<std::string, HexPoint> m_response;
};

inline bool HandBookCheck::Enabled() const
{
    return m_enabled;
}
    
inline void HandBookCheck::SetEnabled(bool enable)
{
    m_enabled = enable;
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // HANDBOOKCHECK_HPP
