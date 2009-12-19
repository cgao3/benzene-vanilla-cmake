//----------------------------------------------------------------------------
/** @file PlayAndSolve.cpp
 */
//----------------------------------------------------------------------------

#include "PlayAndSolve.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

PlayAndSolve::PlayAndSolve(HexBoard& playerBrd, HexBoard& solverBrd,
                           BenzenePlayer& player, DfpnSolver& solver,
                           DfpnPositions& positions, const Game& game)
    : m_playerBrd(playerBrd),
      m_solverBrd(solverBrd),
      m_player(player),
      m_solver(solver),
      m_positions(positions),
      m_game(game)
{
}

HexPoint PlayAndSolve::GenMove(HexColor color, double maxTime)
{
    if (m_game.Board().WhoseTurn() != color)
        LogWarning() << "*** Playing and solving for different colors! ***\n";

    boost::mutex mutex;
    boost::barrier barrier(3);
    m_parallelResult = INVALID_POINT;
    boost::thread playerThread(PlayerThread(*this, mutex, barrier, 
                                            color, maxTime));
    boost::thread solverThread(SolverThread(*this, mutex, barrier, color));
    barrier.wait();
    playerThread.join();
    solverThread.join();
    return m_parallelResult;
}


void PlayAndSolve::PlayerThread::operator()()
{
    LogInfo() << "*** PlayerThread ***\n";
    HexBoard& brd = m_ps.m_playerBrd;
    brd.GetState().SetState(m_ps.m_game.Board());
    double score;
    HexPoint move = m_ps.m_player.GenMove(brd, m_ps.m_game, m_color, 
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
    brd.GetState().SetState(m_ps.m_game.Board());
    PointSequence pv;
    HexColor colorToMove = m_ps.m_game.Board().WhoseTurn();
    HexColor winner = m_ps.m_solver.StartSearch(m_ps.m_solverBrd, colorToMove,
                                                m_ps.m_positions, pv);
    
    if (winner != EMPTY)
    {
        if (!pv.empty() && pv[0] != INVALID_POINT)
        {  
            /// FIXME: do we really need the above checks?
            boost::mutex::scoped_lock lock(m_mutex);
            m_ps.m_parallelResult = pv[0];
            if (winner == colorToMove)
            {
                LogInfo() << "*** FOUND WIN!!! ***\n" 
                          << "PV:" << HexPointUtil::ToString(pv) << '\n';
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
