//----------------------------------------------------------------------------
/** @file DfpnCommands.cpp
 */
//----------------------------------------------------------------------------

#include "BitsetIterator.hpp"
#include "PlayerUtils.hpp"
#include "DfpnCommands.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

DfpnCommands::DfpnCommands(Game& game, HexEnvironment& env,
                           DfpnSolver& solver,
                           boost::scoped_ptr<DfpnHashTable>& tt)
    : m_game(game), 
      m_env(env),
      m_solver(solver),
      m_tt(tt)
{
}

void DfpnCommands::Register(GtpEngine& e)
{
    Register(e, "param_dfpn", &DfpnCommands::CmdParam);
    Register(e, "dfpn-clear-tt", &DfpnCommands::CmdClearTT);
    Register(e, "dfpn-get-state", &DfpnCommands::CmdGetState);    
    Register(e, "dfpn-solve-state", &DfpnCommands::CmdSolveState);
    Register(e, "dfpn-solver-find-winning", &DfpnCommands::CmdFindWinning);
}

void DfpnCommands::Register(GtpEngine& engine, const std::string& command,
                              GtpCallback<DfpnCommands>::Method method)
{
    engine.Register(command, new GtpCallback<DfpnCommands>(this, method));
}

//----------------------------------------------------------------------------

void DfpnCommands::CmdParam(HtpCommand& cmd)
{
    if (cmd.NuArg() == 0)
    {
        cmd << '\n'
            << "[bool] use_bounds_correction "
            << m_solver.UseBoundsCorrection() << '\n'
            << "[bool] use_guifx "
            << m_solver.UseGuiFx() << '\n'
            << "[bool] use_unique_probes "
            << m_solver.UseUniqueProbes() << '\n'
            << "[string] timelimit "
            << m_solver.Timelimit() << '\n';
    }
    else if (cmd.NuArg() == 2)
    {
        std::string name = cmd.Arg(0);
        if (name == "use_guifx")
            m_solver.SetUseGuiFx(cmd.BoolArg(1));
        else if (name == "use_bounds_correction")
            m_solver.SetUseBoundsCorrection(cmd.BoolArg(1));
        else if (name == "use_unique_probes")
            m_solver.SetUseUniqueProbes(cmd.BoolArg(1));
        else if (name == "timelimit")
            m_solver.SetTimelimit(cmd.FloatArg(1));
        else
            throw GtpFailure() << "Unknown parameter: " << name;
    }
    else
        throw GtpFailure() << "Expected 0 or 2 arguments";
}

/** Solves the current state with dfpn using the current hashtable. */
void DfpnCommands::CmdSolveState(HtpCommand& cmd)
{
    cmd.CheckNuArg(0);
    HexBoard& brd = m_env.SyncBoard(m_game.Board());
    HexColor winner = m_solver.StartSearch(brd, *m_tt);
    cmd << winner;
}

/** Finds all winning moves in the current state with dfpn,
    using the current hashtable. */
void DfpnCommands::CmdFindWinning(HtpCommand& cmd)
{
    cmd.CheckNuArg(0);
    HexBoard& brd = m_env.SyncBoard(m_game.Board());
    HexColor colorToMove = brd.WhoseTurn();
    brd.ComputeAll(colorToMove);
    bitset_t consider = (PlayerUtils::IsDeterminedState(brd, colorToMove) ?
                         brd.getEmpty() :
                         PlayerUtils::MovesToConsider(brd, colorToMove));
    bitset_t winning;
    SgTimer timer;

    for (BitsetIterator p(consider); p; ++p)
    {
        StoneBoard board(m_game.Board());
        board.playMove(colorToMove, *p);
        HexBoard& brd = m_env.SyncBoard(board);
        LogInfo() << "****** Trying " << *p << " ******\n" << brd << '\n';

        HexColor winner = m_solver.StartSearch(brd, *m_tt);
        if (winner == colorToMove)
            winning.set(*p);
        LogInfo() << "****** " << winner << " wins ******\n";
    }
    LogInfo() << "Total Elapsed Time: " << timer.GetTime() << '\n';
    cmd << HexPointUtil::ToString(winning);
}

/** Clears the current dfpn hashtable. */
void DfpnCommands::CmdClearTT(HtpCommand& cmd)
{
    UNUSED(cmd);
    m_tt->Clear();
}

/** Displays information about the current state from the
    hashtable. */
void DfpnCommands::CmdGetState(HtpCommand& cmd)
{
    cmd.CheckArgNone();
    DfpnData data;
    if (m_tt->Get(m_game.Board().Hash(), data))
        cmd << data << '\n';
}

//----------------------------------------------------------------------------
