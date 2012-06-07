//----------------------------------------------------------------------------
/** @file MoHexEngine.cpp */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "SgUctTreeUtil.h"

#include "BitsetIterator.hpp"
#include "BoardUtil.hpp"
#include "MoHexEngine.hpp"
#include "MoHexPlayer.hpp"
#include "MoHexUtil.hpp"
#include "PlayAndSolve.hpp"
#include "SwapCheck.hpp"
#include "NeighborTracker.hpp"
#include "Misc.hpp"
#include "Move.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

namespace {

//----------------------------------------------------------------------------

std::string KnowledgeThresholdToString(const std::vector<SgUctValue>& t)
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

std::vector<SgUctValue> KnowledgeThresholdFromString(const std::string& val)
{
    std::vector<SgUctValue> v;
    std::istringstream is(val);
    SgUctValue t;
    while (is >> t)
        v.push_back(t);
    if (v.size() == 1 && v[0] == 0)
        v.clear();
    return v;
}

SgUctMoveSelect MoveSelectArg(const HtpCommand& cmd, size_t number)
{
    std::string arg = cmd.ArgToLower(number);
    if (arg == "value")
        return SG_UCTMOVESELECT_VALUE;
    if (arg == "count")
        return SG_UCTMOVESELECT_COUNT;
    if (arg == "bound")
        return SG_UCTMOVESELECT_BOUND;
    if (arg == "estimate")
        return SG_UCTMOVESELECT_ESTIMATE;
    throw HtpFailure() << "unknown move select argument \"" << arg << '"';
}

std::string MoveSelectToString(SgUctMoveSelect moveSelect)
{
    switch (moveSelect)
    {
    case SG_UCTMOVESELECT_VALUE:
        return "value";
    case SG_UCTMOVESELECT_COUNT:
        return "count";
    case SG_UCTMOVESELECT_BOUND:
        return "bound";
    case SG_UCTMOVESELECT_ESTIMATE:
        return "estimate";
    default:
        return "?";
    }
}

//----------------------------------------------------------------------------

} // namespace

//----------------------------------------------------------------------------

MoHexEngine::MoHexEngine(int boardsize, MoHexPlayer& player)
    : CommonHtpEngine(boardsize),
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
    RegisterCmd("mohex-get-pv", &MoHexEngine::GetPV);
    RegisterCmd("mohex-values", &MoHexEngine::Values);
    RegisterCmd("mohex-rave-values", &MoHexEngine::RaveValues);
    RegisterCmd("mohex-prior-values", &MoHexEngine::PriorValues);
    RegisterCmd("mohex-bounds", &MoHexEngine::Bounds);
    RegisterCmd("mohex-cell-stats", &MoHexEngine::CellStats);
    RegisterCmd("mohex-do-playouts", &MoHexEngine::DoPlayouts);
    RegisterCmd("mohex-playout-move", &MoHexEngine::PlayoutMove);
    RegisterCmd("mohex-playout-weights", &MoHexEngine::PlayoutWeights);
    RegisterCmd("mohex-playout-global-weights", 
                &MoHexEngine::PlayoutGlobalWeights);
    RegisterCmd("mohex-playout-local-weights", 
                &MoHexEngine::PlayoutLocalWeights);
    RegisterCmd("mohex-find-top-moves", &MoHexEngine::FindTopMoves);
    RegisterCmd("mohex-self-play", &MoHexEngine::SelfPlay);
    RegisterCmd("mohex-mark-prunable", &MoHexEngine::MarkPrunablePatterns);
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
        return m_game.TimeRemaining(color) * 0.12;
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

const SgUctNode* MoHexEngine::FindState(const Game& game) const
{
    MoHexSearch& search = m_player.Search();
    // Board size must be the same. This also catches the case where
    // no previous search has been performed.
    const StoneBoard& oldPosition = search.SharedData().rootState.Position();
    const StoneBoard& newPosition = game.Board();
    if (&oldPosition.Const() == 0)
        throw HtpFailure() << "No previous search";
    if (oldPosition.Width() != newPosition.Width() ||
        oldPosition.Height() != newPosition.Height())
        throw HtpFailure() << "Board size differs from last search";
    const MoveSequence& oldSequence = search.SharedData().gameSequence;
    const MoveSequence& newSequence = game.History();
    if (oldSequence.size() > newSequence.size())
        throw HtpFailure() << "Backtracked to earlier position"; 
    if (!MoveSequenceUtil::IsPrefixOf(oldSequence, newSequence))
        throw HtpFailure() << "Not a continuation";
    const SgUctTree& tree = search.Tree();
    const SgUctNode* node = &tree.Root();
    for (std::size_t i = oldSequence.size(); i < newSequence.size(); ++i)
    {
        if (i && newSequence[i-1].Color() == newSequence[i].Color())
            throw HtpFailure() << "Colors do not alternate in continuation";
        HexPoint move = newSequence[i].Point();
        node = SgUctTreeUtil::FindChildWithMove(tree, *node, move);
        if (node == 0)
            throw HtpFailure() << "State not in previous search";
    }
    return node;
}

//----------------------------------------------------------------------------

void MoHexEngine::CmdAnalyzeCommands(HtpCommand& cmd)
{
    CommonHtpEngine::CmdAnalyzeCommands(cmd);
    m_bookCommands.AddAnalyzeCommands(cmd);
    cmd << 
        "param/MoHex Param/param_mohex\n"
        "param/MoHex Policy Param/param_mohex_policy\n"
        "none/MoHex Save Tree/mohex-save-tree %w\n"
        "none/MoHex Save Games/mohex-save-games %w\n"
        "var/MoHex PV/mohex-get-pv %m\n"
        "pspairs/MoHex Values/mohex-values\n"
        "pspairs/MoHex Rave Values/mohex-rave-values\n"
        "pspairs/MoHex Prior Values/mohex-prior-values\n"
        "pspairs/MoHex Bounds/mohex-bounds\n"
        "gfx/MoHex Cell Stats/mohex-cell-stats %P\n"
        "string/MoHex Do Playouts/mohex-do-playouts\n"
        "move/MoHex Playout Move/mohex-playout-move\n"
        "pspairs/MoHex Playout Weights/mohex-playout-weights\n"
        "pspairs/MoHex Playout Global Weights/mohex-playout-global-weights\n"  
        "pspairs/MoHex Playout Local Weights/mohex-playout-local-weights\n"
        "none/MoHex Self Play/mohex-self-play\n"
        "pspairs/MoHex Top Moves/mohex-find-top-moves %c\n";
}

void MoHexEngine::MoHexPolicyParam(HtpCommand& cmd)
{
    MoHexPlayoutPolicyConfig& config = m_player.SharedPolicy().Config();
    if (cmd.NuArg() == 0)
    {
        cmd << '\n'
            << "pattern_heuristic "
            << config.patternHeuristic << '\n';
    }
    else if (cmd.NuArg() == 2)
    {
        std::string name = cmd.Arg(0);
        if (name == "pattern_heuristic")
            config.patternHeuristic = cmd.Arg<bool>(1);
        else
            throw HtpFailure("Unknown option!");
    }
    else
        throw HtpFailure("Expected 0 or 2 arguments!");
}

void MoHexEngine::MoHexParam(HtpCommand& cmd)
{
    MoHexSearch& search = m_player.Search();
    if (cmd.NuArg() == 0) 
    {
        cmd << '\n'
            << "[bool] backup_ice_info "
            << m_player.BackupIceInfo() << '\n'
            << "[bool] extend_unstable_search "
            << search.ExtendUnstableSearch() << '\n'
#if HAVE_GCC_ATOMIC_BUILTINS
            << "[bool] lock_free " 
            << search.LockFree() << '\n'
#endif
            << "[bool] keep_games "
            << search.KeepGames() << '\n'
            << "[bool] lazy_delete "
            << search.LazyDelete() << '\n'
            << "[bool] perform_pre_search " 
            << m_player.PerformPreSearch() << '\n'
            << "[bool] prior_pruning "
            << search.PriorPruning() << '\n'
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
            << "[bool] use_root_data "
            << m_player.UseRootData() << '\n'
            << "[bool] use_time_management "
            << m_player.UseTimeManagement() << '\n'
            << "[bool] weight_rave_updates "
            << search.WeightRaveUpdates() << '\n'
            << "[bool] virtual_loss "
            << search.VirtualLoss() << '\n'
            << "[string] bias_term "
            << search.BiasTermConstant() << '\n'
            << "[string] expand_threshold "
            << search.ExpandThreshold() << '\n'
            << "[string] fillin_map_bits "
            << search.FillinMapBits() << '\n'
            << "[string] knowledge_threshold "
            << KnowledgeThresholdToString(search.KnowledgeThreshold()) << '\n'
            << "[string] number_playouts_per_visit "
            << search.NumberPlayouts() << '\n'
            << "[string] max_games "
            << m_player.MaxGames() << '\n'
            << "[string] max_memory "
            << search.MaxNodes() * 2 * sizeof(SgUctNode) << '\n'
            << "[string] max_nodes "
            << search.MaxNodes() << '\n'
            << "[string] max_time "
            << m_player.MaxTime() << '\n'
            << "[string] move_select "
            << MoveSelectToString(search.MoveSelect()) << '\n'
#if HAVE_GCC_ATOMIC_BUILTINS
            << "[string] num_threads "
            << search.NumberThreads() << '\n'
#endif
            << "[string] progressive_bias "
            << search.ProgressiveBiasConstant() << '\n'
            << "[string] vc_progressive_bias "
            << search.VCProgressiveBiasConstant() << '\n'
            << "[string] vcm_gamma "
            << search.VCMGamma() << '\n'
            << "[string] randomize_rave_frequency "
            << search.RandomizeRaveFrequency() << '\n'
            << "[string] rave_weight_final "
            << search.RaveWeightFinal() << '\n'
            << "[string] rave_weight_initial "
            << search.RaveWeightInitial() << '\n';
    }
    else if (cmd.NuArg() == 2)
    {
        std::string name = cmd.Arg(0);
        if (name == "backup_ice_info")
            m_player.SetBackupIceInfo(cmd.Arg<bool>(1));
        else if (name == "extend_unstable_search")
            search.SetExtendUnstableSearch(cmd.Arg<bool>(1));
        else if (name == "lazy_delete")
            search.SetLazyDelete(cmd.Arg<bool>(1));
#if HAVE_GCC_ATOMIC_BUILTINS
        else if (name == "lock_free")
            search.SetLockFree(cmd.Arg<bool>(1));
#endif
        else if (name == "keep_games")
            search.SetKeepGames(cmd.Arg<bool>(1));
        else if (name == "perform_pre_search")
            m_player.SetPerformPreSearch(cmd.Arg<bool>(1));
        else if (name == "prior_pruning")
            search.SetPriorPruning(cmd.Arg<bool>(1));
        else if (name == "ponder")
            m_player.SetPonder(cmd.Arg<bool>(1));
        else if (name == "use_livegfx")
            search.SetLiveGfx(cmd.Arg<bool>(1));
        else if (name == "use_rave")
            search.SetRave(cmd.Arg<bool>(1));
        else if (name == "use_root_data")
            m_player.SetUseRootData(cmd.Arg<bool>(1));
        else if (name == "randomize_rave_frequency")
            search.SetRandomizeRaveFrequency(cmd.ArgMin<int>(1, 0));
        else if (name == "reuse_subtree")
           m_player.SetReuseSubtree(cmd.Arg<bool>(1));
        else if (name == "bias_term")
            search.SetBiasTermConstant(cmd.Arg<float>(1));
        else if (name == "expand_threshold")
            search.SetExpandThreshold(cmd.ArgMin<int>(1, 0));
        else if (name == "knowledge_threshold")
            search.SetKnowledgeThreshold
                (KnowledgeThresholdFromString(cmd.Arg(1)));
        else if (name == "fillin_map_bits")
            search.SetFillinMapBits(cmd.ArgMin<int>(1, 1));
        else if (name == "max_games")
            m_player.SetMaxGames(cmd.ArgMin<int>(1, 1));
        else if (name == "max_memory")
            search.SetMaxNodes(cmd.ArgMin<std::size_t>(1, 1) 
                               / sizeof(SgUctNode) / 2);
        else if (name == "max_time")
            m_player.SetMaxTime(cmd.Arg<float>(1));
        else if (name == "max_nodes")
            search.SetMaxNodes(cmd.ArgMin<std::size_t>(1, 1));
        else if (name == "move_select")
            search.SetMoveSelect(MoveSelectArg(cmd, 1));
#if HAVE_GCC_ATOMIC_BUILTINS
        else if (name == "num_threads")
            search.SetNumberThreads(cmd.ArgMin<int>(1, 1));
#endif
        else if (name == "number_playouts_per_visit")
            search.SetNumberPlayouts(cmd.ArgMin<int>(1, 1));
        else if (name == "progressive_bias")
            search.SetProgressiveBiasConstant(cmd.ArgMin<float>(1, 0));
        else if (name == "vc_progressive_bias")
            search.SetVCProgressiveBiasConstant(cmd.ArgMin<float>(1, 0));
        else if (name == "vcm_gamma")
            search.SetVCMGamma(cmd.ArgMin<float>(1, 0));
        else if (name == "rave_weight_final")
            search.SetRaveWeightFinal(cmd.ArgMin<float>(1, 0));
        else if (name == "rave_weight_initial")
            search.SetRaveWeightInitial(cmd.ArgMin<float>(1, 0));
        else if (name == "weight_rave_updates")
            search.SetWeightRaveUpdates(cmd.Arg<bool>(1));
        else if (name == "search_singleton")
            m_player.SetSearchSingleton(cmd.Arg<bool>(1));
        else if (name == "use_parallel_solver")
            m_useParallelSolver = cmd.Arg<bool>(1);
        else if (name == "use_time_management")
            m_player.SetUseTimeManagement(cmd.Arg<bool>(1));
        else if (name == "virtual_loss")
            search.SetVirtualLoss(cmd.Arg<bool>(1));
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
    MoHexSearch& search = m_player.Search();

    cmd.CheckNuArg(1);
    std::string filename = cmd.Arg(0);
    int maxDepth = -1;
    std::ofstream file(filename.c_str());
    if (!file)
        throw HtpFailure() << "Could not open '" << filename << "'";
    if (cmd.NuArg() == 2)
        maxDepth = cmd.ArgMin<int>(1, 0);
    search.SaveTree(file, maxDepth);
}

/** Saves games from last search to a SGF. */
void MoHexEngine::SaveGames(HtpCommand& cmd)
{
    MoHexSearch& search = m_player.Search();
    cmd.CheckNuArg(1);
    std::string filename = cmd.Arg(0);
    search.SaveGames(filename);
}

void MoHexEngine::Values(HtpCommand& cmd)
{
    MoHexSearch& search = m_player.Search();
    const SgUctTree& tree = search.Tree();
    const SgUctNode* node = FindState(m_game);
    for (SgUctChildIterator it(tree, *node); it; ++it)
    {
        const SgUctNode& child = *it;
        HexPoint p = static_cast<HexPoint>(child.Move());
        std::size_t count = (size_t)child.MoveCount();
        cmd << ' ' << p << ' ';
        if (child.IsProvenLoss())
            cmd << "W@" << count;
        else if (child.IsProvenWin())
            cmd << "L@" << count;
        else if (count == 0)
            cmd << "0";
        else 
        {
            SgUctValue mean = search.InverseEval(child.Mean());
            cmd << '.' << MoHexUtil::FixedValue(mean, 3)
                << '@' << MoHexUtil::CleanCount(count);
        }
    }
}

void MoHexEngine::RaveValues(HtpCommand& cmd)
{
    MoHexSearch& search = m_player.Search();
    const SgUctTree& tree = search.Tree();
    const SgUctNode* node = FindState(m_game);
    for (SgUctChildIterator it(tree, *node); it; ++it)
    {
        const SgUctNode& child = *it;
        if (! child.HasRaveValue())
            continue;
        cmd << ' ' << static_cast<HexPoint>(child.Move()) << ' '
            << '.' << MoHexUtil::FixedValue(child.RaveValue(), 3)
            << '@' << MoHexUtil::CleanCount((size_t)child.RaveCount());
    }
}

void MoHexEngine::Bounds(HtpCommand& cmd)
{
    MoHexSearch& search = m_player.Search();
    const SgUctTree& tree = search.Tree();
    const SgUctNode* node = FindState(m_game);
    for (SgUctChildIterator it(tree, *node); it; ++it)
    {
        const SgUctNode& child = *it;
        std::size_t count = (size_t)child.MoveCount();
        cmd << ' ' << static_cast<HexPoint>(child.Move()) << ' ';
        if (child.IsProvenLoss())
            cmd << "W@" << count;
        else if (child.IsProvenWin())
            cmd << "L@" << count;
        else
        {
            // show bound even if count is 0
            // FIXME: FixedValue() expects [0,1]... which the bound can exceed
            SgUctValue bound = search.GetBound(search.Rave(), *node, child);
            cmd << '.' << MoHexUtil::FixedValue(bound, 3)
                << '@' << MoHexUtil::CleanCount(count);
        }
    }
}

void MoHexEngine::PriorValues(HtpCommand& cmd)
{
    MoHexSearch& search = m_player.Search();
    const SgUctTree& tree = search.Tree();
    const SgUctNode* node = FindState(m_game);
    for (SgUctChildIterator it(tree, *node); it; ++it)
    {
        const SgUctNode& child = *it;
        cmd << ' ' << static_cast<HexPoint>(child.Move())
            << ' ' << std::fixed << std::setprecision(3) << child.Prior();
    }
}

void MoHexEngine::GetPV(HtpCommand& cmd)
{
    MoHexSearch& search = m_player.Search();
    const SgUctNode* node = FindState(m_game);
    const SgUctNode* current = node;
    while (true)
    {
        current = search.FindBestChild(*current, search.MoveSelect());
        if (current == 0)
            break;
        cmd << ' ' << static_cast<HexPoint>(current->Move());
        if (! current->HasChildren())
            break;
    }
}

void MoHexEngine::PerformPlayout(MoHexThreadState* thread,
                                 const HexState& state,
                                 const HexPoint lastMovePlayed)
{
    thread->StartPlayout(state, lastMovePlayed);
    const MoHexBoard& mobrd = thread->GetMoHexBoard();
    bool skipRaveUpdate;
    const ConstBoard& cbrd = mobrd.Const();
    while (mobrd.NumMoves() < cbrd.Width() * cbrd.Height())
    //while (!mobrd.GameOver())
    {
        SgMove move = thread->GeneratePlayoutMove(skipRaveUpdate);
        if (move == SG_NULLMOVE)
            break;
        thread->ExecutePlayout(move);
    }
}

void MoHexEngine::DoPlayouts(HtpCommand& cmd)
{
    cmd.CheckNuArgLessEqual(1);
    size_t numPlayouts = 1000;
    if (cmd.NuArg() == 1)
        numPlayouts = cmd.ArgMin<size_t>(0, 1);
    MoHexSearch& search = m_player.Search();
    if (!search.ThreadsCreated())
        search.CreateThreads();
    MoHexThreadState* thread 
        = dynamic_cast<MoHexThreadState*>(&search.ThreadState(0));
    if (!thread)
        throw HtpFailure() << "Thread not a MoHexThreadState!";
    const MoHexBoard& mobrd = thread->GetMoHexBoard();
    HexState state(m_game.Board(), m_game.Board().WhoseTurn());
    HexPoint lastMovePlayed
        = MoveSequenceUtil::LastMoveFromHistory(m_game.History());
    size_t wins = 0;
    for (size_t i = 0; i < numPlayouts; ++i)
    {
        PerformPlayout(thread, state, lastMovePlayed);
        if (mobrd.GetWinner() == state.ToPlay())
            wins++;
    }
    cmd << "wins=" << wins << " total=" << numPlayouts
        << " score=" << (double)wins * 100.0 / (double)numPlayouts;
}

void MoHexEngine::CellStats(HtpCommand& cmd)
{
    HexPoint from = HtpUtil::MoveArg(cmd, 0);
    HexPoint to = HtpUtil::MoveArg(cmd, 1);
    MoHexSearch& search = m_player.Search();
    if (!search.ThreadsCreated())
        search.CreateThreads();
    MoHexThreadState* thread 
        = dynamic_cast<MoHexThreadState*>(&search.ThreadState(0));
    if (!thread)
        throw HtpFailure() << "Thread not a MoHexThreadState!";

    HexColor color = BLACK;
    if (m_game.Board().GetColor(from) == m_game.Board().GetColor(to))
        color = m_game.Board().GetColor(from);

    const int NUM_PLAYOUTS = 10000;
    float wins = 0.0f;
    std::vector<int> won(BITSETSIZE, 0);
    std::vector<int> played(BITSETSIZE, 0);

    const MoHexBoard& mobrd = thread->GetMoHexBoard();
    HexState state(m_game.Board(), m_game.Board().WhoseTurn());
    HexPoint lastMovePlayed
        = MoveSequenceUtil::LastMoveFromHistory(m_game.History());
    for (int i = 0; i < NUM_PLAYOUTS; ++i)
    {
        PerformPlayout(thread, state, lastMovePlayed);
        for (BitsetIterator p(m_game.Board().GetEmpty()); p; ++p)
            if (mobrd.GetColor(*p) == color)
                played[*p]++;
        if (mobrd.Parent(from) != mobrd.Parent(to))
            continue;
        wins++;
        for (BitsetIterator p(m_game.Board().GetEmpty()); p; ++p)
            if (mobrd.GetColor(*p) == color)
                won[*p]++;
    }
    cmd << "INFLUENCE ";
    for (BitsetIterator p(m_game.Board().GetEmpty()); p; ++p)
    {
        float v = 0.0f;
        if (played[*p] > 0)
            v = (float)won[*p] / (float)played[*p];
#if 0        
        // zoom into [0.2, 0.8]
        v = 0.5f + (v - 0.5f) / (0.8f - 0.2f);
        if (v < 0.0f) v = 0.0f;
        if (v > 1.0f) v = 1.0f;
#endif
        cmd << ' ' << static_cast<HexPoint>(*p) 
            << ' ' << std::fixed << std::setprecision(3) << v;
    }
    cmd << " TEXT pct=" << wins * 100.0 / NUM_PLAYOUTS;
}

void MoHexEngine::PlayoutMove(HtpCommand& cmd)
{
    MoHexSearch& search = m_player.Search();
    if (!search.ThreadsCreated())
        search.CreateThreads();
    MoHexThreadState* thread 
        = dynamic_cast<MoHexThreadState*>(&search.ThreadState(0));
    if (!thread)
        throw HtpFailure() << "Thread not a MoHexThreadState!";
    HexState state(m_game.Board(), m_game.Board().WhoseTurn());
    HexPoint lastMovePlayed 
        = MoveSequenceUtil::LastMoveFromHistory(m_game.History());
    thread->StartPlayout(state, lastMovePlayed);
    bool skipRaveUpdate;
    const MoHexBoard& mobrd = thread->GetMoHexBoard();
    const ConstBoard& cbrd = mobrd.Const();
    if (mobrd.NumMoves() >= cbrd.Width() * cbrd.Height())
        return;
    HexPoint move = (HexPoint)thread->GeneratePlayoutMove(skipRaveUpdate);
    Play(state.ToPlay(), move);
    cmd << move;
}

void MoHexEngine::PlayoutWeights(HtpCommand& cmd)
{
    MoHexSearch& search = m_player.Search();
    if (!search.ThreadsCreated())
        search.CreateThreads();
    MoHexThreadState* thread 
        = dynamic_cast<MoHexThreadState*>(&search.ThreadState(0));
    if (!thread)
        throw HtpFailure() << "Thread not a MoHexThreadState!";
    if (m_game.Board().GetEmpty().none())
        return;
    HexState state(m_game.Board(), m_game.Board().WhoseTurn());
    HexPoint lastMovePlayed 
        = MoveSequenceUtil::LastMoveFromHistory(m_game.History());
    thread->StartPlayout(state, lastMovePlayed);
    bool skipRaveUpdate;
    thread->GeneratePlayoutMove(skipRaveUpdate);
    std::vector<float> weights;
    thread->Policy().GetWeightsForLastMove(weights, state.ToPlay());
    for (BitsetIterator i(m_game.Board().GetEmpty()); i; ++i)
        if (weights[*i] > 0.0f)
            cmd << ' ' << static_cast<HexPoint>(*i)
                << ' ' << std::fixed << std::setprecision(3) << weights[*i];
}

void MoHexEngine::PlayoutGlobalWeights(HtpCommand& cmd)
{
    MoHexSearch& search = m_player.Search();
    if (!search.ThreadsCreated())
        search.CreateThreads();
    MoHexThreadState* thread 
        = dynamic_cast<MoHexThreadState*>(&search.ThreadState(0));
    if (!thread)
        throw HtpFailure() << "Thread not a MoHexThreadState!";
    if (m_game.Board().GetEmpty().none())
        return;
    HexState state(m_game.Board(), m_game.Board().WhoseTurn());
    HexPoint lastMovePlayed 
        = MoveSequenceUtil::LastMoveFromHistory(m_game.History());
    thread->StartPlayout(state, lastMovePlayed);
    bool skipRaveUpdate;
    thread->GeneratePlayoutMove(skipRaveUpdate);
    std::vector<float> weights;
    thread->Policy().GetGlobalWeightsForLastMove(weights, state.ToPlay());
    for (BitsetIterator i(m_game.Board().GetEmpty()); i; ++i)
        if (weights[*i] > 0.0f)
            cmd << ' ' << static_cast<HexPoint>(*i)
                << ' ' << std::fixed << std::setprecision(3) << weights[*i];
}

void MoHexEngine::PlayoutLocalWeights(HtpCommand& cmd)
{
    MoHexSearch& search = m_player.Search();
    if (!search.ThreadsCreated())
        search.CreateThreads();
    MoHexThreadState* thread 
        = dynamic_cast<MoHexThreadState*>(&search.ThreadState(0));
    if (!thread)
        throw HtpFailure() << "Thread not a MoHexThreadState!";
    if (m_game.Board().GetEmpty().none())
        return;
    HexState state(m_game.Board(), m_game.Board().WhoseTurn());
    HexPoint lastMovePlayed 
        = MoveSequenceUtil::LastMoveFromHistory(m_game.History());
    thread->StartPlayout(state, lastMovePlayed);
    bool skipRaveUpdate;
    thread->GeneratePlayoutMove(skipRaveUpdate);
    std::vector<float> weights;
    thread->Policy().GetLocalWeightsForLastMove(weights, state.ToPlay());
    for (BitsetIterator i(m_game.Board().GetEmpty()); i; ++i)
        if (weights[*i] > 0.0f)
            cmd << ' ' << static_cast<HexPoint>(*i)
                << ' ' << std::fixed << std::setprecision(3) << weights[*i];
}

void MoHexEngine::FindTopMoves(HtpCommand& cmd)
{
    HexColor color = HtpUtil::ColorArg(cmd, 0);
    int num = 5;
    if (cmd.NuArg() >= 2)
        num = cmd.ArgMin<int>(1, 1);
    HexState state(m_game.Board(), color);
    HexBoard& brd = m_pe.SyncBoard(m_game.Board());
    if (EndgameUtil::IsDeterminedState(brd, color))
        throw HtpFailure() << "State is determined.\n";
    bitset_t consider = EndgameUtil::MovesToConsider(brd, color);
    std::vector<HexPoint> moves;
    std::vector<double> scores;
    m_player.FindTopMoves(num, state, m_game, brd, consider, 
                          m_player.MaxTime(), moves, scores);
    for (std::size_t i = 0; i < moves.size(); ++i)
        cmd << ' ' << static_cast<HexPoint>(moves[i]) 
            << ' ' << (i + 1) 
            << '@' << std::fixed << std::setprecision(3) << scores[i];
}

void MoHexEngine::SelfPlay(HtpCommand& cmd)
{
    cmd.CheckNuArg(1);
    std::size_t numGames = cmd.ArgMin<size_t>(0, 1);
    StoneBoard board(m_game.Board());
    Game game(board);
    HexState state(board.Width());
    for (size_t i = 0; i < numGames; ++i)
    {
        LogInfo() << "*********** Game " << (i + 1) << " ***********\n";
        game.NewGame();
        state.Position() = game.Board();
        state.SetToPlay(BLACK);
        
        HexPoint firstMove = BoardUtil::RandomEmptyCell(state.Position());
        game.PlayMove(state.ToPlay(), firstMove);
        state.PlayMove(firstMove);

        while (true)
        {
            double score;
            HexPoint move = m_player.GenMove(state, game,
                                             m_pe.SyncBoard(state.Position()),
                                             m_player.MaxTime(), score);
            if (HexEvalUtil::IsWinOrLoss(score))
                break;
            game.PlayMove(state.ToPlay(), move);
            state.PlayMove(move);
        }
    }    
}

//----------------------------------------------------------------------------

void MoHexEngine::MarkPrunablePatterns(HtpCommand& cmd)
{
    cmd.CheckNuArg(2);
    std::string infile = cmd.Arg(0);
    std::string outfile = cmd.Arg(1);
    std::vector<Pattern> infpat, oppfill, vul, dom;
    HashedPatternSet hoppfill, hvul, hdom;
    std::ifstream ifile;
    MiscUtil::OpenFile("mohex-prior-prune.txt", ifile);
    Pattern::LoadPatternsFromStream(ifile, infpat);
    for (size_t i = 0; i < infpat.size(); ++i)
    {
        if (infpat[i].GetType() == Pattern::DOMINATED)
            dom.push_back(infpat[i]);
        else if (infpat[i].GetType() == Pattern::VULNERABLE)
            vul.push_back(infpat[i]);
        else
            oppfill.push_back(infpat[i]);
    }
    LogInfo() << "Parsed " << oppfill.size() << " opp fill patterns, "
              << vul.size() << " vulnerable patterns, "
              << dom.size() << " domination patterns.\n";
    hoppfill.Hash(oppfill);
    hvul.Hash(vul);
    hdom.Hash(dom);

    std::ifstream f(infile.c_str());
    std::ofstream of(outfile.c_str());
    std::string line;
    if (!std::getline(f, line)) 
        throw HtpFailure("Empty file");
    of << line << '\n';

    double largestPrunedGamma = -1.0f;
    size_t numPrunable = 0;

    StoneBoard brd(11);
    const ConstBoard& cbrd = brd.Const();
    PatternState pastate(brd);
    while (f.good()) 
    {
        if (!std::getline(f, line)) 
            break;
        if (line.size() < 5)
            continue;
        int type, killer;
        size_t w, a;
        std::string pattern, gamma;
        std::istringstream ifs(line);
        ifs >> gamma;
        ifs >> w;
        ifs >> a;
        ifs >> pattern;
        ifs >> type;
        ifs >> killer;

        int size = (int)pattern.size();
        brd.StartNewGame();
        for (int i = 1; i <= size; ++i)
        {
            HexPoint p = cbrd.PatternPoint(HEX_CELL_F6, i);
            char pi = pattern[i - 1];
            if (pi == '1' || pi == '3' || pi == '5')
                brd.SetColor(BLACK, p);
            else if (pi == '2' || pi == '4')
                brd.SetColor(WHITE, p);
        }
        pastate.Update();
        
        type = 0;
        killer = 0;
        PatternHits hits;
        pastate.MatchOnCell(hoppfill, HEX_CELL_F6, 
                            PatternState::STOP_AT_FIRST_HIT, hits);
        if (hits.size() > 0)
            type = 1;
        else 
        {
            pastate.MatchOnCell(hvul, HEX_CELL_F6,
                                PatternState::STOP_AT_FIRST_HIT, hits);
            if (hits.size() > 0)
                type = 2;
            else 
            {
                pastate.MatchOnCell(hdom, HEX_CELL_F6, 
                                    PatternState::STOP_AT_FIRST_HIT, hits);
                if (hits.size() > 0)
                {
                    type = 3;
                    const std::vector<HexPoint>& moves1 = hits[0].Moves1();
                    for (int i = 1; i <= size; ++i)
                        if (cbrd.PatternPoint(HEX_CELL_F6, i) == moves1[0])
                            killer = i;
                    if (killer == 0)
                        throw BenzeneException("Killer not found!");
                }
            }
        }
        if (type > 0) 
        {
            LogInfo() << brd.Write() << '\n'
                      << "gamma=" << gamma 
                      << " pat=" << hits[0].GetPattern()->GetName() 
                      << " type=" << type 
                      << " killer=" << killer << '\n';
            numPrunable++;

            std::istringstream ss(gamma);
            double fgamma;
            ss >> fgamma;
            if (fgamma > largestPrunedGamma)
                largestPrunedGamma = fgamma;
        }
        of << std::setw(16) << std::fixed << std::setprecision(6) << gamma 
           << std::setw(11) << w 
           << std::setw(11) << a
           << std::setw(19) << pattern
           << std::setw(11) << type 
           << std::setw(11) << killer
           << '\n';
    }
    of.close();
    f.close();
    LogInfo() << "numPrunable=" << numPrunable << '\n';
    LogInfo() << "largestPrunedGamma=" << largestPrunedGamma << '\n';
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
    // Force it to search even if root has a singleton consider set
    bool oldSingleton = m_player.SearchSingleton();
    m_player.SetSearchSingleton(true);
    DoSearch(m_game.Board().WhoseTurn(), 600);
    m_player.SetSearchSingleton(oldSingleton);
}

void MoHexEngine::StopPonder()
{
    SgSetUserAbort(true);
}

#endif // GTPENGINE_PONDER

//----------------------------------------------------------------------------

