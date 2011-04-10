//----------------------------------------------------------------------------
/** @file DfpnCommands.hpp */
//----------------------------------------------------------------------------

#ifndef DFPNCOMMANDS_HPP
#define DFPNCOMMANDS_HPP

#include "Game.hpp"
#include "HexBoard.hpp"
#include "HexHtpEngine.hpp"
#include "HexEnvironment.hpp"
#include "SolverDB.hpp"
#include "DfpnSolver.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Commands for building/inspecting virtual connections. */
class DfpnCommands
{
public:
    DfpnCommands(Game& game, HexEnvironment& env, DfpnSolver& solver,
                 boost::scoped_ptr<DfpnHashTable>& hashTable,
                 boost::scoped_ptr<DfpnDB>& db,
                 DfpnStates& positions);

    void Register(GtpEngine& engine);

    void AddAnalyzeCommands(HtpCommand& cmd);

private:
    Game& m_game;

    HexEnvironment& m_env;

    DfpnSolver& m_solver;

    boost::scoped_ptr<DfpnHashTable>& m_tt;

    boost::scoped_ptr<DfpnDB>& m_db;
    
    DfpnStates& m_positions;

    void Register(GtpEngine& engine, const std::string& command,
                  GtpCallback<DfpnCommands>::Method method);

    void CmdParam(HtpCommand& cmd);
    void CmdParamSolverDB(HtpCommand& cmd);
    void CmdSolveState(HtpCommand& cmd);
    void CmdFindWinning(HtpCommand& cmd);
    void CmdClearTT(HtpCommand& cmd);
    void CmdGetState(HtpCommand& cmd);
    void CmdGetBounds(HtpCommand& cmd);
    void CmdGetWork(HtpCommand& cmd);
    void CmdGetPV(HtpCommand& cmd);
    void CmdOpenDB(HtpCommand& cmd);
    void CmdCloseDB(HtpCommand& cmd);
    void CmdDBStat(HtpCommand& cmd);
    void CmdEvaluationInfo(HtpCommand& cmd);
};

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // DFPNCOMMANDS_HPP
