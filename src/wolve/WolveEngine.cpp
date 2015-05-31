//----------------------------------------------------------------------------
/** @file WolveEngine.cpp */
//----------------------------------------------------------------------------

#include "SgSystem.h"

#include "BitsetIterator.hpp"
#include "Misc.hpp"
#include "PlayAndSolve.hpp"
#include "SwapCheck.hpp"
#include "WolveEngine.hpp"
#include "WolvePlayer.hpp"
#include "WolveTimeControl.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

namespace {

//----------------------------------------------------------------------------

std::vector<std::size_t> PlyWidthsFromString(const std::string& val)
{
    std::vector<std::size_t> v;
    std::istringstream is(val);
    std::size_t t;
    while (is >> t)
        v.push_back(t);
    return v;
}

//----------------------------------------------------------------------------

} // namespace

//----------------------------------------------------------------------------

WolveEngine::WolveEngine(int boardsize, WolvePlayer& player)
    : CommonHtpEngine(boardsize),
      m_player(player),
      m_cacheBook(),
      m_useCacheBook(true)
{
    RegisterCmd("param_wolve", &WolveEngine::CmdParam);
    RegisterCmd("wolve-get-pv", &WolveEngine::CmdGetPV);
    RegisterCmd("wolve-scores", &WolveEngine::CmdScores);
    RegisterCmd("wolve-data", &WolveEngine::CmdData);
    RegisterCmd("wolve-clear-hash", &WolveEngine::CmdClearHash);
}

WolveEngine::~WolveEngine()
{
}

//----------------------------------------------------------------------------

void WolveEngine::RegisterCmd(const std::string& name,
                              GtpCallback<WolveEngine>::Method method)
{
    Register(name, new GtpCallback<WolveEngine>(this, method));
}

//----------------------------------------------------------------------------

double WolveEngine::TimeForMove(HexColor c)
{
    if (m_player.UseTimeManagement())
        return WolveTimeControl::TimeForMove(m_game, m_game.TimeRemaining(c));
    return m_player.MaxTime();
}

/** Generates a move. */
HexPoint WolveEngine::GenMove(HexColor color, bool useGameClock)
{
    SG_UNUSED(useGameClock);
    if (SwapCheck::PlaySwap(m_game, color))
        return SWAP_PIECES;
    HexState state(m_game.Board(), color);
    if (m_useCacheBook && m_cacheBook.Exists(state))
    {
        LogInfo() << "Playing move from cache book.\n";
        return m_cacheBook[state];
    }
    double maxTime = TimeForMove(color);
    return DoSearch(color, maxTime);
}

HexPoint WolveEngine::DoSearch(HexColor color, double maxTime)
{
    HexState state(m_game.Board(), color);
    if (m_useParallelSolver)
    {
        PlayAndSolve ps(*m_pe.brd, *m_se.brd, m_player, m_dfpnSolver, 
                        m_dfpnPositions, m_game);
        return ps.GenMove(state, maxTime);
    }
    double score;
    return m_player.GenMove(state, m_game, m_pe.SyncBoard(m_game.Board()),
                            maxTime, score);
}


void WolveEngine::CmdAnalyzeCommands(HtpCommand& cmd)
{
    CommonHtpEngine::CmdAnalyzeCommands(cmd);
    cmd <<
        "param/Wolve Param/param_wolve\n"
        "var/Wolve PV/wolve-get-pv\n"
        "pspairs/Wolve Scores/wolve-scores\n"
        "none/Wolve Clear Hashtable/wolve-clear-hash\n"
        "scores/Wolve Data/wolve-data\n";
}

/** Wolve parameters. */
void WolveEngine::CmdParam(HtpCommand& cmd)
{
    WolveSearch& search = m_player.Search();

    if (cmd.NuArg() == 0) 
    {
        cmd << '\n'
            << "[bool] backup_ice_info "
            << search.BackupIceInfo() << '\n'
            << "[bool] ponder "
            << m_player.Ponder() << '\n'
            << "[bool] use_cache_book "
            << m_useCacheBook << '\n'
            << "[bool] use_guifx "
            << search.GuiFx() << '\n'
            << "[bool] search_singleton "
            << m_player.SearchSingleton() << '\n'
            << "[bool] use_parallel_solver "
            << m_useParallelSolver << '\n'
            << "[bool] use_time_management "
            << m_player.UseTimeManagement() << '\n'
            << "[bool] use_early_abort " 
            << m_player.UseEarlyAbort() << '\n'
            << "[string] ply_width " 
            << search.PlyWidth() << '\n'
            << "[string] specific_ply_widths \"" 
            << MiscUtil::PrintVector(search.SpecificPlyWidths()) << "\"\n"
            << "[string] max_depth "
            << m_player.MaxDepth() << '\n'
            << "[string] max_time "
            << m_player.MaxTime() << '\n'
            << "[string] min_depth "
            << m_player.MinDepth() << '\n'
            << "[string] tt_bits "
            << (m_player.HashTable() 
                ? log2(m_player.HashTable()->MaxHash()) : 0);
    }
    else if (cmd.NuArg() == 2)
    {
        std::string name = cmd.Arg(0);
        if (name == "backup_ice_info")
            search.SetBackupIceInfo(cmd.Arg<bool>(1));
        else if (name == "ponder")
            m_player.SetPonder(cmd.Arg<bool>(1));
        else if (name == "max_time")
            m_player.SetMaxTime(cmd.Arg<float>(1));
        else if (name == "ply_width")
            search.SetPlyWidth(cmd.ArgMin<std::size_t>(1, 1));
        else if (name == "specific_ply_widths")
        {
            std::vector<std::size_t> plywidth 
                = PlyWidthsFromString(cmd.Arg(1));
            search.SetSpecificPlyWidths(plywidth);
        } 
        else if (name == "max_depth")
            m_player.SetMaxDepth(cmd.ArgMin<std::size_t>(1, 1));
        else if (name == "min_depth")
            m_player.SetMinDepth(cmd.ArgMin<std::size_t>(1, 1));
        else if (name == "use_guifx")
            search.SetGuiFx(cmd.Arg<bool>(1));
        else if (name == "search_singleton")
            m_player.SetSearchSingleton(cmd.Arg<bool>(1));
        else if (name == "tt_bits")
	{
	    int bits = cmd.ArgMin<int>(1, 0);
	    if (bits == 0)
		m_player.SetHashTable(0);
	    else
		m_player.SetHashTable(new SgSearchHashTable(1 << bits));
        }
        else if (name == "use_cache_book")
            m_useCacheBook = cmd.Arg<bool>(1);
        else if (name == "use_parallel_solver")
            m_useParallelSolver = cmd.Arg<bool>(1);
        else if (name == "use_time_management")
            m_player.SetUseTimeManagement(cmd.Arg<bool>(1));
        else if (name == "use_early_abort")
            m_player.SetUseEarlyAbort(cmd.Arg<bool>(1));
        else
            throw HtpFailure() << "Unknown parameter: " << name;
    }
    else
        throw HtpFailure("Expected 0 or 2 arguments");
}

void WolveEngine::CmdGetPV(HtpCommand& cmd)
{
    HexState state(m_game.Board(), m_game.Board().WhoseTurn());
    const SgSearchHashTable* hashTable = m_player.HashTable();
    std::vector<HexPoint> seq;
    WolveSearchUtil::ExtractPVFromHashTable(state, *hashTable, seq);
    for (std::size_t i = 0; i < seq.size(); ++i)
        cmd << seq[i] << ' ';
}

/** Prints scores of moves. */
void WolveEngine::CmdScores(HtpCommand& cmd)
{
    HexState state(m_game.Board(), m_game.Board().WhoseTurn());
    const SgSearchHashTable* hashTable = m_player.HashTable();
    if (!hashTable)
        throw HtpFailure("No hashtable!");
    cmd << WolveSearchUtil::PrintScores(state, *hashTable);
}

/** Returns data on this state in the hashtable. */
void WolveEngine::CmdData(HtpCommand& cmd)
{
    const SgSearchHashTable* hashTable = m_player.HashTable();
    if (!hashTable)
        throw HtpFailure("No hashtable!");
    SgSearchHashData data;
    HexState state(m_game.Board(), m_game.Board().WhoseTurn());
    if (hashTable->Lookup(state.Hash(), &data))
        cmd << "[score=" << data.Value()
            << " bestMove=" << m_player.Search().MoveString(data.BestMove())
            << " isExact=" << data.IsExactValue()
            << " isLower=" << data.IsOnlyLowerBound()
            << " isUpper=" << data.IsOnlyUpperBound()
            << " depth=" << data.Depth()
            << ']';
}

void WolveEngine::CmdClearHash(HtpCommand& cmd)
{
    cmd.CheckArgNone();
    SgSearchHashTable* hashTable = m_player.HashTable();
    if (!hashTable)
        throw HtpFailure("No hashtable!");
    hashTable->Clear();
}

//----------------------------------------------------------------------------
// Pondering

#if GTPENGINE_PONDER

void WolveEngine::InitPonder()
{
    SgSetUserAbort(false);
}

void WolveEngine::Ponder()
{
    if (!m_player.Ponder())
        return;
    // Call DoSearch() after 0.2 seconds delay to avoid calls 
    // in very short intervals between received commands
    boost::xtime time;
    #if BOOST_VERSION >= 105000
        boost::xtime_get(&time, boost::TIME_UTC_);
    #else
        boost::xtime_get(&time, boost::TIME_UTC);
    #endif
    for (int i = 0; i < 200; ++i)
    {
        if (SgUserAbort())
            return;
        time.nsec += 1000000; // 1 msec
        boost::thread::sleep(time);
    }
    LogInfo() << "WolveEngine::Ponder: start\n";
    // Search for at most 10 minutes.
    // Force it to search even if root has a singleton consider set
    bool oldSingleton = m_player.SearchSingleton();
    m_player.SetSearchSingleton(true);
    DoSearch(m_game.Board().WhoseTurn(), 600);
    m_player.SetSearchSingleton(oldSingleton);
}

void WolveEngine::StopPonder()
{
    SgSetUserAbort(true);
}

#endif // GTPENGINE_PONDER

//----------------------------------------------------------------------------
