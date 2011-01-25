//----------------------------------------------------------------------------
/** @file VCCommands.cpp */
//----------------------------------------------------------------------------

#include "BoardUtil.hpp"
#include "EndgameUtils.hpp"
#include "VCCommands.hpp"
#include "VCUtils.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

VCCommands::VCCommands(Game& game, HexEnvironment& env)
    : m_game(game), 
      m_env(env)
{
}

VCCommands::~VCCommands()
{
}

void VCCommands::Register(GtpEngine& e)
{
    Register(e, "vc-between-cells", &VCCommands::CmdGetVCsBetween);
    Register(e, "vc-connected-to", &VCCommands::CmdGetCellsConnectedTo);
    Register(e, "vc-get-mustplay", &VCCommands::CmdGetMustPlay);
    Register(e, "vc-intersection", &VCCommands::CmdVCIntersection);
    Register(e, "vc-union", &VCCommands::CmdVCUnion);
    Register(e, "vc-build", &VCCommands::CmdBuildStatic);
    Register(e, "vc-build-incremental", &VCCommands::CmdBuildIncremental);
    Register(e, "vc-undo-incremental", &VCCommands::CmdUndoIncremental);
    Register(e, "vc-set-stats", &VCCommands::CmdSetInfo);
    Register(e, "vc-builder-stats", &VCCommands::CmdBuilderStats);
}

void VCCommands::Register(GtpEngine& engine, const std::string& command,
                          GtpCallback<VCCommands>::Method method)
{
    engine.Register(command, new GtpCallback<VCCommands>(this, method));
}

VC::Type VCCommands::VCTypeArg(const HtpCommand& cmd, std::size_t number) const
{
    return VCTypeUtil::FromString(cmd.ArgToLower(number));
}

//----------------------------------------------------------------------------

/** Builds VCs for both players.
    Displays ice info for the given color in the current
    board-state. */
void VCCommands::CmdBuildStatic(HtpCommand& cmd)
{
    cmd.CheckNuArg(1);
    HexColor color = HtpUtil::ColorArg(cmd, 0);
    HexBoard& brd = m_env.SyncBoard(m_game.Board());
    brd.ComputeAll(color);
    cmd << brd.GetInferiorCells().GuiOutput();
    if (!EndgameUtils::IsDeterminedState(brd, color))
    {
        bitset_t consider = EndgameUtils::MovesToConsider(brd, color);
        cmd << BoardUtil::GuiDumpOutsideConsiderSet(brd.GetPosition(), 
                                                     consider,
                                              brd.GetInferiorCells().All());
    }
    cmd << '\n';
}

/** Builds VCs incrementally.
    Must play move to the board first. Move that was played must be passed
    as an argument. */
void VCCommands::CmdBuildIncremental(HtpCommand& cmd)
{
    cmd.CheckNuArgLessEqual(2);
    HexColor color = HtpUtil::ColorArg(cmd, 0);
    HexPoint point = HtpUtil::MoveArg(cmd, 1);
    HexBoard& brd = *m_env.brd; // <-- NOTE: no call to SyncBoard()!
    brd.PlayMove(color, point);
    cmd << brd.GetInferiorCells().GuiOutput();
    if (!EndgameUtils::IsDeterminedState(brd, color))
    {
        bitset_t consider = EndgameUtils::MovesToConsider(brd, color);
        cmd << BoardUtil::GuiDumpOutsideConsiderSet(brd.GetPosition(), 
                                                     consider,
                                           brd.GetInferiorCells().All());
    }
    cmd << '\n';
}

/** Reverts VCs built incrementally. */
void VCCommands::CmdUndoIncremental(HtpCommand& cmd)
{
    UNUSED(cmd);
    m_env.brd->UndoMove();
}

/** Returns list of VCs between two cells.  
    Usage: "vc-between-cells x y c t", where x and y are the cells, c
    is the color of the player, and t is the type of connection
    (full,semi). */
void VCCommands::CmdGetVCsBetween(HtpCommand& cmd)
{
    cmd.CheckNuArg(4);
    HexPoint from = HtpUtil::MoveArg(cmd, 0);
    HexPoint to = HtpUtil::MoveArg(cmd, 1);
    HexColor color = HtpUtil::ColorArg(cmd, 2);
    VC::Type ctype = VCTypeArg(cmd, 3);
    HexBoard& brd = *m_env.brd;
    HexPoint fcaptain = brd.GetGroups().CaptainOf(from);
    HexPoint tcaptain = brd.GetGroups().CaptainOf(to);
    std::vector<VC> vc;
    brd.Cons(color).VCs(fcaptain, tcaptain, ctype, vc);
    const VCList& lst = brd.Cons(color).GetList(ctype, fcaptain, tcaptain);
    cmd << '\n';
    std::size_t i = 0;
    for (; i < lst.Softlimit() && i < vc.size(); ++i) 
        cmd << color << " " << vc.at(i) << '\n';
    if (i >= vc.size())
        return;
    cmd << color << " " << fcaptain << " " << tcaptain << " ";
    cmd << "softlimit ----------------------";
    cmd << '\n';
    for (; i < vc.size(); ++i)
        cmd << color << " " << vc.at(i) << '\n';
}

/** Returns list of cells given cell shares a vc with. */
void VCCommands::CmdGetCellsConnectedTo(HtpCommand& cmd)
{
    cmd.CheckNuArg(3);
    HexPoint from = HtpUtil::MoveArg(cmd, 0);
    HexColor color = HtpUtil::ColorArg(cmd, 1);
    VC::Type ctype = VCTypeArg(cmd, 2);
    bitset_t pt = VCSetUtil::ConnectedTo(m_env.brd->Cons(color), 
                                         m_env.brd->GetGroups(), from, ctype);
    cmd << HexPointUtil::ToString(pt);
}

/** Prints the cells in the current mustplay. */
void VCCommands::CmdGetMustPlay(HtpCommand& cmd)
{
    cmd.CheckNuArg(1);
    HexColor color = HtpUtil::ColorArg(cmd, 0);
    bitset_t mustplay = VCUtils::GetMustplay(*m_env.brd, color);
    InferiorCells inf(m_env.brd->GetInferiorCells());
    inf.ClearVulnerable();
    inf.ClearReversible();
    inf.ClearDominated();
    cmd << inf.GuiOutput();
    if (!EndgameUtils::IsDeterminedState(*m_env.brd, color))
    {
        bitset_t consider = EndgameUtils::MovesToConsider(*m_env.brd, color);
        cmd << BoardUtil::GuiDumpOutsideConsiderSet(m_env.brd->GetPosition(), 
                                                     consider,
                                                     inf.All());
    }
}

/** Prints cells in intersection of all connections between
    endpoints. */
void VCCommands::CmdVCIntersection(HtpCommand& cmd)
{
    cmd.CheckNuArg(4);
    HexPoint from = HtpUtil::MoveArg(cmd, 0);
    HexPoint to = HtpUtil::MoveArg(cmd, 1);
    HexColor color = HtpUtil::ColorArg(cmd, 2);
    VC::Type ctype = VCTypeArg(cmd, 3);
    HexBoard& brd = *m_env.brd;
    HexPoint fcaptain = brd.GetGroups().CaptainOf(from);
    HexPoint tcaptain = brd.GetGroups().CaptainOf(to);
    const VCList& lst = brd.Cons(color).GetList(ctype, fcaptain, tcaptain);
    bitset_t intersection = lst.HardIntersection();
    cmd << HexPointUtil::ToString(intersection);
}

/** Prints cells in the union of connections between endpoints. */
void VCCommands::CmdVCUnion(HtpCommand& cmd)
{
    cmd.CheckNuArg(4);
    HexPoint from = HtpUtil::MoveArg(cmd, 0);
    HexPoint to = HtpUtil::MoveArg(cmd, 1);
    HexColor color = HtpUtil::ColorArg(cmd, 2);
    VC::Type ctype = VCTypeArg(cmd, 3);
    HexBoard& brd = *m_env.brd;
    HexPoint fcaptain = brd.GetGroups().CaptainOf(from);
    HexPoint tcaptain = brd.GetGroups().CaptainOf(to);
    const VCList& lst = brd.Cons(color).GetList(ctype, fcaptain, tcaptain);
    bitset_t un = lst.GetGreedyUnion(); // FIXME: shouldn't be greedy!!
    cmd << HexPointUtil::ToString(un);
}

//----------------------------------------------------------------------------

/** Obtains statistics on connection set. */
void VCCommands::CmdSetInfo(HtpCommand& cmd)
{
    if (cmd.NuArg() == 0)
        throw HtpFailure("Need at least the color!");
    int maxConnections = 50;
    int numBins = 10;
    HexColor color = HtpUtil::ColorArg(cmd, 0);
    if (cmd.NuArg() == 3)
    {
        maxConnections = cmd.ArgMin<int>(1, 1);
        numBins = cmd.ArgMin<int>(2, 1);
    }
    HexBoard& brd = *m_env.brd;
    VCSetStatistics stats 
        = VCSetUtil::ComputeStatistics(brd.Cons(color), brd.GetGroups(),
                                       maxConnections, numBins);
    cmd << stats.Write();
}

/** Obtains statistics on connection set. */
void VCCommands::CmdBuilderStats(HtpCommand& cmd)
{
    cmd.CheckNuArg(1);
    HexColor color = HtpUtil::ColorArg(cmd, 0);
    HexBoard& brd = *m_env.brd;
    VCBuilderStatistics stats = brd.Builder().Statistics(color);
    cmd << stats.ToString();
}

//----------------------------------------------------------------------------


