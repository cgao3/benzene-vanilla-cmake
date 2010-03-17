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
#include "HexState.hpp"
#include "DfsSolver.hpp"
#include "DfsCommands.hpp"
#include "VCCommands.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Runs a player's genmove() and dfpn search in parallel. */
class PlayAndSolve
{
public:
    PlayAndSolve(HexBoard& playerBrd, HexBoard& solverBrd,
                 BenzenePlayer& player, DfpnSolver& solver,
                 DfpnStates& positions, const Game& game);

    HexPoint GenMove(const HexState& state, double maxTime);

private:    
    class PlayerThread
    {
    public:
        PlayerThread(PlayAndSolve& ps, boost::mutex& mutex, 
                     boost::barrier& barrier, const HexState& state, 
                     double maxTime)
            : m_ps(ps), m_mutex(mutex), m_barrier(barrier),
              m_state(state), m_maxTime(maxTime) {};

        void operator()();
    private:
        PlayAndSolve& m_ps;
        boost::mutex& m_mutex;
        boost::barrier& m_barrier;
        const HexState& m_state;
        double m_maxTime;
    };

    friend class SolverThread;

    class SolverThread
    {
    public:
        SolverThread(PlayAndSolve& ps, boost::mutex& mutex, 
                     boost::barrier& barrier, const HexState& state)
            : m_ps(ps), m_mutex(mutex), m_barrier(barrier),
              m_state(state) {};

        void operator()();
    private:
        PlayAndSolve& m_ps;
        boost::mutex& m_mutex;
        boost::barrier& m_barrier;
        const HexState& m_state;
    };

    HexBoard& m_playerBrd;

    HexBoard& m_solverBrd;

    BenzenePlayer& m_player;

    DfpnSolver& m_solver;

    DfpnStates& m_positions;

    const Game& m_game;

    HexPoint m_parallelResult;
};

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // PLAYANDSOLVE_HPP
