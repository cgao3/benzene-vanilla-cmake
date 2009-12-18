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

    explicit PerfectPlayer(DfpnSolver& solver, DfpnPositions& positions);

    virtual ~PerfectPlayer();
    
    /** Returns "perfect". */
    std::string Name() const;

private:

    /** Generates a move in the given gamestate using DfpnSolver. */
    HexPoint Search(HexBoard& brd, const Game& game_state,
                    HexColor color, const bitset_t& consider,
                    double max_time, double& score);

    DfpnSolver& m_solver;

    DfpnPositions& m_positions;
};

inline std::string PerfectPlayer::Name() const
{
    return "perfect";
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // PERFECTPLAYER_HPP
