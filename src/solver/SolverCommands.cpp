//----------------------------------------------------------------------------
/** @file SolverCommands.cpp
 */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "SgTimer.h"
#include "BitsetIterator.hpp"
#include "PlayerUtils.hpp"
#include "SolverCommands.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

SolverCommands::SolverCommands(Game& game, HexEnvironment& env,
                               Solver& solver,
                               boost::scoped_ptr<SolverTT>& solverTT, 
                               boost::scoped_ptr<SolverDB>& solverDB)
    : m_game(game), 
      m_env(env),
      m_solver(solver),
      m_tt(solverTT),
      m_db(solverDB)
{
}

void SolverCommands::Register(GtpEngine& e)
{
    Register(e, "param_solver", &SolverCommands::CmdParamSolver);
    Register(e, "solve-state", &SolverCommands::CmdSolveState);
    Register(e, "solver-clear-tt", &SolverCommands::CmdSolverClearTT);
    Register(e, "solver-find-winning", &SolverCommands::CmdSolverFindWinning);

    Register(e, "db-open", &SolverCommands::CmdDBOpen);
    Register(e, "db-close", &SolverCommands::CmdDBClose);
    Register(e, "db-get", &SolverCommands::CmdDBGet);
}

void SolverCommands::Register(GtpEngine& engine, const std::string& command,
                              GtpCallback<SolverCommands>::Method method)
{
    engine.Register(command, new GtpCallback<SolverCommands>(this, method));
}

//----------------------------------------------------------------------------

void SolverCommands::CmdParamSolver(HtpCommand& cmd)
{
    if (cmd.NuArg() == 0)
    {
        cmd << '\n'
            << "[bool] backup_ice_info "
            << m_solver.BackupIceInfo() << '\n'
            << "[bool] shrink_proofs "
            << m_solver.ShrinkProofs() << '\n'
            << "[bool] use_decompositions "
            << m_solver.UseDecompositions() << '\n'
            << "[bool] use_guifx " 
            << m_solver.UseGuiFx() << '\n'
            << "[string] move_ordering "
            << m_solver.MoveOrdering() << '\n' // FIXME: PRINT NICELY!!
            << "[string] progress_depth "
            << m_solver.ProgressDepth() << '\n'
            << "[string] tt_bits "  
            << m_tt->Bits() << '\n'
            << "[string] update_depth "  
            << m_solver.UpdateDepth() << '\n';
    }
    else if (cmd.NuArg() == 2)
    {
        std::string name = cmd.Arg(0);
        if (name == "backup_ice_info")
            m_solver.SetBackupIceInfo(cmd.BoolArg(1));
        else if (name == "shrink_proofs")
            m_solver.SetShrinkProofs(cmd.BoolArg(1));
        else if (name == "use_decompositions")
            m_solver.SetUseDecompositions(cmd.BoolArg(1));
        else if (name == "use_guifx")
            m_solver.SetUseGuiFx(cmd.BoolArg(1));
        else if (name == "move_ordering")
            m_solver.SetMoveOrdering(cmd.IntArg(1,0,7));
        else if (name == "progress_depth")
            m_solver.SetProgressDepth(cmd.IntArg(1, 0));
	else if (name == "tt_bits")
	{
	    int bits = cmd.IntArg(1, 0);
	    if (bits == 0)
		m_tt.reset(0);
	    else
		m_tt.reset(new SolverTT(bits));
	    m_solver.SetTT(m_tt.get());
	}
        else if (name == "update_depth")
            m_solver.SetUpdateDepth(cmd.IntArg(1, 0));
        else
            throw HtpFailure() << "unknown parameter: " << name;
    }
}

/** Solves the given state.
    Usage: "solve-state [color] { [db-file] { M | T M } }
    (Where M is maximum number of stones in db and T is the maximum
    number of stones for which transpositions are computed.)
*/
void SolverCommands::CmdSolveState(HtpCommand& cmd)
{
    cmd.CheckNuArgLessEqual(4);
    HexColor color = HtpUtil::ColorArg(cmd, 0);

    bool use_db = false;
    std::string filename = "dummy";
    int maxstones = 5;
    int transtones = maxstones;
    if (cmd.NuArg() >= 2) {
        use_db = true;
        filename = cmd.Arg(1);
    }
    if (cmd.NuArg() == 3) {
        maxstones = cmd.IntArg(2, 1);
        transtones = maxstones;
    } else if (cmd.NuArg() == 4) {
        transtones = cmd.IntArg(2, -1);
        maxstones = cmd.IntArg(3, 1);
    }

    double timelimit = -1.0;  // FIXME: WHY ARE THESE HERE?!?!
    int depthlimit = -1;

    HexBoard& brd = m_env.SyncBoard(m_game.Board());

    Solver::SolutionSet solution;
    Solver::Result result = (use_db) ?
        m_solver.Solve(brd, color, filename, maxstones, transtones, solution,
                      depthlimit, timelimit) :
        m_solver.Solve(brd, color, solution, depthlimit, timelimit);

    m_solver.DumpStats(solution);

    HexColor winner = EMPTY;
    if (result != Solver::UNKNOWN) {
        winner = (result==Solver::WIN) ? color : !color;
        LogInfo() << winner << " wins!" << '\n'
		  << brd.Write(solution.proof) << '\n';
    } else {
        LogInfo() << "Search aborted!" << '\n';
    }
    cmd << winner;
}

/** Clears the current TT. */
void SolverCommands::CmdSolverClearTT(HtpCommand& cmd)
{
    UNUSED(cmd);
    m_tt->Clear();
}

/** Finds all winning moves in this state by calling 'solve-state'
    on each child move. 
    Usage: same as 'solve-state'.
*/
void SolverCommands::CmdSolverFindWinning(HtpCommand& cmd)
{
    cmd.CheckNuArgLessEqual(4);
    HexColor color = HtpUtil::ColorArg(cmd, 0);
    HexColor other = !color;

    bool use_db = false;
    std::string filename = "dummy";
    int maxstones = 5; // FIXME: Good?
    int transtones = maxstones;
    if (cmd.NuArg() >= 2)
    {
        use_db = true;
        filename = cmd.Arg(1);
    }

    if (cmd.NuArg() == 3)
    {
        maxstones = cmd.IntArg(2, 1);
        transtones = maxstones;
    }
    else if (cmd.NuArg() == 4)
    {
        transtones = cmd.IntArg(2, -1);
        maxstones = cmd.IntArg(3, 1);
    }

    HexBoard& brd = m_env.SyncBoard(m_game.Board());
    brd.ComputeAll(color);
    bitset_t consider = (PlayerUtils::IsDeterminedState(brd, color) ?
                         brd.getEmpty() :
                         PlayerUtils::MovesToConsider(brd, color));
    bitset_t winning;
    SgTimer timer;

    for (BitsetIterator p(consider); p; ++p)
    {
	if (!consider.test(*p)) continue;

        StoneBoard board(m_game.Board());
        board.playMove(color, *p);

        HexBoard& brd = m_env.SyncBoard(board);

        bitset_t proof;
        std::vector<HexPoint> pv;

        LogInfo() << "****** Trying " << *p << " ******\n" << brd << '\n';

        HexColor winner = EMPTY;
        Solver::SolutionSet solution;
        Solver::Result result = (use_db) 
            ? m_solver.Solve(brd, other, filename, 
                              maxstones, transtones, solution)
            : m_solver.Solve(brd, other, solution);
        m_solver.DumpStats(solution);
        LogInfo() << "Proof:" << brd.Write(solution.proof) << '\n';

        if (result != Solver::UNKNOWN) {
            winner = (result==Solver::WIN) ? !color : color;
            LogInfo() << "****** " << winner << " wins ******\n";
        } else {
            LogInfo() << "****** unknown ******\n";
        }

        if (winner == color) {
            winning.set(*p);
        } else {
	    consider &= solution.proof;
	}
    }
    LogInfo() << "****** Winning Moves ******\n"
              << m_game.Board().Write(winning) << '\n';
    LogInfo() << "Total Elapsed Time: " << timer.GetTime() << '\n';
    cmd << HexPointUtil::ToPointListString(winning);
}

//----------------------------------------------------------------------------

/** Opens a database. 
    Usage: "db-open [filename] { M | T M }"
*/
void SolverCommands::CmdDBOpen(HtpCommand& cmd)
{
    cmd.CheckNuArgLessEqual(3);
    std::string filename = cmd.Arg(0);
    int maxstones = -1;
    int transtones = -1;

    if (cmd.NuArg() == 2) {
        maxstones = cmd.IntArg(1, 1);
        transtones = maxstones;
    } else if (cmd.NuArg() == 3) {
        transtones = cmd.IntArg(1, -1);
        maxstones = cmd.IntArg(2, 1);
    }

    const StoneBoard& brd = m_game.Board();

    m_db.reset(new SolverDB());
    try {
        if (maxstones == -1)
            m_db->open(brd.width(), brd.height(), filename);
        else
            m_db->open(brd.width(), brd.height(),  maxstones,
                       transtones, filename);
    }
    catch (HexException& e) {
        m_db.reset(0);
        throw HtpFailure() << "Error opening db: '" << e.what() << "'\n";
    }
}

/** Closes an open database. */
void SolverCommands::CmdDBClose(HtpCommand& cmd)
{
    cmd.CheckNuArg(0);
    m_db.reset(0);
}

/** Dumps info from db on current state. */
void SolverCommands::CmdDBGet(HtpCommand& cmd)
{
    cmd.CheckNuArg(0);
    if (!m_db) 
        throw HtpFailure() << "No open database.";

    StoneBoard brd(m_game.Board());
    HexColor toplay = brd.WhoseTurn();
    SolvedState state;

    if (!m_db->get(brd, state)) 
    {
        cmd << "State not in database.";
        return;
    }

    // dump winner and proof
    cmd << (state.win ? toplay : !toplay);
    cmd << ' ' << state.nummoves;
    cmd << HexPointUtil::ToPointListString(state.proof);

    std::vector<int> nummoves(BITSETSIZE);
    std::vector<int> flags(BITSETSIZE);
    std::vector<HexPoint> winning, losing;
    for (BitsetIterator p(brd.getEmpty()); p; ++p) 
    {
        brd.playMove(toplay, *p);

        if (m_db->get(brd, state)) 
        {
            if (state.win)
                losing.push_back(*p);
            else
                winning.push_back(*p);
            nummoves[*p] = state.nummoves;
            flags[*p] = state.flags;
        }
        brd.undoMove(*p);
    }

    cmd << " Winning";
    for (unsigned i = 0; i < winning.size(); ++i) 
    {
        cmd << " " << HexPointUtil::toString(winning[i]);
        cmd << " " << nummoves[winning[i]];
        if (flags[winning[i]] & SolvedState::FLAG_MIRROR_TRANSPOSITION)
            cmd << "m";
        else if (flags[winning[i]] & SolvedState::FLAG_TRANSPOSITION)
            cmd << "t";
    }

    cmd << " Losing";
    for (unsigned i = 0; i < losing.size(); ++i)
    {
        cmd << " " << HexPointUtil::toString(losing[i]);
        cmd << " " << nummoves[losing[i]];
        if (flags[losing[i]] & SolvedState::FLAG_MIRROR_TRANSPOSITION)
            cmd << "m";
        else if (flags[losing[i]] & SolvedState::FLAG_TRANSPOSITION)
            cmd << "t";
    }
}

//----------------------------------------------------------------------------
