//----------------------------------------------------------------------------
/** @file BenzeneHtpEngine.cpp
 */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "SgGameReader.h"

#include <cmath>
#include <functional>
#include "BoardUtils.hpp"
#include "BookCheck.hpp"
#include "BitsetIterator.hpp"
#include "EndgameCheck.hpp"
#include "GraphUtils.hpp"
#include "HandBookCheck.hpp"
#include "HexProgram.hpp"
#include "HexSgUtil.hpp"
#include "BenzeneHtpEngine.hpp"
#include "PlayerUtils.hpp"
#include "Resistance.hpp"
#include "Solver.hpp"
#include "SolverDB.hpp"
#include "SwapCheck.hpp"
#include "TwoDistance.hpp"
#include "VCSet.hpp"
#include "VCUtils.hpp"
#include "VulPreCheck.hpp"
#include "PlayAndSolve.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

BenzeneHtpEngine::BenzeneHtpEngine(GtpInputStream& in, GtpOutputStream& out,
                                   int boardsize, BenzenePlayer& player)
    : HexHtpEngine(in, out, boardsize),
      m_player(player),
      m_pe(m_board.Width(), m_board.Height()),
      m_se(m_board.Width(), m_board.Height()),
      m_solver(),
      m_solverDfpn(),
      m_solver_tt(new SolverTT(20)), // TT with 2^20 entries
      m_dfpn_tt(new DfpnHashTable(20)), // TT with 2^20 entries
      m_db(0),
      m_playerEnvCommands(m_pe),
      m_solverEnvCommands(m_se),
      m_vcCommands(m_game, m_pe),
      m_solverCommands(m_game, m_se, m_solver, m_solver_tt, m_db),
      m_dfpnCommands(m_game, m_se, m_solverDfpn, m_dfpn_tt),
      m_useParallelSolver(false)
{
    RegisterCmd("benzene-license", &BenzeneHtpEngine::CmdLicense);

    RegisterCmd("reg_genmove", &BenzeneHtpEngine::CmdRegGenMove);

    RegisterCmd("get_absorb_group", &BenzeneHtpEngine::CmdGetAbsorbGroup);

    RegisterCmd("handbook-add", &BenzeneHtpEngine::CmdHandbookAdd);

    RegisterCmd("compute-inferior", &BenzeneHtpEngine::CmdComputeInferior);
    RegisterCmd("compute-fillin", &BenzeneHtpEngine::CmdComputeFillin);
    RegisterCmd("compute-vulnerable", &BenzeneHtpEngine::CmdComputeVulnerable);
    RegisterCmd("compute-reversible", &BenzeneHtpEngine::CmdComputeReversible);
    RegisterCmd("compute-dominated", &BenzeneHtpEngine::CmdComputeDominated);
    RegisterCmd("find-comb-decomp", &BenzeneHtpEngine::CmdFindCombDecomp);
    RegisterCmd("find-split-decomp", &BenzeneHtpEngine::CmdFindSplitDecomp);
    RegisterCmd("encode-pattern", &BenzeneHtpEngine::CmdEncodePattern);

    m_playerEnvCommands.Register(*this, "player");
    m_solverEnvCommands.Register(*this, "solver");
    m_vcCommands.Register(*this);
    m_solverCommands.Register(*this);
    m_dfpnCommands.Register(*this);

    RegisterCmd("param_player", &BenzeneHtpEngine::CmdParamPlayer);

    RegisterCmd("eval-twod", &BenzeneHtpEngine::CmdEvalTwoDist);
    RegisterCmd("eval-resist", &BenzeneHtpEngine::CmdEvalResist);
    RegisterCmd("eval-resist-delta", &BenzeneHtpEngine::CmdEvalResistDelta);
    RegisterCmd("eval-influence", &BenzeneHtpEngine::CmdEvalInfluence);

    RegisterCmd("misc-debug", &BenzeneHtpEngine::CmdMiscDebug);

    // Set some defaults
    m_solver.SetTT(m_solver_tt.get());
}

BenzeneHtpEngine::~BenzeneHtpEngine()
{
}

//----------------------------------------------------------------------------

void BenzeneHtpEngine::RegisterCmd(const std::string& name,
                               GtpCallback<BenzeneHtpEngine>::Method method)
{
    Register(name, new GtpCallback<BenzeneHtpEngine>(this, method));
}

void BenzeneHtpEngine::NewGame(int width, int height)
{
    HexHtpEngine::NewGame(width, height);

    m_pe.NewGame(width, height);
    m_se.NewGame(width, height);
}

/** Generates a move. */
HexPoint BenzeneHtpEngine::GenMove(HexColor color, double maxTime)
{
    if (m_useParallelSolver)
    {
        PlayAndSolve ps(*m_pe.brd, *m_se.brd, m_player, m_solverDfpn, 
                        *m_dfpn_tt, m_game);
        return ps.GenMove(color, maxTime);
    }
    double score;
    return m_player.genmove(m_pe.SyncBoard(m_game.Board()), m_game, 
                            color, maxTime, score);
}

////////////////////////////////////////////////////////////////////////
// Commands
////////////////////////////////////////////////////////////////////////

/** Displays usage license. */
void BenzeneHtpEngine::CmdLicense(HtpCommand& cmd)
{
    cmd << 
        HexProgram::Get().getName() << " " <<
        HexProgram::Get().getVersion() << " " <<
        HexProgram::Get().getDate() << "\n"
        "Copyright (C) 2009 by the authors of the Benzene project.\n"
        "This version is for private use only. DO NOT DISTRIBUTE.\n\n";
}

/** Generates a move, but does not play it. */
void BenzeneHtpEngine::CmdRegGenMove(HtpCommand& cmd)
{
    cmd.CheckNuArg(1);
    double score;
    HexPoint move = m_player.genmove(m_pe.SyncBoard(m_game.Board()),
                                     m_game, HtpUtil::ColorArg(cmd, 0),
                                     -1, score);
    cmd << move;
}

/** Returns the set of stones this stone is part of. */
void BenzeneHtpEngine::CmdGetAbsorbGroup(HtpCommand& cmd)
{
    cmd.CheckNuArg(1);
    HexPoint cell = HtpUtil::MoveArg(cmd, 0);
    if (m_game.Board().GetColor(cell) == EMPTY)
        return;

    Groups groups;
    GroupBuilder::Build(m_game.Board(), groups);

    const Group& group = groups.GetGroup(cell);
    cmd << group.Captain();
    for (BitsetIterator p(group.Members()); p; ++p) 
        if (*p != group.Captain()) 
            cmd << ' ' << *p;
}

//---------------------------------------------------------------------------

void BenzeneHtpEngine::ParamPlayer(BenzenePlayer* player, HtpCommand& cmd)
{
    using namespace benzene::BenzenePlayerUtil;
    BookCheck* book = GetInstanceOf<BookCheck>(player);
    EndgameCheck* endgame = GetInstanceOf<EndgameCheck>(player);
    HandBookCheck* handbook = GetInstanceOf<HandBookCheck>(player);
    
    if (cmd.NuArg() == 0)
    {
        cmd << '\n';
        if (endgame)
            cmd << "[bool] search_singleton "
                << endgame->SearchSingleton() << '\n';
        if (book) 
            cmd << "[bool] use_book "
                << book->Enabled() << '\n';
        if (endgame) 
            cmd << "[bool] use_endgame_check "
                << endgame->Enabled() << '\n';
        if (handbook)
            cmd << "[bool] use_handbook "
                << handbook->Enabled() << '\n';
        cmd << "[bool] use_parallel_solver "
            << m_useParallelSolver << '\n';
        if (book) 
        {
            cmd << "[string] book_count_weight "
                << book->CountWeight() << '\n'
                << "[string] book_min_count "
                << book->MinCount() << '\n'
                << "[string] book_name "
                << book->BookName() << '\n';
        }
    }
    else if (cmd.NuArg() == 2)
    {
        std::string name = cmd.Arg(0);

        if (book && name == "book_min_count")
            book->SetMinCount(cmd.SizeTypeArg(1, 0));
        else if (book && name == "book_count_weight")
            book->SetCountWeight(cmd.FloatArg(1));
        else if (book && name == "book_name")
            book->SetBookName(cmd.Arg(1));
	else if (book && name == "use_book")
	    book->SetEnabled(cmd.BoolArg(1));
        else if (endgame && name == "search_singleton")
            endgame->SetSearchSingleton(cmd.BoolArg(1));
        else if (endgame && name == "use_endgame_check") 
            endgame->SetEnabled(cmd.BoolArg(1));
        else if (handbook && name == "use_handbook")
            handbook->SetEnabled(cmd.BoolArg(1));
        else if (name == "use_parallel_solver")
            m_useParallelSolver = cmd.BoolArg(1);
    }
    else 
        throw HtpFailure("Expected 0 ore 2 arguments");
}

void BenzeneHtpEngine::CmdParamPlayer(HtpCommand& cmd)
{
    ParamPlayer(&m_player, cmd);
}

//----------------------------------------------------------------------

/** Pulls moves out of the game for given color and appends them to
    the given handbook file. Skips the first move (ie, the move from
    the empty board). Performs no duplicate checking.
    Usage: 
      handbook-add [handbook.txt] [sgf file] [color] [max move #] 
*/
void BenzeneHtpEngine::CmdHandbookAdd(HtpCommand& cmd)
{
    cmd.CheckNuArg(4);
    std::string bookfilename = cmd.Arg(0);
    std::string sgffilename = cmd.Arg(1);
    HexColor colorToSave = HtpUtil::ColorArg(cmd, 2);
    int maxMove = cmd.IntArg(3, 0);
    
    std::ifstream sgffile(sgffilename.c_str());
    if (!sgffile)
        throw HtpFailure() << "cannot load sgf";

    SgGameReader sgreader(sgffile, 11);
    SgNode* root = sgreader.ReadGame(); 
    if (root == 0)
        throw HtpFailure() << "cannot load file";
    sgreader.PrintWarnings(std::cerr);

    if (HexSgUtil::NodeHasSetupInfo(root)) 
        throw HtpFailure() << "Root has setup info!";

    int size = root->GetIntProp(SG_PROP_SIZE);
    if (size != m_game.Board().Width() || 
        size != m_game.Board().Height())
        throw HtpFailure() << "Sgf boardsize does not match board";

    StoneBoard brd(m_game.Board());
    HexColor color = FIRST_TO_PLAY;
    PointSequence responses;
    std::vector<hash_t> hashes;
    SgNode* cur = root;
    for (int moveNum = 0; moveNum < maxMove;) 
    {
        cur = cur->NodeInDirection(SgNode::NEXT);
        if (!cur) 
            break;

        if (HexSgUtil::NodeHasSetupInfo(cur)) 
            throw HtpFailure() << "Node has setup info";

        // SgGameReader does not support reading "resign" moves from
        // an sgf, so any such node will have no move. This should not
        // be treated as an error if it is the last node in the game.
        // This isn't exact, but close enough.
        if (!cur->HasNodeMove() && !cur->HasSon())
            break;

        // If node does not have a move and is *not* the last node in
        // the game, then this sgf should not be passed in here.
        if (!cur->HasNodeMove()) 
            throw HtpFailure() << "Node has no move";

        HexColor sgfColor = HexSgUtil::SgColorToHexColor(cur->NodePlayer());
        HexPoint sgfPoint = HexSgUtil::SgPointToHexPoint(cur->NodeMove(), 
                                                         brd.Height());
        if (color != sgfColor)
            throw HtpFailure() << "Unexpected color to move";

        if (moveNum && color == colorToSave)
        {
            hashes.push_back(brd.Hash());
            responses.push_back(sgfPoint);
        }
        brd.PlayMove(color, sgfPoint);
        color = !color;
        ++moveNum;
    }
    HexAssert(hashes.size() == responses.size());
 
    std::ofstream out(bookfilename.c_str(), std::ios_base::app);
    for (std::size_t i = 0 ; i < hashes.size(); ++i)
        out << HashUtil::toString(hashes[i]) << ' ' << responses[i] << '\n';
    out.close();
}

//----------------------------------------------------------------------

/** Outputs inferior cell info for current state.
    Usage: "compute-inferior [color]"
 */
void BenzeneHtpEngine::CmdComputeInferior(HtpCommand& cmd)
{
    cmd.CheckNuArg(1);
    HexColor color = HtpUtil::ColorArg(cmd, 0);
    HexBoard& brd = m_pe.SyncBoard(m_game.Board());
    brd.GetPatternState().Update();
    GroupBuilder::Build(brd.GetState(), brd.GetGroups());
    InferiorCells inf;
    m_pe.ice.ComputeInferiorCells(color, brd.GetGroups(), 
                                  brd.GetPatternState(), inf);
    cmd << inf.GuiOutput();
    cmd << '\n';
}

/** Computes fillin for the given board. Color argument affects order
    for computing vulnerable/presimplicial pairs. */
void BenzeneHtpEngine::CmdComputeFillin(HtpCommand& cmd)
{
    cmd.CheckNuArg(1);
    HexColor color = HtpUtil::ColorArg(cmd, 0);
    HexBoard& brd = m_pe.SyncBoard(m_game.Board());
    brd.GetPatternState().Update();
    GroupBuilder::Build(brd.GetState(), brd.GetGroups());
    InferiorCells inf;
    m_pe.ice.ComputeFillin(color, brd.GetGroups(), brd.GetPatternState(), inf);
    inf.ClearVulnerable();
    cmd << inf.GuiOutput();
    cmd << '\n';
}

/** Computes vulnerable cells on the current board for the given color. */
void BenzeneHtpEngine::CmdComputeVulnerable(HtpCommand& cmd)
{
    cmd.CheckNuArg(1);
    HexColor col = HtpUtil::ColorArg(cmd, 0);
    HexBoard& brd = m_pe.SyncBoard(m_game.Board());
    brd.GetPatternState().Update();
    GroupBuilder::Build(brd.GetState(), brd.GetGroups());
    InferiorCells inf;
    m_pe.ice.FindVulnerable(brd.GetPatternState(), col, 
                            brd.GetState().GetEmpty(), inf);
    cmd << inf.GuiOutput();
    cmd << '\n';
}

/** Computes reversible cells on the current board for the given color. */
void BenzeneHtpEngine::CmdComputeReversible(HtpCommand& cmd)
{
    cmd.CheckNuArg(1);
    HexColor col = HtpUtil::ColorArg(cmd, 0);
    HexBoard& brd = m_pe.SyncBoard(m_game.Board());
    brd.GetPatternState().Update();
    GroupBuilder::Build(brd.GetState(), brd.GetGroups());
    InferiorCells inf;
    m_pe.ice.FindReversible(brd.GetPatternState(), col, 
                            brd.GetState().GetEmpty(), inf);
    cmd << inf.GuiOutput();
    cmd << '\n';
}

/** Computes dominated cells on the current board for the given color. */
void BenzeneHtpEngine::CmdComputeDominated(HtpCommand& cmd)
{
    cmd.CheckNuArg(1);
    HexColor col = HtpUtil::ColorArg(cmd, 0);
    HexBoard& brd = m_pe.SyncBoard(m_game.Board());
    brd.GetPatternState().Update();
    GroupBuilder::Build(brd.GetState(), brd.GetGroups());
    InferiorCells inf;
    m_pe.ice.FindDominated(brd.GetPatternState(), col, 
                           brd.GetState().GetEmpty(), inf);
    cmd << inf.GuiOutput();
    cmd << '\n';
}

/** Tries to find a combinatorial decomposition of the board state.
    Outputs cells in the vc if there is a decomposition.
    Usage: 'find-comb-decomp [color]'
*/
void BenzeneHtpEngine::CmdFindCombDecomp(HtpCommand& cmd)
{
    cmd.CheckNuArg(1);
    HexColor color = HtpUtil::ColorArg(cmd, 0);
    HexBoard& brd = m_pe.SyncBoard(m_game.Board());
    // Turn of decomps in the board, then call ComputeAll(). Otherwise
    // decomps will be found and filled-in by ComputeAll().
    bool useDecomps = brd.UseDecompositions();
    brd.SetUseDecompositions(false);
    brd.ComputeAll(BLACK);
    brd.SetUseDecompositions(useDecomps);
    bitset_t capturedVC;
    if (BoardUtils::FindCombinatorialDecomposition(brd, color, capturedVC)) 
        cmd << HexPointUtil::ToString(capturedVC);
}

/** Tries to find a group that crowds both opponent edges. Outputs
    group that crowds both edges if one exists.  
    Usage: 'find-split-decomp [color]'

    FIXME: Dump inferior cell info as well? It's hard to see what's
    actually going on if it is not displayed.
*/
void BenzeneHtpEngine::CmdFindSplitDecomp(HtpCommand& cmd)
{
    cmd.CheckNuArg(1);
    HexColor color = HtpUtil::ColorArg(cmd, 0);
    HexBoard& brd = m_pe.SyncBoard(m_game.Board());
    brd.ComputeAll(BLACK);
    HexPoint group;
    if (BoardUtils::FindSplittingDecomposition(brd, color, group))
        cmd << group;
}

/** Outputs pattern in encoded form. 
    Takes a list of cells, the first cell being the center of the
    pattern (that is not actually in the pattern).
    FIXME: clean this up!
*/
void BenzeneHtpEngine::CmdEncodePattern(HtpCommand& cmd)
{
    HexAssert(cmd.NuArg() > 0);

    // Build direction offset look-up matrix.
    int xoffset[Pattern::NUM_SLICES][32];
    int yoffset[Pattern::NUM_SLICES][32];
    for (int s=0; s<Pattern::NUM_SLICES; s++)
    {
        int fwd = s;
        int lft = (s + 2) % NUM_DIRECTIONS;
        int x1 = HexPointUtil::DeltaX(fwd);
        int y1 = HexPointUtil::DeltaY(fwd);
        for (int i=1, g=0; i<=Pattern::MAX_EXTENSION; i++)
        {
            int x2 = x1;
            int y2 = y1;
            for (int j=0; j<i; j++)
            {
                xoffset[s][g] = x2;
                yoffset[s][g] = y2;
                x2 += HexPointUtil::DeltaX(lft);
                y2 += HexPointUtil::DeltaY(lft);
                g++;
            }
            x1 += HexPointUtil::DeltaX(fwd);
            y1 += HexPointUtil::DeltaY(fwd);
        }
    }

    int pattOut[Pattern::NUM_SLICES * 5];
    memset(pattOut, 0, sizeof(pattOut));
    StoneBoard brd(m_game.Board());
    HexPoint center = HtpUtil::MoveArg(cmd, 0);
    LogInfo() << "Center of pattern: " << center << '\n' << "Includes: ";
    int x1, y1, x2, y2;
    HexPointUtil::pointToCoords(center, x1, y1);
    std::size_t i = 1;
    while (i < cmd.NuArg())
    {
        HexPoint p = HtpUtil::MoveArg(cmd, i++);
        HexPointUtil::pointToCoords(p, x2, y2);
        x2 = x2 - x1;
        y2 = y2 - y1;
        int sliceNo;
        if (y2 > 0)
        {
            if ((x2 + y2) < 0)          // Point is in bottom of 4th slice
                sliceNo = 3;
            else if ((x2 < 0))          // Point is in 5th slice
                sliceNo = 4;
            else                        // point is in 6th slice
                sliceNo = 5;
        }
        else
        {
            if ((x2 + y2) > 0)          // Point is in 1st slice
                sliceNo = 0;
            else if (x2 > 0)            // Point is in 2nd slice
                sliceNo = 1;
            else if (x2 < 0 && y2 == 0) // Point is in upper part of 4th slice
                sliceNo = 3;
            else                        // Point is in 3rd slice
                sliceNo = 2;
        }
        int j = 0;
        while (j < 32 && (xoffset[sliceNo][j] != x2 ||
                          yoffset[sliceNo][j] != y2))
            j++;
        HexAssert(j != 32);
        pattOut[sliceNo*5] += (1 << j);

        if (brd.IsBlack(p))
            pattOut[(sliceNo*5) + 1] += (1 << j);
        else if (brd.IsWhite(p))
            pattOut[(sliceNo*5) + 2] += (1 << j);
        LogInfo() << p << ":" << brd.GetColor(p) << ", ";
    }
    LogInfo() << '\n';
    
    std::string encPattStr = "d:";

    for (int k = 0; k < Pattern::NUM_SLICES; k++)
    {
        for (int l = 0; l < 4; l++)
        {
            std::stringstream out; //FIXME: Isn't there a better way??
           out << (pattOut[(k*5) + l]) << ",";
           encPattStr.append(out.str());
        }
           std::stringstream out;
           out << (pattOut[(k*5) + 4]) << ";";
           encPattStr.append(out.str());
    }
    LogInfo() << encPattStr << '\n';
}

//----------------------------------------------------------------------------
// Evaluation commands
//----------------------------------------------------------------------------

void BenzeneHtpEngine::CmdEvalTwoDist(HtpCommand& cmd)
{
    cmd.CheckNuArg(1);
    HexColor color = HtpUtil::ColorArg(cmd, 0);

    HexBoard& brd = m_pe.SyncBoard(m_game.Board());
    brd.ComputeAll(color);
    TwoDistance twod(TwoDistance::ADJACENT);
    twod.Evaluate(brd);

    for (BoardIterator it(brd.Const().Interior()); it; ++it) 
    {
        if (brd.GetState().IsOccupied(*it)) continue;
        HexEval energy = twod.Score(*it, color);
        if (energy == EVAL_INFINITY)
            energy = -1;
        cmd << " " << *it << " " << energy;
    }
}

void BenzeneHtpEngine::CmdEvalResist(HtpCommand& cmd)
{
    cmd.CheckNuArg(1);
    HexColor color = HtpUtil::ColorArg(cmd, 0);

    HexBoard& brd = *m_pe.brd;
    Resistance resist;
    resist.Evaluate(brd);

    cmd << " res " << std::fixed << std::setprecision(3) << resist.Score()
        << " rew " << std::fixed << std::setprecision(3) << resist.Resist(WHITE)
        << " reb " << std::fixed << std::setprecision(3) << resist.Resist(BLACK);

    for (BoardIterator it(brd.Const().Interior()); it; ++it) {
        if (brd.GetState().IsOccupied(*it)) continue;
        HexEval energy = resist.Score(*it, color);
        if (energy == EVAL_INFINITY)
            energy = -1;
        cmd << " " << *it << " " 
            << std::fixed << std::setprecision(3) << energy;
    }
}

void BenzeneHtpEngine::CmdEvalResistDelta(HtpCommand& cmd)
{
    cmd.CheckNuArg(1);
    HexColor color = HtpUtil::ColorArg(cmd, 0);

    HexBoard& brd = m_pe.SyncBoard(m_game.Board());
    brd.ComputeAll(color);
    Resistance resist;
    resist.Evaluate(brd);
    HexEval base = resist.Score();

    cmd << " res " << std::fixed << std::setprecision(3) << base;
    for (BitsetIterator it(brd.GetState().GetEmpty()); it; ++it) 
    {
        brd.PlayMove(color, *it);
        resist.Evaluate(brd);
        HexEval cur = resist.Score();
        cmd << " " << *it << " " 
            << std::fixed << std::setprecision(3) << (cur - base);
        brd.UndoMove();
    }
}

void BenzeneHtpEngine::CmdEvalInfluence(HtpCommand& cmd)
{
    cmd.CheckNuArg(1);
    HexColor color = HtpUtil::ColorArg(cmd, 0);

    HexBoard& brd = m_pe.SyncBoard(m_game.Board());
    brd.ComputeAll(color);

    // Pre-compute edge adjacencies
    const Groups& groups = brd.GetGroups();
    bitset_t northNbs 
        = VCSetUtil::ConnectedTo(brd.Cons(BLACK), groups, NORTH, VC::FULL);
    bitset_t southNbs 
        = VCSetUtil::ConnectedTo(brd.Cons(BLACK), groups, SOUTH, VC::FULL);
    bitset_t eastNbs 
        = VCSetUtil::ConnectedTo(brd.Cons(WHITE), groups, EAST, VC::FULL);
    bitset_t westNbs 
        = VCSetUtil::ConnectedTo(brd.Cons(WHITE), groups, WEST, VC::FULL);

    for (BoardIterator it(brd.Const().Interior()); it; ++it) {
        if (brd.GetState().IsOccupied(*it)) continue;

	// Compute neighbours, giving over-estimation to edges
	bitset_t b1 = VCSetUtil::ConnectedTo(brd.Cons(BLACK), brd.GetGroups(),
                                             *it, VC::FULL);
	if (b1.test(NORTH)) b1 |= northNbs;
	if (b1.test(SOUTH)) b1 |= southNbs;
	b1 &= brd.GetState().GetEmpty();
	bitset_t b2 = VCSetUtil::ConnectedTo(brd.Cons(WHITE), brd.GetGroups(),
                                             *it, VC::FULL);
	if (b2.test(EAST)) b2 |= eastNbs;
	if (b2.test(WEST)) b2 |= westNbs;
	b2 &= brd.GetState().GetEmpty();

	// Compute ratio of VCs at this cell, and use as measure of influence
	double v1 = (double) b1.count();
	double v2 = (double) b2.count();
	HexAssert(v1+v2 >= 1.0);
	double influence;
	if (color == BLACK)
	    influence = v1 / (v1 + v2);
	else
	    influence = v2 / (v1 + v2);

        cmd << " " << *it << " "
	    << std::fixed << std::setprecision(2) << influence;
    }
}

//----------------------------------------------------------------------------

void BenzeneHtpEngine::CmdMiscDebug(HtpCommand& cmd)
{
//     cmd.CheckNuArg(1);
//     HexPoint point = HtpUtil::MoveArg(cmd, 0);
    cmd << *m_pe.brd << '\n';
}

//----------------------------------------------------------------------------
