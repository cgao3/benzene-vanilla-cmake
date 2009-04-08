//----------------------------------------------------------------------------
// $Id: SolverCheck.hpp 1877 2009-01-29 00:57:27Z broderic $
//----------------------------------------------------------------------------

#ifndef SOLVERCHECK_HPP
#define SOLVERCHECK_HPP

#include "BenzenePlayer.hpp"

//----------------------------------------------------------------------------

/** Runs solver for a short-time in an attempt to find simple wins
    that the players may miss. */
class SolverCheck : public BenzenePlayerFunctionality
{
public:

    /** Extends the given player. */
    SolverCheck(BenzenePlayer* player);

    /** Destructor. */
    virtual ~SolverCheck();

    /** Returns a winning move if Solver finds one, otherwise passes
	gamestate onto the player it is extending. Time remaining is
	modified in this case. */
    virtual HexPoint pre_search(HexBoard& brd, const Game& game_state,
				HexColor color, bitset_t& consider,
                                double time_remaining, double& score);
    bool Enabled() const;

    void SetEnabled(bool enable);

private:

    bool m_enabled;

    int m_threshold;
    
    double m_timelimit;
};

inline bool SolverCheck::Enabled() const
{
    return m_enabled;
}
    
inline void SolverCheck::SetEnabled(bool enable)
{
    m_enabled = enable;
}

//----------------------------------------------------------------------------

#endif // SOLVERCHECK_HPP
