//----------------------------------------------------------------------------
/** @file BenzeneHtpEngine.hpp
 */
//----------------------------------------------------------------------------

#ifndef BENZENEHTPENGINE_H
#define BENZENEHTPENGINE_H

#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/thread/mutex.hpp>

#include "BenzenePlayer.hpp"
#include "HexEnvironment.hpp"
#include "HexHtpEngine.hpp"
#include "Solver.hpp"
#include "SolverDFPN.hpp"
#include "VCCommands.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** HTP engine with commands for stuff common to all UofA Hex
    players. */
class BenzeneHtpEngine: public HexHtpEngine
{
public:

    BenzeneHtpEngine(std::istream& in, std::ostream& out, int boardsize, 
                  BenzenePlayer& player);
    
    ~BenzeneHtpEngine();

    /** @name Command Callbacks */
    // @{
    // The callback functions are documented in the cpp file

    virtual void CmdLicense(HtpCommand& cmd);

    virtual void CmdRegGenMove(HtpCommand& cmd);

    virtual void CmdGetAbsorbGroup(HtpCommand& cmd);
    
    void CmdHandbookAdd(HtpCommand& cmd);

    virtual void CmdComputeInferior(HtpCommand& cmd);
    virtual void CmdComputeFillin(HtpCommand& cmd);
    virtual void CmdComputeVulnerable(HtpCommand& cmd);
    virtual void CmdComputeDominated(HtpCommand& cmd);
    virtual void CmdFindCombDecomp(HtpCommand& cmd);
    virtual void CmdFindSplitDecomp(HtpCommand& cmd);
    virtual void CmdEncodePattern(HtpCommand& cmd);

    virtual void CmdParamPlayer(HtpCommand& cmd);
    virtual void CmdParamSolver(HtpCommand& cmd);
    virtual void CmdParamSolverDfpn(HtpCommand& cmd);

    virtual void CmdEvalTwoDist(HtpCommand& cmd);
    virtual void CmdEvalResist(HtpCommand& cmd);
    virtual void CmdEvalResistDelta(HtpCommand& cmd);
    virtual void CmdEvalInfluence(HtpCommand& cmd);

    void CmdDfpnGetState(HtpCommand& cmd);

    virtual void CmdSolveState(HtpCommand& cmd);
    virtual void CmdSolveStateDfpn(HtpCommand& cmd);
    virtual void CmdSolverClearTT(HtpCommand& cmd);
    virtual void CmdSolverClearDfpnTT(HtpCommand& cmd);
    virtual void CmdSolverFindWinning(HtpCommand& cmd);

    void CmdDBOpen(HtpCommand& cmd);
    void CmdDBClose(HtpCommand& cmd);
    void CmdDBGet(HtpCommand& cmd);

    virtual void CmdMiscDebug(HtpCommand& cmd);

    // @} // @name

protected:

    virtual void NewGame(int width, int height);
    
    virtual HexPoint GenMove(HexColor color, double max_time);

    /** Searches through the player decorators to find an instance
        of type T. Returns 0 on failure. */
    template<typename T> T* GetInstanceOf(BenzenePlayer* player);

    void ParamPlayer(BenzenePlayer* player, HtpCommand& cmd);

    //-----------------------------------------------------------------------

    BenzenePlayer& m_player;

    /** Player's environment. */
    HexEnvironment m_pe;

    /** Solver's environment. */
    HexEnvironment m_se;

    HexEnvironmentCommands m_playerEnvCommands;

    HexEnvironmentCommands m_solverEnvCommands;

    VCCommands m_vcCommands;

    boost::scoped_ptr<Solver> m_solver;

    boost::scoped_ptr<SolverDFPN> m_solverDfpn;

    boost::scoped_ptr<SolverTT> m_solver_tt;

    boost::scoped_ptr<DfpnHashTable> m_dfpn_tt;

    boost::scoped_ptr<SolverDB> m_db;

private:
    void RegisterCmd(const std::string& name,
                     GtpCallback<BenzeneHtpEngine>::Method method);

    //-----------------------------------------------------------------------

    friend class PlayerThread;

    class PlayerThread
    {
    public:
        PlayerThread(BenzeneHtpEngine& engine, boost::mutex& mutex,
                     boost::barrier& barrier, HexColor color, 
                     double max_time)
            : m_engine(engine), m_mutex(mutex), m_barrier(barrier),
              m_color(color), m_max_time(max_time) {};

        void operator()();
    private:
        BenzeneHtpEngine& m_engine;
        boost::mutex& m_mutex;
        boost::barrier& m_barrier;
        HexColor m_color;
        double m_max_time;
    };

    friend class SolverThread;

    class SolverThread
    {
    public:
        SolverThread(BenzeneHtpEngine& engine, boost::mutex& mutex,
                     boost::barrier& barrier, HexColor color)
            : m_engine(engine), m_mutex(mutex), m_barrier(barrier),
              m_color(color) {};

        void operator()();
    private:
        BenzeneHtpEngine& m_engine;
        boost::mutex& m_mutex;
        boost::barrier& m_barrier;
        HexColor m_color;
    };

    bool m_useParallelSolver;

    HexPoint m_parallelResult;

    HexPoint ParallelGenMove(HexColor color, double max_time);
};

template<typename T> T* BenzeneHtpEngine::GetInstanceOf(BenzenePlayer* player)
{
    T* obj = dynamic_cast<T*>(player);
    if (obj)
        return obj;
    BenzenePlayerFunctionality* func 
        = dynamic_cast<BenzenePlayerFunctionality*>(player);
    if (func)
        return GetInstanceOf<T>(func->PlayerExtending());
    return 0;
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // BENZENEHTPENGINE_H
