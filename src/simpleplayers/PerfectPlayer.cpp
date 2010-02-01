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
    LogInfo() << "PerfectPlayer::Search()\n";
    double timeForMove = std::min(60.0, maxTime);
    LogInfo() << "TimeForMove=" << timeForMove << '\n';
    double oldTimelimit = m_solver.Timelimit();
    m_solver.SetTimelimit(timeForMove);
    PointSequence pv;
    HexColor winner = m_solver.StartSearch(brd, color, m_positions, pv);
    m_solver.SetTimelimit(oldTimelimit);
    // Return winning/best losing move.
    if (winner != EMPTY && !pv.empty())
        return pv[0];
    // NOTE: This can happen if the current state is a terminal state
    // under a rotation, but it is not detected as terminal here
    // (there can be slight differences in vcs between rotated
    // states). In this case, DFPN does not have a move stored and we
    // are stuck if we continue to use the current set of stored
    // positions. So we create a new empty DfpnPositions object with a
    // small hashtable to use for this (hopefully really small) search
    // to find the winning move.
    if (winner != EMPTY && pv.empty())
    {
        boost::scoped_ptr<DfpnHashTable> myTable(new DfpnHashTable(10));
        boost::scoped_ptr<DfpnDB> myDB(0);
        SolverDBParameters myParam;
        DfpnPositions myPositions(myTable, myDB, myParam);
        LogInfo() << "PerfectPlayer: Found win with empty pv at this state:\n";
        LogInfo() << brd << '\n';
        LogInfo() << "PerfectPlayer: Re-solving with temporary hash table.\n";
        winner = m_solver.StartSearch(brd, color, myPositions, pv);
        HexAssert(winner != EMPTY);
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
    for (std::size_t i = 0; i < data.m_children.Size(); ++i)
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
