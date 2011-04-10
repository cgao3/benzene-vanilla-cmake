//----------------------------------------------------------------------------
/** @file DfsCommands.hpp */
//----------------------------------------------------------------------------

#ifndef SOLVERCOMMANDS_HPP
#define SOLVERCOMMANDS_HPP

#include "Game.hpp"
#include "HexBoard.hpp"
#include "HexHtpEngine.hpp"
#include "HexEnvironment.hpp"
#include "DfsSolver.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Commands for building/inspecting virtual connections. */
class DfsCommands
{
public:
    DfsCommands(Game& game, HexEnvironment& env, DfsSolver& solver,
                boost::scoped_ptr<DfsHashTable>& tt, 
                boost::scoped_ptr<DfsDB>& db, 
                DfsStates& positions);

    void Register(GtpEngine& engine);

    void AddAnalyzeCommands(HtpCommand& cmd);

private:
    Game& m_game;

    HexEnvironment& m_env;

    DfsSolver& m_solver;

    boost::scoped_ptr<DfsHashTable>& m_tt;

    boost::scoped_ptr<DfsDB>& m_db;

    DfsStates& m_positions;

    void Register(GtpEngine& engine, const std::string& command,
                  GtpCallback<DfsCommands>::Method method);

    void CmdParamSolver(HtpCommand& cmd);
    void CmdParamSolverDB(HtpCommand& cmd);
    void CmdSolveState(HtpCommand& cmd);
    void CmdSolverClearTT(HtpCommand& cmd);
    void CmdSolverFindWinning(HtpCommand& cmd);
    void CmdDBOpen(HtpCommand& cmd);
    void CmdDBClose(HtpCommand& cmd);
    void CmdDBStat(HtpCommand& cmd);
    void CmdGetState(HtpCommand& cmd);
    void CmdGetPV(HtpCommand& cmd);
    void CmdHistogram(HtpCommand& cmd);
};

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // SOLVERCOMMANDS_HPP
