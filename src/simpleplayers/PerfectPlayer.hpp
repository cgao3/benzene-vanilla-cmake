//----------------------------------------------------------------------------
/** @file PerfectPlayer.hpp
 */
//----------------------------------------------------------------------------

#ifndef PERFECTPLAYER_HPP
#define PERFECTPLAYER_HPP

#include "DfpnSolver.hpp"
#include "HexBoard.hpp"
#include "BenzenePlayer.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Player using Solver to generate moves. Works best on boards 7x7
    and smaller.

    @note This player is currently not used!!
*/
class PerfectPlayer : public BenzenePlayer
{
public:

    explicit PerfectPlayer(DfpnSolver& solver, DfpnStates& positions);

    virtual ~PerfectPlayer();
    
    /** Returns "perfect". */
    std::string Name() const;

    /** Maximum time to use per search. */
    double MaxTime() const;

    /** See MaxTime() */
    void SetMaxTime(double time);

    /** See DfpnSolver::PropagateBackwards(). */
    bool PropagateBackwards() const;

    /** See PropagateBackwards() */
    void SetPropagateBackwards(bool flag);

private:
    DfpnSolver& m_solver;

    DfpnStates& m_positions;

    double m_maxTime;

    bool m_propagateBackwards;

    /** Generates a move in the given gamestate using DfpnSolver. */
    HexPoint Search(const HexState& state, const Game& game,
                    HexBoard& brd, const bitset_t& consider,
                    double maxTime, double& score);
};

inline std::string PerfectPlayer::Name() const
{
    return "perfect";
}

inline double PerfectPlayer::MaxTime() const
{
    return m_maxTime;
}

inline void PerfectPlayer::SetMaxTime(double time)
{
    m_maxTime = time;
}

inline bool PerfectPlayer::PropagateBackwards() const
{
    return m_propagateBackwards;
}

inline void PerfectPlayer::SetPropagateBackwards(bool flag)
{
    m_propagateBackwards = flag;
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // PERFECTPLAYER_HPP
