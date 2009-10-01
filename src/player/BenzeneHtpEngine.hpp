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
#include "DfpnCommands.hpp"
#include "HexEnvironment.hpp"
#include "HexHtpEngine.hpp"
#include "Solver.hpp"
#include "SolverCommands.hpp"
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

    /** @page benzenehtpenginecommands BenzeneHtpEngine Commands
        - @link CmdLicense() @c benzene-license @endlink
        - @link CmdRegGenMove() @c reg_genmove @endlink
        - @link CmdGetAbsorbGroup() @c get_absorb_group @endlink
        - @link CmdHandbookAdd() @c handbook-add @endlink
        - @link CmdComputeInferior() @c compute-inferior @endlink
        - @link CmdComputeFillin() @c compute-fillin @endlink
        - @link CmdComputeVulnerable() @c compute-vulnerable @endlink
        - @link CmdComputeDominated() @c compute-dominated @endlink
        - @link CmdFindCombDecomp() @c find-comb-decomp @endlink
        - @link CmdFindSplitDecomp() @c find-split-decomp @endlink
        - @link CmdEncodePattern() @c encode-pattern @endlink
        - @link CmdParamPlayer() @c param_player @endlink
        - @link CmdParamDfpn() @c param_dfpn @endlink
        - @link CmdEvalTwoDist() @c eval-twod @endlink
        - @link CmdEvalResist() @c eval-resist @endlink
        - @link CmdEvalResistDelta() @c eval-resist-delta @endlink
        - @link CmdEvalInfluence() @c eval-influence @endlink
        - @link CmdMiscDebug() @c misc_debug @endlink
    */

    /** @name Command Callbacks */
    // @{
    // The callback functions are documented in the cpp file

    void CmdLicense(HtpCommand& cmd);

    void CmdRegGenMove(HtpCommand& cmd);

    void CmdGetAbsorbGroup(HtpCommand& cmd);
    
    void CmdHandbookAdd(HtpCommand& cmd);

    void CmdComputeInferior(HtpCommand& cmd);
    void CmdComputeFillin(HtpCommand& cmd);
    void CmdComputeVulnerable(HtpCommand& cmd);
    void CmdComputeDominated(HtpCommand& cmd);
    void CmdFindCombDecomp(HtpCommand& cmd);
    void CmdFindSplitDecomp(HtpCommand& cmd);
    void CmdEncodePattern(HtpCommand& cmd);
    
    void CmdParamPlayer(HtpCommand& cmd);
    void CmdParamDfpn(HtpCommand& cmd);
    
    void CmdEvalTwoDist(HtpCommand& cmd);
    void CmdEvalResist(HtpCommand& cmd);
    void CmdEvalResistDelta(HtpCommand& cmd);
    void CmdEvalInfluence(HtpCommand& cmd);

    void CmdMiscDebug(HtpCommand& cmd);

    // @} // @name

protected:

    BenzenePlayer& m_player;

    /** Player's environment. */
    HexEnvironment m_pe;

    /** Solver's environment. */
    HexEnvironment m_se;

    Solver m_solver;

    DfpnSolver m_solverDfpn;

    boost::scoped_ptr<SolverTT> m_solver_tt;

    boost::scoped_ptr<DfpnHashTable> m_dfpn_tt;

    boost::scoped_ptr<SolverDB> m_db;

    HexEnvironmentCommands m_playerEnvCommands;

    HexEnvironmentCommands m_solverEnvCommands;

    VCCommands m_vcCommands;

    SolverCommands m_solverCommands;

    DfpnCommands m_dfpnCommands;

    virtual void NewGame(int width, int height);
    
    virtual HexPoint GenMove(HexColor color, double max_time);

    void ParamPlayer(BenzenePlayer* player, HtpCommand& cmd);

private:
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

    void RegisterCmd(const std::string& name,
                     GtpCallback<BenzeneHtpEngine>::Method method);

};

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // BENZENEHTPENGINE_H
