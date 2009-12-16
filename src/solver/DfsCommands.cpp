//----------------------------------------------------------------------------
/** @file SolverCommands.cpp
 */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "SgTimer.h"
#include "BitsetIterator.hpp"
#include "PlayerUtils.hpp"
#include "DfsCommands.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

SolverCommands::SolverCommands(Game& game, HexEnvironment& env,
                               DfsSolver& solver,
                               boost::scoped_ptr<DfsHashTable>& solverTT, 
                               boost::scoped_ptr<DfsDB>& solverDB)
    : m_game(game), 
      m_env(env),
      m_solver(solver),
      m_tt(solverTT),
      m_db(solverDB)
{
}

void SolverCommands::Register(GtpEngine& e)
{
    Register(e, "param_dfs", &SolverCommands::CmdParamSolver);
    Register(e, "dfs-solve-state", &SolverCommands::CmdSolveState);
    Register(e, "dfs-clear-tt", &SolverCommands::CmdSolverClearTT);
    Register(e, "dfs-solver-find-winning", 
             &SolverCommands::CmdSolverFindWinning);

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
		m_tt.reset(new DfsHashTable(bits));
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

    int maxStones = 5;
    int transStones = maxStones;
    boost::scoped_ptr<DfsDB> db(0);
    if (cmd.NuArg() >= 2) 
    {
        std::string filename = cmd.Arg(1);
        db.reset(new DfsDB(filename));
    }
    if (cmd.NuArg() == 3) 
    {
        maxStones = cmd.IntArg(2, 1);
        transStones = maxStones;
    } 
    else if (cmd.NuArg() == 4) 
    {
        transStones = cmd.IntArg(2, -1);
        maxStones = cmd.IntArg(3, 1);
    }

    double timelimit = -1.0;
    int depthlimit = -1;

    HexBoard& brd = m_env.SyncBoard(m_game.Board());

    DfsSolver::SolutionSet solution;
    DfsSolver::Result result = 
        m_solver.Solve(brd, color, db.get(), maxStones, transStones, solution,
                       depthlimit, timelimit);
    m_solver.DumpStats(solution);

    HexColor winner = EMPTY;
    if (result != DfsSolver::UNKNOWN) 
    {
        winner = (result == DfsSolver::WIN) ? color : !color;
        LogInfo() << winner << " wins!\n"
                  << brd.Write(solution.proof) << '\n';
    } 
    else 
        LogInfo() << "Search aborted!\n";
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

    boost::scoped_ptr<DfsDB> db(0);
    int maxStones = 5;
    int transStones = maxStones;
    if (cmd.NuArg() >= 2)
    {
        std::string filename = cmd.Arg(1);
        db.reset(new DfsDB(filename));
    }

    if (cmd.NuArg() == 3)
    {
        maxStones = cmd.IntArg(2, 1);
        transStones = maxStones;
    }
    else if (cmd.NuArg() == 4)
    {
        transStones = cmd.IntArg(2, -1);
        maxStones = cmd.IntArg(3, 1);
    }

    HexBoard& brd = m_env.SyncBoard(m_game.Board());
    brd.ComputeAll(color);
    bitset_t consider = (PlayerUtils::IsDeterminedState(brd, color) ?
                         brd.GetState().GetEmpty() :
                         PlayerUtils::MovesToConsider(brd, color));
    bitset_t winning;
    SgTimer timer;

    for (BitsetIterator p(consider); p; ++p)
    {
	if (!consider.test(*p)) continue;

        StoneBoard board(m_game.Board());
        board.PlayMove(color, *p);

        HexBoard& brd = m_env.SyncBoard(board);

        LogInfo() << "****** Trying " << *p << " ******\n" << brd << '\n';

        HexColor winner = EMPTY;
        DfsSolver::SolutionSet solution;
        DfsSolver::Result result = m_solver.Solve(brd, other, db.get(), 
                                                  maxStones, transStones, 
                                                  solution);
        m_solver.DumpStats(solution);
        LogInfo() << "Proof:" << brd.Write(solution.proof) << '\n';

        if (result != DfsSolver::UNKNOWN) 
        {
            winner = (result == DfsSolver::WIN) ? !color : color;
            LogInfo() << "****** " << winner << " wins ******\n";
        } 
        else 
            LogInfo() << "****** unknown ******\n";

        if (winner == color)
            winning.set(*p);
        else
	    consider &= solution.proof;
    }
    LogInfo() << "****** Winning Moves ******\n"
              << m_game.Board().Write(winning) << '\n';
    LogInfo() << "Total Elapsed Time: " << timer.GetTime() << '\n';
    cmd << HexPointUtil::ToString(winning);
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
    if (cmd.NuArg() == 2) 
    {
        maxstones = cmd.IntArg(1, 1);
        transtones = maxstones;
    } 
    else if (cmd.NuArg() == 3) 
    {
        transtones = cmd.IntArg(1, -1);
        maxstones = cmd.IntArg(2, 1);
    }

    try {
        m_db.reset(new DfsDB(filename));
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
    DfsData state;
    if (!m_db->Get(brd, state)) 
    {
        cmd << "State not in database.";
        return;
    }

    // dump winner and proof
    cmd << (state.win ? toplay : !toplay);
    cmd << ' ' << state.nummoves;

    std::vector<int> nummoves(BITSETSIZE);
    std::vector<int> flags(BITSETSIZE);
    std::vector<HexPoint> winning, losing;
    for (BitsetIterator p(brd.GetEmpty()); p; ++p) 
    {
        brd.PlayMove(toplay, *p);

        if (m_db->Get(brd, state)) 
        {
            if (state.win)
                losing.push_back(*p);
            else
                winning.push_back(*p);
            nummoves[*p] = state.nummoves;
            flags[*p] = state.flags;
        }
        brd.UndoMove(*p);
    }

    cmd << " Winning";
    for (unsigned i = 0; i < winning.size(); ++i) 
    {
        cmd << " " << winning[i];
        cmd << " " << nummoves[winning[i]];
        if (flags[winning[i]] & DfsData::FLAG_MIRROR_TRANSPOSITION)
            cmd << "m";
        else if (flags[winning[i]] & DfsData::FLAG_TRANSPOSITION)
            cmd << "t";
    }

    cmd << " Losing";
    for (unsigned i = 0; i < losing.size(); ++i)
    {
        cmd << " " << losing[i];
        cmd << " " << nummoves[losing[i]];
        if (flags[losing[i]] & DfsData::FLAG_MIRROR_TRANSPOSITION)
            cmd << "m";
        else if (flags[losing[i]] & DfsData::FLAG_TRANSPOSITION)
            cmd << "t";
    }
}

//----------------------------------------------------------------------------
