//----------------------------------------------------------------------------
/** @file MoHexEngine.cpp
 */
//----------------------------------------------------------------------------

#include "SgSystem.h"

#include "BitsetIterator.hpp"
#include "MoHexEngine.hpp"
#include "MoHexPlayer.hpp"
#include "PlayAndSolve.hpp"
#include "SwapCheck.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

namespace {

std::string KnowledgeThresholdToString(const std::vector<std::size_t>& t)
{
    if (t.empty())
        return "0";
    std::ostringstream os;
    os << '\"';
    for (std::size_t i = 0; i < t.size(); ++i)
    {
        if (i > 0) 
            os << ' ';
        os << t[i];
    }
    os << '\"';
    return os.str();
}

std::vector<std::size_t> KnowledgeThresholdFromString(const std::string& val)
{
    std::vector<std::size_t> v;
    std::istringstream is(val);
    std::size_t t;
    while (is >> t)
        v.push_back(t);
    if (v.size() == 1 && v[0] == 0)
        v.clear();
    return v;
}

}

//----------------------------------------------------------------------------

MoHexEngine::MoHexEngine(GtpInputStream& in, GtpOutputStream& out, 
                         int boardsize, MoHexPlayer& player)
    : BenzeneHtpEngine(in, out, boardsize),
      m_player(player), 
      m_book(0),
      m_bookCheck(m_book),
      m_bookCommands(m_game, m_pe, m_book, m_bookCheck, m_player)
{
    m_bookCommands.Register(*this);
    RegisterCmd("param_mohex", &MoHexEngine::MoHexParam);
    RegisterCmd("param_mohex_policy", &MoHexEngine::MoHexPolicyParam);
    RegisterCmd("mohex-save-tree", &MoHexEngine::SaveTree);
    RegisterCmd("mohex-save-games", &MoHexEngine::SaveGames);
    RegisterCmd("mohex-values", &MoHexEngine::Values);
    RegisterCmd("mohex-rave-values", &MoHexEngine::RaveValues);
    RegisterCmd("mohex-bounds", &MoHexEngine::Bounds);
}

MoHexEngine::~MoHexEngine()
{
}

//----------------------------------------------------------------------------

void MoHexEngine::RegisterCmd(const std::string& name,
                              GtpCallback<MoHexEngine>::Method method)
{
    Register(name, new GtpCallback<MoHexEngine>(this, method));
}

double MoHexEngine::TimeForMove(HexColor color)
{
    if (m_player.UseTimeManagement())
        return m_game.TimeRemaining(color) * 0.08;
    return m_player.MaxTime();
}

HexPoint MoHexEngine::GenMove(HexColor color, bool useGameClock)
{
    SG_UNUSED(useGameClock);
    if (SwapCheck::PlaySwap(m_game, color))
        return SWAP_PIECES;
    HexPoint bookMove = m_bookCheck.BestMove(HexState(m_game.Board(), color));
    if (bookMove != INVALID_POINT)
        return bookMove;
    double maxTime = TimeForMove(color);
    return DoSearch(color, maxTime);
}

HexPoint MoHexEngine::DoSearch(HexColor color, double maxTime)
{
    HexState state(m_game.Board(), color);
    if (m_useParallelSolver)
    {
        PlayAndSolve ps(*m_pe.brd, *m_se.brd, m_player, m_dfpnSolver, 
                        m_dfpnPositions, m_game);
        return ps.GenMove(state, maxTime);
    }
    else
    {
        double score;
        return m_player.GenMove(state, m_game, m_pe.SyncBoard(m_game.Board()),
                                maxTime, score);
    }
}

//----------------------------------------------------------------------------

void MoHexEngine::MoHexPolicyParam(HtpCommand& cmd)
{
    HexUctPolicyConfig& config = m_player.SharedPolicy().Config();
    if (cmd.NuArg() == 0)
    {
        cmd << '\n'
            << "pattern_check_percent "
            << config.pattern_check_percent << '\n'
            << "pattern_heuristic "
            << config.patternHeuristic << '\n'
            << "response_heuristic "
            << config.responseHeuristic << '\n'
            << "response_threshold "
            << config.response_threshold << '\n';
    }
    else if (cmd.NuArg() == 2)
    {
        std::string name = cmd.Arg(0);
        if (name == "pattern_check_percent")
            config.pattern_check_percent = cmd.IntArg(1, 0, 100);
        else if (name == "pattern_heuristic")
            config.patternHeuristic = cmd.BoolArg(1);
        else if (name == "response_heuristic")
            config.responseHeuristic = cmd.BoolArg(1);
        else if (name == "response_threshold")
            config.response_threshold = cmd.SizeTypeArg(1, 0);
        else
            throw HtpFailure("Unknown option!");
    }
    else
        throw HtpFailure("Expected 0 or 2 arguments!");
}

void MoHexEngine::MoHexParam(HtpCommand& cmd)
{
    HexUctSearch& search = m_player.Search();

    if (cmd.NuArg() == 0) 
    {
        cmd << '\n'
            << "[bool] backup_ice_info "
            << m_player.BackupIceInfo() << '\n'
            << "[bool] lock_free " 
            << search.LockFree() << '\n'
            << "[bool] keep_games "
            << search.KeepGames() << '\n'
            << "[bool] perform_pre_search " 
            << m_player.PerformPreSearch() << '\n'
            << "[bool] ponder "
            << m_player.Ponder() << '\n'
            << "[bool] reuse_subtree " 
            << m_player.ReuseSubtree() << '\n'
            << "[bool] search_singleton "
            << m_player.SearchSingleton() << '\n'
            << "[bool] use_livegfx "
            << search.LiveGfx() << '\n'
            << "[bool] use_parallel_solver "
            << m_useParallelSolver << '\n'
            << "[bool] use_rave "
            << search.Rave() << '\n'
            << "[bool] use_time_management "
            << m_player.UseTimeManagement() << '\n'
            << "[bool] weight_rave_updates "
            << search.WeightRaveUpdates() << '\n'
            << "[string] bias_term "
            << search.BiasTermConstant() << '\n'
            << "[string] expand_threshold "
            << search.ExpandThreshold() << '\n'
            << "[string] knowledge_threshold "
            << KnowledgeThresholdToString(search.KnowledgeThreshold()) << '\n'
            << "[string] livegfx_interval "
            << search.LiveGfxInterval() << '\n'
            << "[string] max_games "
            << m_player.MaxGames() << '\n'
            << "[string] max_memory "
            << search.MaxNodes() * 2 * sizeof(SgUctNode) << '\n'
            << "[string] max_nodes "
            << search.MaxNodes() << '\n'
            << "[string] max_time "
            << m_player.MaxTime() << '\n'
            << "[string] num_threads "
            << search.NumberThreads() << '\n'
            << "[string] playout_update_radius "
            << search.PlayoutUpdateRadius() << '\n'
            << "[string] randomize_rave_frequency "
            << search.RandomizeRaveFrequency() << '\n'
            << "[string] rave_weight_final "
            << search.RaveWeightFinal() << '\n'
            << "[string] rave_weight_initial "
            << search.RaveWeightInitial() << '\n'
            << "[string] tree_update_radius " 
            << search.TreeUpdateRadius() << '\n';
    }
    else if (cmd.NuArg() == 2)
    {
        std::string name = cmd.Arg(0);
        if (name == "backup_ice_info")
            m_player.SetBackupIceInfo(cmd.BoolArg(1));
        else if (name == "lock_free")
            search.SetLockFree(cmd.BoolArg(1));
        else if (name == "keep_games")
            search.SetKeepGames(cmd.BoolArg(1));
        else if (name == "perform_pre_search")
            m_player.SetPerformPreSearch(cmd.BoolArg(1));
        else if (name == "ponder")
            m_player.SetPonder(cmd.BoolArg(1));
        else if (name == "use_livegfx")
            search.SetLiveGfx(cmd.BoolArg(1));
        else if (name == "use_rave")
            search.SetRave(cmd.BoolArg(1));
        else if (name == "randomize_rave_frequency")
            search.SetRandomizeRaveFrequency(cmd.IntArg(1, 0));
        else if (name == "reuse_subtree")
           m_player.SetReuseSubtree(cmd.BoolArg(1));
        else if (name == "bias_term")
            search.SetBiasTermConstant(cmd.FloatArg(1));
        else if (name == "expand_threshold")
            search.SetExpandThreshold(cmd.IntArg(1, 0));
        else if (name == "knowledge_threshold")
            search.SetKnowledgeThreshold
                (KnowledgeThresholdFromString(cmd.Arg(1)));
        else if (name == "livegfx_interval")
            search.SetLiveGfxInterval(cmd.IntArg(1, 0));
        else if (name == "max_games")
            m_player.SetMaxGames(cmd.IntArg(1, 0));
        else if (name == "max_memory")
            search.SetMaxNodes(cmd.SizeTypeArg(1, 1) / sizeof(SgUctNode) / 2);
        else if (name == "max_time")
            m_player.SetMaxTime(cmd.FloatArg(1));
        else if (name == "max_nodes")
            search.SetMaxNodes(cmd.SizeTypeArg(1, 1));
        else if (name == "num_threads")
            search.SetNumberThreads(cmd.IntArg(1, 0));
        else if (name == "playout_update_radius")
            search.SetPlayoutUpdateRadius(cmd.IntArg(1, 0));
        else if (name == "rave_weight_final")
            search.SetRaveWeightFinal(cmd.IntArg(1, 0));
        else if (name == "rave_weight_initial")
            search.SetRaveWeightInitial(cmd.IntArg(1, 0));
        else if (name == "weight_rave_updates")
            search.SetWeightRaveUpdates(cmd.BoolArg(1));
        else if (name == "tree_update_radius")
            search.SetTreeUpdateRadius(cmd.IntArg(1, 0));
        else if (name == "search_singleton")
            m_player.SetSearchSingleton(cmd.BoolArg(1));
        else if (name == "use_parallel_solver")
            m_useParallelSolver = cmd.BoolArg(1);
        else if (name == "use_time_management")
            m_player.SetUseTimeManagement(cmd.BoolArg(1));
        else
            throw HtpFailure() << "Unknown parameter: " << name;
    }
    else 
        throw HtpFailure("Expected 0 or 2 arguments");
}

/** Saves the search tree from the previous search to the specified
    file.  The optional second parameter sets the max depth to
    output. If not given, entire tree is saved.    
*/
void MoHexEngine::SaveTree(HtpCommand& cmd)
{
    HexUctSearch& search = m_player.Search();

    cmd.CheckNuArg(1);
    std::string filename = cmd.Arg(0);
    int maxDepth = -1;
    std::ofstream file(filename.c_str());
    if (!file)
        throw HtpFailure() << "Could not open '" << filename << "'";
    if (cmd.NuArg() == 2)
        maxDepth = cmd.IntArg(1, 0);
    search.SaveTree(file, maxDepth);
}

/** Saves games from last search to a SGF. */
void MoHexEngine::SaveGames(HtpCommand& cmd)
{
    HexUctSearch& search = m_player.Search();
    cmd.CheckNuArg(1);
    std::string filename = cmd.Arg(0);
    search.SaveGames(filename);
}

void MoHexEngine::Values(HtpCommand& cmd)
{
    HexUctSearch& search = m_player.Search();
    const SgUctTree& tree = search.Tree();
    for (SgUctChildIterator it(tree, tree.Root()); it; ++it)
    {
        const SgUctNode& child = *it;
        SgPoint p = child.Move();
        std::size_t count = child.MoveCount();
        float mean = 0.0;
        if (count > 0)
            mean = child.Mean();
        cmd << ' ' << static_cast<HexPoint>(p)
            << ' ' << std::fixed << std::setprecision(3) << mean
            << '@' << count;
    }
}

void MoHexEngine::RaveValues(HtpCommand& cmd)
{
    HexUctSearch& search = m_player.Search();
    const SgUctTree& tree = search.Tree();
    for (SgUctChildIterator it(tree, tree.Root()); it; ++it)
    {
        const SgUctNode& child = *it;
        SgPoint p = child.Move();
        if (p == SG_PASS || ! child.HasRaveValue())
            continue;
        cmd << ' ' << static_cast<HexPoint>(p)
            << ' ' << std::fixed << std::setprecision(3) << child.RaveValue();
    }
}

void MoHexEngine::Bounds(HtpCommand& cmd)
{
    HexUctSearch& search = m_player.Search();
    const SgUctTree& tree = search.Tree();
    for (SgUctChildIterator it(tree, tree.Root()); it; ++it)
    {
        const SgUctNode& child = *it;
        SgPoint p = child.Move();
        if (p == SG_PASS || ! child.HasRaveValue())
            continue;
        float bound = search.GetBound(search.Rave(), tree.Root(), child);
        cmd << ' ' << static_cast<HexPoint>(p) 
            << ' ' << std::fixed << std::setprecision(3) << bound;
    }
}

//----------------------------------------------------------------------------
// Pondering

#if GTPENGINE_PONDER

void MoHexEngine::InitPonder()
{
    SgSetUserAbort(false);
}

void MoHexEngine::Ponder()
{
    if (!m_player.Ponder())
        return;
    if (!m_player.ReuseSubtree())
    {
        LogWarning() << "Pondering requires reuse_subtree.\n";
        return;
    }
    // Call genmove() after 0.2 seconds delay to avoid calls 
    // in very short intervals between received commands
    boost::xtime time;
    boost::xtime_get(&time, boost::TIME_UTC);
    for (int i = 0; i < 200; ++i)
    {
        if (SgUserAbort())
            return;
        time.nsec += 1000000; // 1 msec
        boost::thread::sleep(time);
    }
    LogInfo() << "MoHexEngine::Ponder: start\n";
    // Search for at most 10 minutes
    DoSearch(m_game.Board().WhoseTurn(), 600);
}

void MoHexEngine::StopPonder()
{
    SgSetUserAbort(true);
}

#endif // GTPENGINE_PONDER

//----------------------------------------------------------------------------

