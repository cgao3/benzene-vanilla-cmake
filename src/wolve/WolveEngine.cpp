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

template<typename TYPE>
void ParseDashSeparatedString(const std::string& str, std::vector<TYPE>& out)
{
    // remove the '-' separators
    std::string widths(str);
    for (std::size_t i = 0; i < widths.size(); ++i)
        if (widths[i] == '-') widths[i] = ' ';

    // parse the ' ' separated widths
    std::istringstream is;
    is.str(widths);
    while (is)
    {
        TYPE j;
        if (is >> j)
            out.push_back(j);
    }
}

//----------------------------------------------------------------------------

} // namespace

//----------------------------------------------------------------------------

WolveEngine::WolveEngine(int boardsize, WolvePlayer& player)
    : CommonHtpEngine(boardsize),
      m_player(player),
      m_book(0),
      m_bookCheck(m_book),
      m_bookCommands(m_game, m_pe, m_book, m_bookCheck),
      m_cacheBook()
{
    m_bookCommands.Register(*this);
    RegisterCmd("param_wolve", &WolveEngine::CmdParam);
    RegisterCmd("wolve-scores", &WolveEngine::CmdScores);
    RegisterCmd("wolve-data", &WolveEngine::CmdData);
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
    HexPoint bookMove = m_bookCheck.BestMove(state);
    if (bookMove != INVALID_POINT)
        return bookMove;
    if (m_cacheBook.Exists(state))
        return m_cacheBook[state];
    double maxTime = TimeForMove(color);
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

void WolveEngine::CmdParam(HtpCommand& cmd)
{
    WolveSearch& search = m_player.Search();

    if (cmd.NuArg() == 0) 
    {
        cmd << '\n'
            << "[bool] backup_ice_info "
            << search.BackupIceInfo() << '\n'
            << "[string] max_time "
            << m_player.MaxTime() << '\n'
            << "[bool] use_guifx "
            << search.GuiFx() << '\n'
            << "[string] ply_width " 
            << MiscUtil::PrintVector(search.PlyWidth()) << '\n'
            << "[string] search_depths "
            << MiscUtil::PrintVector(m_player.SearchDepths()) << '\n'
            << "[bool] search_singleton "
            << m_player.SearchSingleton() << '\n'
            << "[bool] use_parallel_solver "
            << m_useParallelSolver << '\n'
            << "[bool] use_time_management "
            << m_player.UseTimeManagement() << '\n';
    }
    else if (cmd.NuArg() == 2)
    {
        std::string name = cmd.Arg(0);
        if (name == "backup_ice_info")
            search.SetBackupIceInfo(cmd.Arg<bool>(1));
        else if (name == "max_time")
            m_player.SetMaxTime(cmd.Arg<float>(1));
        else if (name == "ply_width")
        {
            std::vector<std::size_t> plywidth;
            ParseDashSeparatedString(cmd.Arg(1), plywidth);
            search.SetPlyWidth(plywidth);
        } 
        else if (name == "search_depths")
        {
            std::vector<std::size_t> depths;
            ParseDashSeparatedString(cmd.Arg(1), depths);
            m_player.SetSearchDepths(depths);
        }
        else if (name == "use_guifx")
            search.SetGuiFx(cmd.Arg<bool>(1));
        else if (name == "search_singleton")
            m_player.SetSearchSingleton(cmd.Arg<bool>(1));
        else if (name == "use_parallel_solver")
            m_useParallelSolver = cmd.Arg<bool>(1);
        else if (name == "use_time_management")
            m_player.SetUseTimeManagement(cmd.Arg<bool>(1));
        else
            throw HtpFailure() << "Unknown parameter: " << name;
    }
    else
        throw HtpFailure("Expected 0 or 2 arguments");
}

/** Prints scores of moves. */
void WolveEngine::CmdScores(HtpCommand& cmd)
{
    HexState state(m_game.Board(), m_game.Board().WhoseTurn());
    const SgSearchHashTable& hashTable = m_player.HashTable();
    cmd << WolveSearchUtil::PrintScores(state, hashTable);
}

/** Returns data on this state in the hashtable. */
void WolveEngine::CmdData(HtpCommand& cmd)
{
    const SgSearchHashTable& hashTable = m_player.HashTable();
    SgSearchHashData data;
    HexState state(m_game.Board(), m_game.Board().WhoseTurn());
    if (hashTable.Lookup(state.Hash(), &data))
        cmd << "[score=" << data.Value()
            << " bestMove=" << m_player.Search().MoveString(data.BestMove())
            << " isExact=" << data.IsExactValue()
            << " depth=" << data.Depth()
            << ']';
}

//----------------------------------------------------------------------------
// Pondering

#if GTPENGINE_PONDER

void WolveEngine::InitPonder()
{
}

void WolveEngine::Ponder()
{
}

void WolveEngine::StopPonder()
{
}

#endif // GTPENGINE_PONDER

//----------------------------------------------------------------------------
