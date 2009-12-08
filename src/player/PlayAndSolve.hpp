//----------------------------------------------------------------------------
/** @file PlayAndSolve.hpp
 */
//----------------------------------------------------------------------------

#ifndef PLAYANDSOLVE_H
#define PLAYANDSOLVE_H

#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/thread/mutex.hpp>

#include "BenzenePlayer.hpp"
#include "DfpnCommands.hpp"
#include "HexEnvironment.hpp"
#include "HexHtpEngine.hpp"
#include "Solver.hpp"
#include "SolverCommands.hpp"
#include "VCCommands.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Runs a player's genmove() and dfpn search in parallel. */
class PlayAndSolve
{
public:
    PlayAndSolve(HexBoard& playerBrd, HexBoard& solverBrd,
                 BenzenePlayer& player, DfpnSolver& solver,
                 DfpnHashTable& hashTable, const Game& game);

    HexPoint GenMove(HexColor color, double maxTime);

private:    
    class PlayerThread
    {
    public:
        PlayerThread(PlayAndSolve& ps, boost::mutex& mutex, 
                     boost::barrier& barrier, HexColor color, double maxTime)
            : m_ps(ps), m_mutex(mutex), m_barrier(barrier),
              m_color(color), m_maxTime(maxTime) {};

        void operator()();
    private:
        PlayAndSolve& m_ps;
        boost::mutex& m_mutex;
        boost::barrier& m_barrier;
        HexColor m_color;
        double m_maxTime;
    };

    friend class SolverThread;

    class SolverThread
    {
    public:
        SolverThread(PlayAndSolve& ps, boost::mutex& mutex, 
                     boost::barrier& barrier,
                     HexColor color)
            : m_ps(ps), m_mutex(mutex), m_barrier(barrier),
              m_color(color) {};

        void operator()();
    private:
        PlayAndSolve& m_ps;
        boost::mutex& m_mutex;
        boost::barrier& m_barrier;
        HexColor m_color;
    };

    HexBoard& m_playerBrd;

    HexBoard& m_solverBrd;

    BenzenePlayer& m_player;

    DfpnSolver& m_solver;

    DfpnHashTable& m_hashTable;

    const Game& m_game;

    HexPoint m_parallelResult;
};

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // PLAYANDSOLVE_HPP
