//----------------------------------------------------------------------------
/** @file DfpnCommands.hpp
 */
//----------------------------------------------------------------------------

#ifndef DFPNCOMMANDS_HPP
#define DFPNCOMMANDS_HPP

#include "Game.hpp"
#include "HexBoard.hpp"
#include "HexHtpEngine.hpp"
#include "HexEnvironment.hpp"
#include "DfpnSolver.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Commands for building/inspecting virtual connections. */
class DfpnCommands
{
public:
    DfpnCommands(Game& game, HexEnvironment& env, DfpnSolver& solver,
                 boost::scoped_ptr<DfpnHashTable>& hashTable);

    void Register(GtpEngine& engine);

private:
    Game& m_game;

    HexEnvironment& m_env;

    DfpnSolver& m_solver;

    boost::scoped_ptr<DfpnHashTable>& m_tt;
    
    void Register(GtpEngine& engine, const std::string& command,
                  GtpCallback<DfpnCommands>::Method method);

    void CmdParam(HtpCommand& cmd);
    void CmdSolveState(HtpCommand& cmd);
    void CmdFindWinning(HtpCommand& cmd);
    void CmdClearTT(HtpCommand& cmd);
    void CmdGetState(HtpCommand& cmd);
};

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // DFPNCOMMANDS_HPP
