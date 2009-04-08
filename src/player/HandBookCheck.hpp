//----------------------------------------------------------------------------
// $Id: HandBookCheck.hpp 1877 2009-01-29 00:57:27Z broderic $
//----------------------------------------------------------------------------

#ifndef HANDBOOKCHECK_HPP
#define HANDBOOKCHECK_HPP

#include "BenzenePlayer.hpp"

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
	current state if "player-use-hand-book" is true. If matching
	suggestion is found, returns hand book move. Otherwise
        calls player's pre_search() and returns its computed move.
    */
    virtual HexPoint pre_search(HexBoard& brd, const Game& game_state,
				HexColor color, bitset_t& consider,
                                double time_remaining, double& score);

    bool Enabled() const;

    void SetEnabled(bool enable);

private:

    //----------------------------------------------------------------------
    
    /** Reads in the hand-book from a file and stores the
	board ID-response pairs. */
    void LoadHandBook();
    
    /** Uses the hand book to determine a response (if possible).
        @return INVALID_POINT on failure, valid move on success.
    */
    HexPoint HandBookResponse(const StoneBoard& brd, HexColor color);

    bool m_enabled;
    
    bool m_handBookLoaded;

    std::vector<std::string> m_id;

    std::vector<HexPoint> m_response;
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

#endif // HANDBOOKCHECK_HPP
