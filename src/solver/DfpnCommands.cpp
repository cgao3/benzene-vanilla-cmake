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
                           boost::scoped_ptr<DfpnHashTable>& tt,
                           boost::scoped_ptr<DfpnDB>& db,
                           DfpnPositions& positions)
    : m_game(game), 
      m_env(env),
      m_solver(solver),
      m_tt(tt),
      m_db(db),
      m_positions(positions)
{
}

void DfpnCommands::Register(GtpEngine& e)
{
    Register(e, "param_dfpn", &DfpnCommands::CmdParam);
    Register(e, "param_dfpn_db", &DfpnCommands::CmdParamSolverDB);
    Register(e, "dfpn-clear-tt", &DfpnCommands::CmdClearTT);
    Register(e, "dfpn-get-bounds", &DfpnCommands::CmdGetBounds);
    Register(e, "dfpn-get-state", &DfpnCommands::CmdGetState);    
    Register(e, "dfpn-get-work", &DfpnCommands::CmdGetWork);
    Register(e, "dfpn-solve-state", &DfpnCommands::CmdSolveState);
    Register(e, "dfpn-solver-find-winning", &DfpnCommands::CmdFindWinning);
    Register(e, "dfpn-open-db", &DfpnCommands::CmdOpenDB);
    Register(e, "dfpn-close-db", &DfpnCommands::CmdCloseDB);
}

void DfpnCommands::Register(GtpEngine& engine, const std::string& command,
                            GtpCallback<DfpnCommands>::Method method)
{
    engine.Register(command, new GtpCallback<DfpnCommands>(this, method));
}

//----------------------------------------------------------------------------

void DfpnCommands::CmdParamSolverDB(HtpCommand& cmd)
{
    SolverDBParameters& param = m_positions.Parameters();
    if (cmd.NuArg() == 0)
    {
        cmd << '\n'
            << "[bool] use_flipped_states " << param.m_useFlippedStates << '\n'
            << "[bool] use_proof_transpositions " 
            << param.m_useProofTranspositions << '\n'
            << "[string] max_stones " << param.m_maxStones << '\n'
            << "[string] trans_stones " << param.m_transStones << '\n';
    }
    else if (cmd.NuArg() == 2)
    {
        std::string name = cmd.Arg(0);
        if (name == "use_flipped_states")
            param.m_useFlippedStates = cmd.BoolArg(1);
        else if (name == "use_proof_transpositions")
            param.m_useProofTranspositions = cmd.BoolArg(1);
        else if (name == "max_stones")
            param.m_maxStones = cmd.IntArg(1, 0);
        else if (name == "trans_stones")
            param.m_transStones = cmd.IntArg(1, 0);
        else
            throw HtpFailure() << "unknown parameter: " << name;
    }
    else 
        throw HtpFailure("Expected 0 or 2 arguments");
}

void DfpnCommands::CmdParam(HtpCommand& cmd)
{
    if (cmd.NuArg() == 0)
    {
        cmd << '\n'
            << "[bool] use_guifx "
            << m_solver.UseGuiFx() << '\n'
            << "[string] timelimit "
            << m_solver.Timelimit() << '\n'
            << "[string] widening_base "
            << m_solver.WideningBase() << '\n'
            << "[string] widening_factor "
            << m_solver.WideningFactor() << '\n';
    }
    else if (cmd.NuArg() == 2)
    {
        std::string name = cmd.Arg(0);
        if (name == "use_guifx")
            m_solver.SetUseGuiFx(cmd.BoolArg(1));
        else if (name == "timelimit")
            m_solver.SetTimelimit(cmd.FloatArg(1));
        else if (name == "widening_base")
        {
            int value = cmd.IntArg(1, 0);
            if (0 < value)
                m_solver.SetWideningBase(value);
            else
                throw GtpFailure() << "widening_base must be positive.";
        }
        else if (name == "widening_factor")
        {
            float value = cmd.FloatArg(1);
            if (0.0f < value && value <= 1.0f)
                m_solver.SetWideningFactor(value);
            else
                throw GtpFailure() << "widening_factor must be in (0, 1]";
        }
        else
            throw GtpFailure() << "Unknown parameter: " << name;
    }
    else
        throw GtpFailure() << "Expected 0 or 2 arguments";
}

/** Solves the current state with dfpn using the current hashtable. */
void DfpnCommands::CmdSolveState(HtpCommand& cmd)
{
    cmd.CheckNuArg(1);
    HexColor colorToMove = HtpUtil::ColorArg(cmd, 0);
    PointSequence pv;
    HexBoard& brd = m_env.SyncBoard(m_game.Board());
    HexColor winner = m_solver.StartSearch(brd, m_positions, pv, colorToMove);
    cmd << winner;
}

/** Finds all winning moves in the current state with dfpn,
    using the current hashtable. */
void DfpnCommands::CmdFindWinning(HtpCommand& cmd)
{
    cmd.CheckNuArg(1);
    HexColor colorToMove = HtpUtil::ColorArg(cmd, 0);
    HexBoard& brd = m_env.SyncBoard(m_game.Board());
    brd.ComputeAll(colorToMove);
    bitset_t consider = (PlayerUtils::IsDeterminedState(brd, colorToMove) ?
                         brd.GetState().GetEmpty() :
                         PlayerUtils::MovesToConsider(brd, colorToMove));
    bitset_t winning;
    SgTimer timer;

    for (BitsetIterator p(consider); p; ++p)
    {
        StoneBoard board(m_game.Board());
        board.PlayMove(colorToMove, *p);
        HexBoard& brd = m_env.SyncBoard(board);
        LogInfo() << "****** Trying " << *p << " ******\n" << brd << '\n';
        PointSequence pv;
        HexColor winner = m_solver.StartSearch(brd, m_positions,
                                               pv, !colorToMove);
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
    if (m_positions.Get(m_game.Board(), data))
        cmd << data << '\n';
}

/** Displays bounds of every empty cell in current state.
    Bounds are obtained from the current hashtable. */
void DfpnCommands::CmdGetBounds(HtpCommand& cmd)
{
    cmd.CheckNuArg(1);
    HexColor colorToMove = HtpUtil::ColorArg(cmd, 0);
    StoneBoard brd(m_game.Board());
    for (BitsetIterator it(brd.GetEmpty()); it; ++it)
    {
        brd.PlayMove(colorToMove, *it);
        DfpnData data;
        if (m_positions.Get(brd, data))
        {
            cmd << ' ' << *it << ' ';
            if (data.m_bounds.IsWinning())
                cmd << 'L';
            else if (data.m_bounds.IsLosing())
                cmd << 'W';
            else 
                cmd << data.m_bounds.phi << ':' << data.m_bounds.delta;
        }
        brd.UndoMove(*it);
    }
}

/** Displays work of every empty cell in current state.
    Bounds are obtained from the current hashtable. */
void DfpnCommands::CmdGetWork(HtpCommand& cmd)
{
    cmd.CheckNuArg(1);
    HexColor colorToMove = HtpUtil::ColorArg(cmd, 0);
    StoneBoard brd(m_game.Board());
    for (BitsetIterator it(brd.GetEmpty()); it; ++it)
    {
        brd.PlayMove(colorToMove, *it);
        DfpnData data;
        if (m_positions.Get(brd, data))
            cmd << ' ' << *it << ' ' << data.m_work;
        brd.UndoMove(*it);
    }
}

/** Opens a database. 
    Usage: "db-open [filename]"
*/
void DfpnCommands::CmdOpenDB(HtpCommand& cmd)
{
    cmd.CheckNuArgLessEqual(3);
    std::string filename = cmd.Arg(0);
    try {
        m_db.reset(new DfpnDB(filename));
    }
    catch (BenzeneException& e) {
        m_db.reset(0);
        throw HtpFailure() << "Error opening db: '" << e.what() << "'\n";
    }
}

/** Closes an open database. */
void DfpnCommands::CmdCloseDB(HtpCommand& cmd)
{
    cmd.CheckNuArg(0);
    if (m_db.get() == 0)
        throw HtpFailure("No open database!\n");
    m_db.reset(0);
}

//----------------------------------------------------------------------------
