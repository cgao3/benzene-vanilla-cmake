//----------------------------------------------------------------------------
/** @file CommonHtpEngine.cpp */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "SgGameReader.h"

#include "BoardUtil.hpp"
#include "BitsetIterator.hpp"
#include "Decompositions.hpp"
#include "CommonProgram.hpp"
#include "HexSgUtil.hpp"
#include "CommonHtpEngine.hpp"
#include "Resistance.hpp"
#include "DfsSolver.hpp"
#include "SwapCheck.hpp"
#include "TwoDistance.hpp"
#include "VCSet.hpp"
#include "VCUtil.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

CommonHtpEngine::CommonHtpEngine(int boardsize)
    : HexHtpEngine(boardsize),
      m_pe(m_board.Width(), m_board.Height()),
      m_se(m_board.Width(), m_board.Height()),
      m_dfsSolver(),
      m_dfpnSolver(),
      m_dfsHashTable(new DfsHashTable(1 << 20)), // TT with 2^20 entries
      m_dfpnHashTable(new DfpnHashTable(1 << 21)), // TT with 2^21 entries
      m_dfsDB(0),
      m_dfpnDB(0),
      m_dfsParam(),
      m_dfpnParam(),
      m_dfsPositions(m_dfsHashTable, m_dfsDB, m_dfsParam),
      m_dfpnPositions(m_dfpnHashTable, m_dfpnDB, m_dfpnParam),
      m_playerEnvCommands(m_pe),
      m_solverEnvCommands(m_se),
      m_vcCommands(m_game, m_pe),
      m_dfsSolverCommands(m_game, m_se, m_dfsSolver, m_dfsHashTable, m_dfsDB,
                          m_dfsPositions),
      m_dfpnSolverCommands(m_game, m_se, m_dfpnSolver, m_dfpnHashTable,
                           m_dfpnDB, m_dfpnPositions),
      m_useParallelSolver(false)
{
    RegisterCmd("benzene-license", &CommonHtpEngine::CmdLicense);
    RegisterCmd("group-get", &CommonHtpEngine::CmdGroupGet);
    RegisterCmd("handbook-add", &CommonHtpEngine::CmdHandbookAdd);
    RegisterCmd("compute-inferior", &CommonHtpEngine::CmdComputeInferior);
    RegisterCmd("compute-fillin", &CommonHtpEngine::CmdComputeFillin);
    RegisterCmd("compute-vulnerable", &CommonHtpEngine::CmdComputeVulnerable);
    RegisterCmd("compute-reversible", &CommonHtpEngine::CmdComputeReversible);
    RegisterCmd("compute-dominated", &CommonHtpEngine::CmdComputeDominated);
    RegisterCmd("compute-dominated-cell",
                &CommonHtpEngine::CmdComputeDominatedOnCell);
    RegisterCmd("find-comb-decomp", &CommonHtpEngine::CmdFindCombDecomp);
    RegisterCmd("find-split-decomp", &CommonHtpEngine::CmdFindSplitDecomp);
    RegisterCmd("encode-pattern", &CommonHtpEngine::CmdEncodePattern);

    m_playerEnvCommands.Register(*this, "player");
    m_solverEnvCommands.Register(*this, "solver");
    m_vcCommands.Register(*this);
    m_dfsSolverCommands.Register(*this);
    m_dfpnSolverCommands.Register(*this);

    RegisterCmd("eval-twod", &CommonHtpEngine::CmdEvalTwoDist);
    RegisterCmd("eval-resist", &CommonHtpEngine::CmdEvalResist);
    RegisterCmd("eval-resist-cells", &CommonHtpEngine::CmdEvalResistCells);
}

CommonHtpEngine::~CommonHtpEngine()
{
}

//----------------------------------------------------------------------------

void CommonHtpEngine::RegisterCmd(const std::string& name,
                               GtpCallback<CommonHtpEngine>::Method method)
{
    Register(name, new GtpCallback<CommonHtpEngine>(this, method));
}

void CommonHtpEngine::NewGame(int width, int height)
{
    HexHtpEngine::NewGame(width, height);
    m_pe.NewGame(width, height);
    m_se.NewGame(width, height);
}

//----------------------------------------------------------------------------

void CommonHtpEngine::CmdAnalyzeCommands(HtpCommand& cmd)
{
    HexHtpEngine::CmdAnalyzeCommands(cmd);
    cmd << 
        "string/Benzene License/benzene-license\n"
        "inferior/Compute Inferior/compute-inferior %m\n"
        "inferior/Compute Vulnerable/compute-vulnerable %m\n"
        "inferior/Compute Fillin/compute-fillin %m\n"
        "inferior/Compute Reversible/compute-reversible %m\n"
        "inferior/Compute Dominated/compute-dominated %m\n"
        "inferior/Compute Dominated Cell/compute-dominated-cell %m\n"
        "plist/Find Comb Decomp/find-comb-decomp %c\n"
        "plist/Find Split Decomp/find-split-decomp %c\n"
        "string/Encode Pattern/encode-pattern %P\n"
        "group/Show Group/group-get %p\n"
        "pspairs/Show TwoDistance/eval-twod %c\n"
        "string/Show Resist/eval-resist %c\n"
        "pspairs/Show Cell Energy/eval-resist-cells %c\n";
    m_playerEnvCommands.AddAnalyzeCommands(cmd, "player");
    m_solverEnvCommands.AddAnalyzeCommands(cmd, "solver");
    m_vcCommands.AddAnalyzeCommands(cmd);
    m_dfsSolverCommands.AddAnalyzeCommands(cmd);
    m_dfpnSolverCommands.AddAnalyzeCommands(cmd);
}

/** Displays usage license. */
void CommonHtpEngine::CmdLicense(HtpCommand& cmd)
{
    cmd << 
        BenzeneEnvironment::Get().GetProgram().GetName() << " " <<
        BenzeneEnvironment::Get().GetProgram().GetVersion() << " " <<
        BenzeneEnvironment::Get().GetProgram().GetDate() << "\n"
        "Copyright (C) 2007-2010 by the authors of the Benzene project.\n"
        "See http://benzene.sourceforge.net for information about benzene.\n"
        "Benzene comes with NO WARRANTY to the extent permitted by law.\n"
        "This program is free software; you can redistribute it and/or\n"
        "modify it under the terms of the GNU Lesser General Public License\n"
        "as published by the Free Software Foundation - version 3. For more\n"
"information about these matters, see the files COPYING and COPYING.LESSER.\n";
}

/** Returns the set of stones this stone is part of. */
void CommonHtpEngine::CmdGroupGet(HtpCommand& cmd)
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

/** Pulls moves out of the game for given color and appends them to
    the given handbook file. Skips the first move (ie, the move from
    the empty board). Performs no duplicate checking.
    Usage: 
      handbook-add [handbook.txt] [sgf file] [color] [max move #] 
*/
void CommonHtpEngine::CmdHandbookAdd(HtpCommand& cmd)
{
    cmd.CheckNuArg(4);
    std::string bookfilename = cmd.Arg(0);
    std::string sgffilename = cmd.Arg(1);
    HexColor colorToSave = HtpUtil::ColorArg(cmd, 2);
    int maxMove = cmd.ArgMin<int>(3, 0);
    
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
    std::vector<SgHashCode> hashes;
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
    BenzeneAssert(hashes.size() == responses.size());
 
    std::ofstream out(bookfilename.c_str(), std::ios_base::app);
    for (std::size_t i = 0 ; i < hashes.size(); ++i)
        out << hashes[i] << ' ' << responses[i] << '\n';
    out.close();
}

//----------------------------------------------------------------------

/** Outputs inferior cell info for current state. */
void CommonHtpEngine::CmdComputeInferior(HtpCommand& cmd)
{
    cmd.CheckNuArg(1);
    HexColor color = HtpUtil::ColorArg(cmd, 0);
    HexBoard& brd = m_pe.SyncBoard(m_game.Board());
    brd.GetPatternState().Update();
    GroupBuilder::Build(brd.GetPosition(), brd.GetGroups());
    InferiorCells inf;
    m_pe.ice.ComputeInferiorCells(color, brd.GetGroups(), 
                                  brd.GetPatternState(), inf);
    cmd << inf.GuiOutput();
    cmd << '\n';
}

/** Computes fillin for the given board. Color argument affects order
    for computing vulnerable/presimplicial pairs. */
void CommonHtpEngine::CmdComputeFillin(HtpCommand& cmd)
{
    cmd.CheckNuArg(1);
    HexColor color = HtpUtil::ColorArg(cmd, 0);
    HexBoard& brd = m_pe.SyncBoard(m_game.Board());
    brd.GetPatternState().Update();
    GroupBuilder::Build(brd.GetPosition(), brd.GetGroups());
    InferiorCells inf;
    m_pe.ice.ComputeFillin(color, brd.GetGroups(), brd.GetPatternState(), inf);
    inf.ClearVulnerable();
    cmd << inf.GuiOutput();
    cmd << '\n';
}

/** Computes vulnerable cells on the current board for the given color. */
void CommonHtpEngine::CmdComputeVulnerable(HtpCommand& cmd)
{
    cmd.CheckNuArg(1);
    HexColor col = HtpUtil::ColorArg(cmd, 0);
    HexBoard& brd = m_pe.SyncBoard(m_game.Board());
    brd.GetPatternState().Update();
    GroupBuilder::Build(brd.GetPosition(), brd.GetGroups());
    InferiorCells inf;
    m_pe.ice.FindVulnerable(brd.GetPatternState(), col, 
                            brd.GetPosition().GetEmpty(), inf);
    cmd << inf.GuiOutput();
    cmd << '\n';
}

/** Computes reversible cells on the current board for the given color. */
void CommonHtpEngine::CmdComputeReversible(HtpCommand& cmd)
{
    cmd.CheckNuArg(1);
    HexColor col = HtpUtil::ColorArg(cmd, 0);
    HexBoard& brd = m_pe.SyncBoard(m_game.Board());
    brd.GetPatternState().Update();
    GroupBuilder::Build(brd.GetPosition(), brd.GetGroups());
    InferiorCells inf;
    m_pe.ice.FindReversible(brd.GetPatternState(), col, 
                            brd.GetPosition().GetEmpty(), inf);
    cmd << inf.GuiOutput();
    cmd << '\n';
}

/** Computes dominated cells on the current board for the given color. */
void CommonHtpEngine::CmdComputeDominated(HtpCommand& cmd)
{
    cmd.CheckNuArg(1);
    HexColor col = HtpUtil::ColorArg(cmd, 0);
    HexBoard& brd = m_pe.SyncBoard(m_game.Board());
    brd.GetPatternState().Update();
    GroupBuilder::Build(brd.GetPosition(), brd.GetGroups());
    InferiorCells inf;
    m_pe.ice.FindDominated(brd.GetPatternState(), col, 
                           brd.GetPosition().GetEmpty(), inf);
    cmd << inf.GuiOutput();
    cmd << '\n';
}

/** Finds dominated patterns matching given cell. */
void CommonHtpEngine::CmdComputeDominatedOnCell(HtpCommand& cmd)
{
    cmd.CheckNuArg(2);
    HexColor col = HtpUtil::ColorArg(cmd, 0);
    HexPoint cell = HtpUtil::MoveArg(cmd, 1);
    HexBoard& brd = m_pe.SyncBoard(m_game.Board());
    if (m_game.Board().GetColor(cell) != EMPTY)
        return;
    brd.GetPatternState().Update();
    PatternHits hits;
    m_pe.ice.FindDominatedOnCell(brd.GetPatternState(), col, 
                                 cell, hits);
    for (std::size_t i = 0; i < hits.size(); ++i)
        cmd << " " << hits[i].GetPattern()->GetName();
    cmd << '\n';
}

/** Tries to find a combinatorial decomposition of the board state.
    Outputs cells in the vc if there is a decomposition. */
void CommonHtpEngine::CmdFindCombDecomp(HtpCommand& cmd)
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
    if (Decompositions::Find(brd, color, capturedVC)) 
        cmd << HexPointUtil::ToString(capturedVC);
}

/** Tries to find a group that crowds both opponent edges. Outputs
    group that crowds both edges if one exists.  

    @todo Dump inferior cell info as well? It's hard to see what's
    actually going on if it is not displayed.
*/
void CommonHtpEngine::CmdFindSplitDecomp(HtpCommand& cmd)
{
    cmd.CheckNuArg(1);
    HexColor color = HtpUtil::ColorArg(cmd, 0);
    HexBoard& brd = m_pe.SyncBoard(m_game.Board());
    brd.ComputeAll(BLACK);
    HexPoint group;
    if (Decompositions::FindSplitting(brd, color, group))
        cmd << group;
}

/** Outputs pattern in encoded form. 
    Takes a list of cells, the first cell being the center of the
    pattern (that is not actually in the pattern).
    @todo Clean this up!
*/
void CommonHtpEngine::CmdEncodePattern(HtpCommand& cmd)
{
    BenzeneAssert(cmd.NuArg() > 0);

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
        BenzeneAssert(j != 32);
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

/** Displays two-distance values for current state. */
void CommonHtpEngine::CmdEvalTwoDist(HtpCommand& cmd)
{
    cmd.CheckNuArg(1);
    HexColor color = HtpUtil::ColorArg(cmd, 0);
    HexBoard& brd = m_pe.SyncBoard(m_game.Board());
    brd.ComputeAll(color);
    TwoDistance twod(TwoDistance::ADJACENT);
    twod.Evaluate(brd);
    for (BoardIterator it(brd.Const().Interior()); it; ++it) 
    {
        if (brd.GetPosition().IsOccupied(*it)) continue;
        HexEval energy = twod.Score(*it, color);
        if (energy == EVAL_INFINITY)
            energy = -1;
        cmd << " " << *it << " " << energy;
    }
}

/** Displays resistance values for current state. */
void CommonHtpEngine::CmdEvalResist(HtpCommand& cmd)
{
    cmd.CheckNuArg(1);
    HexBoard& brd = *m_pe.brd;
    Resistance resist;
    resist.Evaluate(brd);
    cmd << " res " << std::fixed << std::setprecision(3) << resist.Score()
        << " rew " << std::fixed << std::setprecision(3) << resist.Resist(WHITE)
        << " reb " << std::fixed << std::setprecision(3) 
        << resist.Resist(BLACK);
}

/** Displays resistance values for current state. */
void CommonHtpEngine::CmdEvalResistCells(HtpCommand& cmd)
{
    cmd.CheckNuArg(1);
    HexColor color = HtpUtil::ColorArg(cmd, 0);
    HexBoard& brd = *m_pe.brd;
    Resistance resist;
    resist.Evaluate(brd);
    for (BoardIterator it(brd.Const().Interior()); it; ++it) 
    {
        if (brd.GetPosition().IsOccupied(*it)) 
            continue;
        HexEval energy = resist.Score(*it, color);
        if (energy == EVAL_INFINITY)
            energy = -1;
        cmd << " " << *it << " " 
            << std::fixed << std::setprecision(3) << energy;
    }
}

//----------------------------------------------------------------------------
