//----------------------------------------------------------------------------
/** @file BenzeneTestEngine.cpp
 */
//----------------------------------------------------------------------------

#include "SgSystem.h"

#include "Misc.hpp"
#include "BenzeneTestEngine.hpp"
#include "PerfectPlayer.hpp"
#include "SwapCheck.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

BenzeneTestEngine::BenzeneTestEngine(GtpInputStream& in, GtpOutputStream& out,
                                     int boardsize)
    : BenzeneHtpEngine(in, out, boardsize),
      m_player(0)
{
    RegisterCmd("set_player", &BenzeneTestEngine::CmdSetPlayer);
}

BenzeneTestEngine::~BenzeneTestEngine()
{
}

//----------------------------------------------------------------------------

void BenzeneTestEngine::RegisterCmd(const std::string& name,
                              GtpCallback<BenzeneTestEngine>::Method method)
{
    Register(name, new GtpCallback<BenzeneTestEngine>(this, method));
}

//----------------------------------------------------------------------------

double BenzeneTestEngine::TimeForMove(HexColor color)
{
    return m_game.TimeRemaining(color);
}

/** Generates a move. */
HexPoint BenzeneTestEngine::GenMove(HexColor color, bool useGameClock)
{
    if (m_player == 0)
        throw HtpFailure() << "No player specified!";
    SG_UNUSED(useGameClock);
    if (SwapCheck::PlaySwap(m_game, color))
        return SWAP_PIECES;
    HexEval score;
    double maxTime = TimeForMove(color);
    return m_player->GenMove(m_pe.SyncBoard(m_game.Board()), m_game, 
                             color, maxTime, score);
}

void BenzeneTestEngine::CmdSetPlayer(HtpCommand& cmd)
{
    cmd.CheckNuArg(1);
    std::string name = cmd.Arg(0);
    if (name == "perfect")
        m_player.reset(new PerfectPlayer(m_dfpnSolver, m_dfpnPositions));
    else if (name == "none")
        m_player.reset(0);
    else
        throw HtpFailure() << "Unknown player name!";
}

//----------------------------------------------------------------------------
// Pondering

#if GTPENGINE_PONDER

void BenzeneTestEngine::InitPonder()
{
}

void BenzeneTestEngine::Ponder()
{
}

void BenzeneTestEngine::StopPonder()
{
}

#endif // GTPENGINE_PONDER

//----------------------------------------------------------------------------
