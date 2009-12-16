//----------------------------------------------------------------------------
/** @file DfsCommands.hpp
 */
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
                   boost::scoped_ptr<DfsHashTable>& solverTT, 
                   boost::scoped_ptr<DfsDB>& solverDB);

    void Register(GtpEngine& engine);

private:
    Game& m_game;

    HexEnvironment& m_env;

    DfsSolver& m_solver;

    boost::scoped_ptr<DfsHashTable>& m_tt;
    
    boost::scoped_ptr<DfsDB>& m_db;
        
    void Register(GtpEngine& engine, const std::string& command,
                  GtpCallback<DfsCommands>::Method method);

    void CmdParamSolver(HtpCommand& cmd);
    void CmdSolveState(HtpCommand& cmd);
    void CmdSolverClearTT(HtpCommand& cmd);
    void CmdSolverFindWinning(HtpCommand& cmd);
    void CmdDBOpen(HtpCommand& cmd);
    void CmdDBClose(HtpCommand& cmd);
    void CmdDBGet(HtpCommand& cmd);
};

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // SOLVERCOMMANDS_HPP
