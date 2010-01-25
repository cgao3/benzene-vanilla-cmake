//----------------------------------------------------------------------------
/** @file PerfectPlayer.cpp
    NOT TESTED!
 */
//----------------------------------------------------------------------------

#include "PerfectPlayer.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

PerfectPlayer::PerfectPlayer(DfpnSolver& solver, DfpnPositions& positions)
    : BenzenePlayer(),
      m_solver(solver),
      m_positions(positions)
{
}

PerfectPlayer::~PerfectPlayer()
{
}

//----------------------------------------------------------------------------

HexPoint PerfectPlayer::Search(HexBoard& brd, const Game& game_state,
			       HexColor color, const bitset_t& consider,
                               double max_time, double& score)
{
    SG_UNUSED(game_state);
    SG_UNUSED(consider); 
    SG_UNUSED(max_time); 
    SG_UNUSED(color);
    SG_UNUSED(score);
    HexAssert(game_state.Board().IsStandardPosition());
    PointSequence pv;
    m_solver.StartSearch(brd, color, m_positions, pv);
    HexAssert(!pv.empty());
    return pv[0];
}

//----------------------------------------------------------------------------
