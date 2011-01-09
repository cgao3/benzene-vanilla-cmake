//----------------------------------------------------------------------------
/** @file BenzeneTestEngine.cpp */
//----------------------------------------------------------------------------

#include "SgSystem.h"

#include "Misc.hpp"
#include "BenzeneTestEngine.hpp"
#include "PerfectPlayer.hpp"
#include "SwapCheck.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

BenzeneTestEngine::BenzeneTestEngine(int boardsize)
    : CommonHtpEngine(boardsize),
      m_player(0)
{
    RegisterCmd("set_player", &BenzeneTestEngine::CmdSetPlayer);
    RegisterCmd("param_player", &BenzeneTestEngine::CmdParamPlayer);
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
    return m_player->GenMove(HexState(m_game.Board(), color), m_game, 
                             m_pe.SyncBoard(m_game.Board()), maxTime, score);
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

void BenzeneTestEngine::CmdParamPlayer(HtpCommand& cmd)
{
    if (m_player.get() == 0)
        throw HtpFailure() << "No player specified!";
    if (m_player->Name() == "perfect")
    {
        PerfectPlayer* player = dynamic_cast<PerfectPlayer*>(m_player.get());
        if (!player)
            throw HtpFailure() << "Not an instance of PerfectPlayer!";
        if (cmd.NuArg() == 0) 
        {
            cmd << '\n'
                << "[bool] propagate_backwards "
                << player->PropagateBackwards() << '\n'
                << "[string] max_time "
                << player->MaxTime() << '\n';
        }
        else if (cmd.NuArg() == 2)
        {
            std::string name = cmd.Arg(0);
            if (name == "max_time")
                player->SetMaxTime(cmd.ArgMin<float>(1, 0.0));
            else if (name == "propagate_backwards")
                player->SetPropagateBackwards(cmd.Arg<bool>(1));
            else
                throw HtpFailure() << "Unknown parameter: " << name;
        }
        else
            throw HtpFailure("Expected 0 or 2 arguments");
    }
    else
        throw HtpFailure("No parameters for this player!");
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
