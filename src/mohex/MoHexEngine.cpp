//----------------------------------------------------------------------------
/** @file MoHexEngine.cpp
 */
//----------------------------------------------------------------------------

#include "SgSystem.h"

#include "BitsetIterator.hpp"
#include "MoHexEngine.hpp"
#include "MoHexPlayer.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

MoHexEngine::MoHexEngine(GtpInputStream& in, GtpOutputStream& out, 
                         int boardsize, BenzenePlayer& player)
    : BenzeneHtpEngine(in, out, boardsize, player),
      m_bookCommands(m_game, m_pe, 
                     BenzenePlayerUtil::GetInstanceOf<BookCheck>(&player),
                     *BenzenePlayerUtil::GetInstanceOf<MoHexPlayer>(&player))
{
    m_bookCommands.Register(*this);
    RegisterCmd("param_mohex", &MoHexEngine::MoHexParam);
    RegisterCmd("param_mohex_policy", &MoHexEngine::MoHexPolicyParam);
    RegisterCmd("mohex-save-tree", &MoHexEngine::SaveTree);
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
    /** @todo Use a proper time control mechanism! */
    MoHexPlayer* mohex = 
        BenzenePlayerUtil::GetInstanceOf<MoHexPlayer>(&m_player);
    if (!mohex)
        throw HtpFailure("No MoHex instance!");

    return std::min(m_game.TimeRemaining(color), mohex->MaxTime());
}

//----------------------------------------------------------------------------

void MoHexEngine::MoHexPolicyParam(HtpCommand& cmd)
{
    MoHexPlayer* mohex = 
        BenzenePlayerUtil::GetInstanceOf<MoHexPlayer>(&m_player);
    if (!mohex)
        throw HtpFailure("No MoHex instance!");
    HexUctPolicyConfig& config = mohex->SharedPolicy().Config();
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
    MoHexPlayer* mohex = 
        BenzenePlayerUtil::GetInstanceOf<MoHexPlayer>(&m_player);
    if (!mohex)
        throw HtpFailure("No MoHex instance!");

    HexUctSearch& search = mohex->Search();

    if (cmd.NuArg() == 0) 
    {
        cmd << '\n'
            << "[bool] backup_ice_info "
            << mohex->BackupIceInfo() << '\n'
            << "[bool] lock_free " 
            << search.LockFree() << '\n'
            << "[bool] ponder "
            << mohex->Ponder() << '\n'
            << "[bool] reuse_subtree " 
            << mohex->ReuseSubtree() << '\n'
            << "[bool] use_livegfx "
            << search.LiveGfx() << '\n'
            << "[bool] use_rave "
            << search.Rave() << '\n'
            << "[string] bias_term "
            << search.BiasTermConstant() << '\n'
            << "[string] expand_threshold "
            << search.ExpandThreshold() << '\n'
            << "[string] knowledge_threshold "
            << search.KnowledgeThreshold() << '\n'
            << "[string] livegfx_interval "
            << search.LiveGfxInterval() << '\n'
            << "[string] max_games "
            << mohex->MaxGames() << '\n'
            << "[string] max_memory "
            << search.MaxNodes() * 2 * sizeof(SgUctNode) << '\n'
            << "[string] max_nodes "
            << search.MaxNodes() << '\n'
            << "[string] max_time "
            << mohex->MaxTime() << '\n'
            << "[string] num_threads "
            << search.NumberThreads() << '\n'
            << "[string] playout_update_radius "
            << search.PlayoutUpdateRadius() << '\n'
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
            mohex->SetBackupIceInfo(cmd.BoolArg(1));
        else if (name == "lock_free")
            search.SetLockFree(cmd.BoolArg(1));
        else if (name == "ponder")
            mohex->SetPonder(cmd.BoolArg(1));
        else if (name == "use_livegfx")
            search.SetLiveGfx(cmd.BoolArg(1));
        else if (name == "use_rave")
            search.SetRave(cmd.BoolArg(1));
        else if (name == "reuse_subtree")
           mohex->SetReuseSubtree(cmd.BoolArg(1));
        else if (name == "bias_term")
            search.SetBiasTermConstant(cmd.FloatArg(1));
        else if (name == "expand_threshold")
            search.SetExpandThreshold(cmd.IntArg(1, 0));
        else if (name == "knowledge_threshold")
            search.SetKnowledgeThreshold(cmd.SizeTypeArg(1, 0));
        else if (name == "livegfx_interval")
            search.SetLiveGfxInterval(cmd.IntArg(1, 0));
        else if (name == "max_games")
            mohex->SetMaxGames(cmd.IntArg(1, 0));
        else if (name == "max_memory")
            search.SetMaxNodes(cmd.SizeTypeArg(1, 1) / sizeof(SgUctNode) / 2);
        else if (name == "max_time")
            mohex->SetMaxTime(cmd.FloatArg(1));
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
        else if (name == "tree_update_radius")
            search.SetTreeUpdateRadius(cmd.IntArg(1, 0));
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
    MoHexPlayer* mohex = 
        BenzenePlayerUtil::GetInstanceOf<MoHexPlayer>(&m_player);
    if (!mohex)
        throw HtpFailure("No MoHex instance!");
    HexUctSearch& search = mohex->Search();

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

//----------------------------------------------------------------------------
// Pondering

#if GTPENGINE_PONDER

void MoHexEngine::InitPonder()
{
    SgSetUserAbort(false);
}

void MoHexEngine::Ponder()
{
    MoHexPlayer* mohex = 
        BenzenePlayerUtil::GetInstanceOf<MoHexPlayer>(&m_player);
    HexAssert(mohex);

    if (!mohex->Ponder())
        return;

    if (!mohex->ReuseSubtree())
    {
        LogWarning() << "Pondering requires reuse_subtree." << '\n';
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

    LogInfo() << "MoHexEngine::Ponder: start" << '\n';

    // Do a search for at most 10 minutes.
    HexEval score;
    m_player.genmove(m_pe.SyncBoard(m_game.Board()), m_game,
                     m_game.Board().WhoseTurn(), 600, score);

}

void MoHexEngine::StopPonder()
{
    SgSetUserAbort(true);
}

#endif // GTPENGINE_PONDER

//----------------------------------------------------------------------------
