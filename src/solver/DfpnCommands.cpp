//----------------------------------------------------------------------------
/** @file DfpnCommands.cpp */
//----------------------------------------------------------------------------

#include "BitsetIterator.hpp"
#include "EndgameUtil.hpp"
#include "DfpnCommands.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

DfpnCommands::DfpnCommands(Game& game, HexEnvironment& env,
                           DfpnSolver& solver,
                           boost::scoped_ptr<DfpnHashTable>& tt,
                           boost::scoped_ptr<DfpnDB>& db,
                           DfpnStates& positions)
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
    Register(e, "dfpn-get-pv", &DfpnCommands::CmdGetPV);
    Register(e, "dfpn-solve-state", &DfpnCommands::CmdSolveState);
    Register(e, "dfpn-solver-find-winning", &DfpnCommands::CmdFindWinning);
    Register(e, "dfpn-open-db", &DfpnCommands::CmdOpenDB);
    Register(e, "dfpn-close-db", &DfpnCommands::CmdCloseDB);
    Register(e, "dfpn-db-stat", &DfpnCommands::CmdDBStat);
    Register(e, "dfpn-evaluation-info", &DfpnCommands::CmdEvaluationInfo);
}

void DfpnCommands::Register(GtpEngine& engine, const std::string& command,
                            GtpCallback<DfpnCommands>::Method method)
{
    engine.Register(command, new GtpCallback<DfpnCommands>(this, method));
}

//----------------------------------------------------------------------------

void DfpnCommands::AddAnalyzeCommands(HtpCommand& cmd)
{
    cmd <<
        "param/DFPN Param/param_dfpn\n"
        "param/DFPN DB Param/param_dfpn_db\n"
        "none/DFPN Clear TT/dfpn-clear-tt\n"
        "string/DFPN Get Bounds/dfpn-get-bounds %m\n"
        "string/DFPN Get State/dfpn-get-state %m\n"
        "pspairs/DFPN Get Work/dfpn-get-work %m\n"
        "var/DFPN Get PV/dfpn-get-pv %m\n"
        "string/DFPN Solve State/dfpn-solve-state %m\n"
        "plist/DFPN Find Winning/dfpn-solver-find-winning %m\n"
        "none/DFPN Open DB/dfpn-open-db %r\n"
        "none/DFPN Close DB/dfpn-close-db\n"
        "string/DFPN DB Stats/dfpn-db-stat\n"
        "string/DFPN Eval Info/dfpn-evaluation-info\n";
}

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
            param.m_useFlippedStates = cmd.Arg<bool>(1);
        else if (name == "use_proof_transpositions")
            param.m_useProofTranspositions = cmd.Arg<bool>(1);
        else if (name == "max_stones")
            param.m_maxStones = cmd.ArgMin<int>(1, 0);
        else if (name == "trans_stones")
            param.m_transStones = cmd.ArgMin<int>(1, 0);
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
            << "[string] tt_bits "  
            << ((m_tt.get() == 0) ? 0 : log2(m_tt->MaxHash())) << '\n'
            << "[string] widening_base "
            << m_solver.WideningBase() << '\n'
            << "[string] widening_factor "
            << m_solver.WideningFactor() << '\n';
    }
    else if (cmd.NuArg() == 2)
    {
        std::string name = cmd.Arg(0);
        if (name == "use_guifx")
            m_solver.SetUseGuiFx(cmd.Arg<bool>(1));
        else if (name == "timelimit")
            m_solver.SetTimelimit(cmd.ArgMin<float>(1, 0.0));
	else if (name == "tt_bits")
	{
	    int bits = cmd.ArgMin<int>(1, 0);
	    if (bits == 0)
		m_tt.reset(0);
	    else
		m_tt.reset(new DfpnHashTable(1 << bits));
	}
        else if (name == "widening_base")
            m_solver.SetWideningBase(cmd.ArgMin<int>(1, 1));
        else if (name == "widening_factor")
        {
            float value = cmd.Arg<float>(1);
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
    cmd.CheckNuArgLessEqual(3);
    HexColor colorToMove = m_game.Board().WhoseTurn();
    if (cmd.NuArg() >= 1)
        colorToMove = HtpUtil::ColorArg(cmd, 0);
    // DfpnBounds::MAX_WORK cannot be used as an argument to ArgMinMax()
    // directly, because it is an integral constant class member that is not
    // defined anywhere and arguments to ArgMinMax() are passed by reference.
    // Older versions of GCC (including the current Cygwin GCC 4.3.4) generate
    // and error ("undefined reference to DfpnBounds::MAX_WORK"), probably in
    // accordance to the C++ standard. The best solution would be to change
    // GtpCommand::ArgMinMax() in Fuego to pass arguments by value.
    const DfpnBoundType maxWork = DfpnBounds::MAX_WORK;
    DfpnBoundType maxPhi = maxWork;
    DfpnBoundType maxDelta = maxWork;
    if (cmd.NuArg() >= 2)
        maxPhi = cmd.ArgMinMax<DfpnBoundType>(1, 0, maxWork);
    if (cmd.NuArg() >= 3)
        maxDelta = cmd.ArgMinMax<DfpnBoundType>(2, 0, maxWork);
    DfpnBounds maxBounds(maxPhi, maxDelta);
    PointSequence pv;
    HexBoard& brd = m_env.SyncBoard(m_game.Board());
    HexColor winner 
        = m_solver.StartSearch(HexState(m_game.Board(), colorToMove), brd, 
                               m_positions, pv, maxBounds);
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
    bitset_t consider = (EndgameUtil::IsDeterminedState(brd, colorToMove) ?
                         brd.GetPosition().GetEmpty() :
                         EndgameUtil::MovesToConsider(brd, colorToMove));
    bitset_t winning;
    SgTimer timer;

    HexState state(m_game.Board(), colorToMove);
    for (BitsetIterator p(consider); p; ++p)
    {
        state.PlayMove(*p);
        HexBoard& brd = m_env.SyncBoard(state.Position());
        LogInfo() << "****** Trying " << *p << " ******\n" << brd << '\n';
        PointSequence pv;
        HexColor winner = m_solver.StartSearch(state, brd, m_positions, pv);
        if (winner == colorToMove)
            winning.set(*p);
        LogInfo() << "****** " << winner << " wins ******\n";
        state.UndoMove(*p);
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
    cmd.CheckNuArg(1);
    HexColor colorToMove = HtpUtil::ColorArg(cmd, 0);    
    HexState state(m_game.Board(), colorToMove);
    DfpnData data;
    if (m_positions.Get(state, data))
        cmd << data << '\n';
}

/** Displays bounds of every empty cell in current state.
    Bounds are obtained from the current hashtable. */
void DfpnCommands::CmdGetBounds(HtpCommand& cmd)
{
    cmd.CheckNuArg(1);
    HexColor colorToMove = HtpUtil::ColorArg(cmd, 0);
    HexState state(m_game.Board(), colorToMove);
    for (BitsetIterator it(state.Position().GetEmpty()); it; ++it)
    {
        state.PlayMove(*it);
        DfpnData data;
        if (m_positions.Get(state, data))
        {
            cmd << ' ' << *it << ' ';
            if (data.m_bounds.IsWinning())
                cmd << 'L';
            else if (data.m_bounds.IsLosing())
                cmd << 'W';
            else 
                cmd << data.m_bounds.phi << ':' << data.m_bounds.delta;
        }
        state.UndoMove(*it);
    }
}

/** Displays work of every empty cell in current state.
    Bounds are obtained from the current hashtable. */
void DfpnCommands::CmdGetWork(HtpCommand& cmd)
{
    cmd.CheckNuArg(1);
    HexColor colorToMove = HtpUtil::ColorArg(cmd, 0);
    HexState state(m_game.Board(), colorToMove);
    for (BitsetIterator it(state.Position().GetEmpty()); it; ++it)
    {
        state.PlayMove(*it);
        DfpnData data;
        if (m_positions.Get(state, data))
            cmd << ' ' << *it << ' ' << data.m_work;
        state.UndoMove(*it);
    }
}

/** Displays PV from current position. */
void DfpnCommands::CmdGetPV(HtpCommand& cmd)
{
    cmd.CheckNuArg(1);
    HexColor colorToMove = HtpUtil::ColorArg(cmd, 0);
    PointSequence pv;
    SolverDBUtil::GetVariation(HexState(m_game.Board(), colorToMove), 
                               m_positions, pv);
    cmd << HexPointUtil::ToString(pv);
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

/** Prints database statistics. */
void DfpnCommands::CmdDBStat(HtpCommand& cmd)
{
    cmd.CheckNuArg(0);
    if (m_db.get() == 0)
        throw HtpFailure("No open database!\n");
    cmd << m_db->BDBStatistics();
}

void DfpnCommands::CmdEvaluationInfo(HtpCommand& cmd)
{
    cmd.CheckNuArg(0);
    cmd << m_solver.EvaluationInfo();
}

//----------------------------------------------------------------------------
