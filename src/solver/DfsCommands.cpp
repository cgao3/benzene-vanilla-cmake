//----------------------------------------------------------------------------
/** @file DfsCommands.cpp
 */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "SgTimer.h"
#include "BitsetIterator.hpp"
#include "PlayerUtils.hpp"
#include "DfsCommands.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

DfsCommands::DfsCommands(Game& game, HexEnvironment& env,
                         DfsSolver& solver, 
                         boost::scoped_ptr<DfsHashTable>& hashTable, 
                         boost::scoped_ptr<DfsDB>& db, 
                         DfsPositions& positions)
    : m_game(game), 
      m_env(env),
      m_solver(solver),
      m_tt(hashTable),
      m_db(db),
      m_positions(positions)
{
}

void DfsCommands::Register(GtpEngine& e)
{
    Register(e, "param_dfs", &DfsCommands::CmdParamSolver);
    Register(e, "param_dfs_db", &DfsCommands::CmdParamSolverDB);
    Register(e, "dfs-solve-state", &DfsCommands::CmdSolveState);
    Register(e, "dfs-clear-tt", &DfsCommands::CmdSolverClearTT);
    Register(e, "dfs-solver-find-winning", 
             &DfsCommands::CmdSolverFindWinning);
    Register(e, "dfs-get-state", &DfsCommands::CmdGetState);
    Register(e, "dfs-open-db", &DfsCommands::CmdDBOpen);
    Register(e, "dfs-close-db", &DfsCommands::CmdDBClose);
}

void DfsCommands::Register(GtpEngine& engine, const std::string& command,
                              GtpCallback<DfsCommands>::Method method)
{
    engine.Register(command, new GtpCallback<DfsCommands>(this, method));
}

//----------------------------------------------------------------------------

void DfsCommands::CmdParamSolverDB(HtpCommand& cmd)
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

void DfsCommands::CmdParamSolver(HtpCommand& cmd)
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
	else if (name == "tt_bits")
	{
	    int bits = cmd.IntArg(1, 0);
	    if (bits == 0)
		m_tt.reset(0);
	    else
		m_tt.reset(new DfsHashTable(bits));
	}
        else if (name == "update_depth")
            m_solver.SetUpdateDepth(cmd.IntArg(1, 0));
        else
            throw HtpFailure() << "unknown parameter: " << name;
    }
}

/** Solves the given state.
    Usage: "solve-state [color to play]
*/
void DfsCommands::CmdSolveState(HtpCommand& cmd)
{
    cmd.CheckNuArg(1);
    HexColor color = HtpUtil::ColorArg(cmd, 0);
    HexBoard& brd = m_env.SyncBoard(m_game.Board());
    if (brd.ICE().FindPermanentlyInferior())
        throw HtpFailure("Permanently inferior not supported in DfsSolver.");
    DfsSolutionSet solution;
    HexColor winner = m_solver.Solve(brd, color, solution, m_positions);
    m_solver.DumpStats(solution);
    if (winner != EMPTY) 
        LogInfo() << winner << " wins!\n" << brd.Write(solution.proof) << '\n';
    else 
        LogInfo() << "Search aborted!\n";
    cmd << winner;
}

/** Clears the current TT. */
void DfsCommands::CmdSolverClearTT(HtpCommand& cmd)
{
    UNUSED(cmd);
    m_tt->Clear();
}

/** Finds all winning moves in this state by calling 'solve-state'
    on each child move. 
    Usage: same as 'solve-state'.
*/
void DfsCommands::CmdSolverFindWinning(HtpCommand& cmd)
{
    cmd.CheckNuArg(1);
    HexColor color = HtpUtil::ColorArg(cmd, 0);
    HexBoard& brd = m_env.SyncBoard(m_game.Board());
    if (brd.ICE().FindPermanentlyInferior())
        throw HtpFailure("Permanently inferior not supported in DfsSolver");
    brd.ComputeAll(color);
    bitset_t consider = (PlayerUtils::IsDeterminedState(brd, color) ?
                         brd.GetState().GetEmpty() :
                         PlayerUtils::MovesToConsider(brd, color));
    bitset_t winning;
    SgTimer timer;
    for (BitsetIterator p(consider); p; ++p)
    {
	if (!consider.test(*p))
            continue;
        StoneBoard board(m_game.Board());
        board.PlayMove(color, *p);
        HexBoard& brd = m_env.SyncBoard(board);
        LogInfo() << "****** Trying " << *p << " ******\n" << brd << '\n';
        DfsSolutionSet solution;
        HexColor winner = m_solver.Solve(brd, !color, solution, m_positions);
        m_solver.DumpStats(solution);
        LogInfo() << "Proof:" << brd.Write(solution.proof) << '\n';

        if (winner != EMPTY) 
            LogInfo() << "****** " << winner << " wins ******\n";
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
    Usage: "db-open [filename]"
*/
void DfsCommands::CmdDBOpen(HtpCommand& cmd)
{
    cmd.CheckNuArgLessEqual(3);
    std::string filename = cmd.Arg(0);
    try {
        m_db.reset(new DfsDB(filename));
    }
    catch (BenzeneException& e) {
        m_db.reset(0);
        throw HtpFailure() << "Error opening db: '" << e.what() << "'\n";
    }
}

/** Closes an open database. */
void DfsCommands::CmdDBClose(HtpCommand& cmd)
{
    cmd.CheckNuArg(0);
    if (m_db.get() == 0)
        throw HtpFailure("No open database!\n");
    m_db.reset(0);
}

/** Dumps info from on current state. */
void DfsCommands::CmdGetState(HtpCommand& cmd)
{
    cmd.CheckNuArg(0);
    StoneBoard brd(m_game.Board());
    HexColor toplay = brd.WhoseTurn();
    DfsData state;
    if (!m_positions.Get(brd, state)) 
    {
        cmd << "State not available.";
        return;
    }
    cmd << (state.m_win ? toplay : !toplay);
    cmd << ' ' << state.m_numMoves;

    std::vector<int> nummoves(BITSETSIZE);
    std::vector<int> flags(BITSETSIZE);
    std::vector<HexPoint> winning, losing;
    for (BitsetIterator p(brd.GetEmpty()); p; ++p) 
    {
        brd.PlayMove(toplay, *p);
        if (m_positions.Get(brd, state)) 
        {
            if (state.m_win)
                losing.push_back(*p);
            else
                winning.push_back(*p);
            nummoves[*p] = state.m_numMoves;
            flags[*p] = state.m_flags;
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
