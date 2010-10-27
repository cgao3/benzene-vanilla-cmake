//----------------------------------------------------------------------------
/** @file PlayAndSolve.cpp
 */
//----------------------------------------------------------------------------

#include "PlayAndSolve.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

PlayAndSolve::PlayAndSolve(HexBoard& playerBrd, HexBoard& solverBrd,
                           BenzenePlayer& player, DfpnSolver& solver,
                           DfpnStates& positions, const Game& game)
    : m_playerBrd(playerBrd),
      m_solverBrd(solverBrd),
      m_player(player),
      m_solver(solver),
      m_positions(positions),
      m_game(game)
{
}

HexPoint PlayAndSolve::GenMove(const HexState& state, double maxTime)
{
    {
        // HACK: The player and solver threads could race to call
        // VCPattern::GetPatterns(), which constructs the patterns for
        // the first time. Force the player to build the vcs first so
        // this is not a problem.
        LogInfo() << "PlayAndSolve: Building VCs to avoid race condition.\n";
        m_playerBrd.GetPosition().SetPosition(state.Position());
        m_playerBrd.ComputeAll(state.ToPlay());
        LogInfo() << "PlayAndSolve: Continuing on as usual.\n";
    }

    boost::mutex mutex;
    boost::barrier barrier(3);
    m_parallelResult = INVALID_POINT;
    boost::thread playerThread(PlayerThread(*this, mutex, barrier, 
                                            state, maxTime));
    boost::thread solverThread(SolverThread(*this, mutex, barrier, state));
    barrier.wait();
    playerThread.join();
    solverThread.join();
    return m_parallelResult;
}


void PlayAndSolve::PlayerThread::operator()()
{
    LogInfo() << "*** PlayerThread ***\n";
    HexBoard& brd = m_ps.m_playerBrd;
    brd.GetPosition().SetPosition(m_state.Position());
    double score;
    HexPoint move = m_ps.m_player.GenMove(m_state, m_ps.m_game, brd,
                                          m_maxTime, score);
    {
        boost::mutex::scoped_lock lock(m_mutex);
        if (m_ps.m_parallelResult == INVALID_POINT)
        {
            LogInfo() << "*** Player move: " << move << '\n';
            m_ps.m_parallelResult = move;
        }
    }
    SgSetUserAbort(true);
    m_barrier.wait();
}


void PlayAndSolve::SolverThread::operator()()
{
    LogInfo() << "*** SolverThread ***\n";
    HexBoard& brd = m_ps.m_solverBrd;
    brd.GetPosition().SetPosition(m_state.Position());
    PointSequence pv;
    HexColor winner = m_ps.m_solver.StartSearch(m_state, m_ps.m_solverBrd,
                                                m_ps.m_positions, pv);
    
    if (winner != EMPTY)
    {
        if (!pv.empty() && pv[0] != INVALID_POINT)
        {  
            boost::mutex::scoped_lock lock(m_mutex);
            m_ps.m_parallelResult = pv[0];
            if (winner == m_state.ToPlay())
            {
                LogInfo() << "*** FOUND WIN!!! ***\n" 
                          << "PV: " << HexPointUtil::ToString(pv) << '\n';
            }
            else
            {
                LogInfo() << "*** FOUND LOSS!! ***\n" 
                          << "PV: " << HexPointUtil::ToString(pv) << '\n';
            }
            SgSetUserAbort(true);
        }
    }
    m_barrier.wait();
}

//----------------------------------------------------------------------------
