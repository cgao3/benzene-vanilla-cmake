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

HexPoint PerfectPlayer::Search(HexBoard& brd, const Game& gameState,
			       HexColor color, const bitset_t& consider,
                               double maxTime, double& score)
{
    SG_UNUSED(gameState);
    SG_UNUSED(consider);
    SG_UNUSED(score);
    HexAssert(gameState.Board().IsStandardPosition());
    PointSequence pv;
    double oldTimelimit = m_solver.Timelimit();
    m_solver.SetTimelimit(maxTime);
    HexColor winner = m_solver.StartSearch(brd, color, m_positions, pv);
    m_solver.SetTimelimit(oldTimelimit);
    // Return winning/best losing move.
    if (winner != EMPTY)
    {
        HexAssert(!pv.empty());
        return pv[0];
    }
    // Didn't prove it, find non-losing move with most work.
    DfpnData data;
    if (!m_positions.Get(gameState.Board(), data))
        throw BenzeneException("Root node not in database!?!");
    std::size_t maxWork = 0;
    HexPoint bestMove = pv[0];
    StoneBoard myBrd(gameState.Board());
    for (std::size_t i = 0; data.m_children.Size(); ++i)
    {
        data.m_children.PlayMove(i, myBrd, color);
        DfpnData child;
        if (m_positions.Get(myBrd, child))
        {
            // We're assuming no children are losing (ie, no move is winning),
            // so we just need to avoid losing moves (winning children).
            if (!child.m_bounds.IsWinning() && child.m_work > maxWork)
            {
                bestMove = data.m_children.FirstMove(i);
                maxWork = child.m_work;
            }
        }
        data.m_children.UndoMove(i, myBrd);
    }
    return bestMove;
}

//----------------------------------------------------------------------------
