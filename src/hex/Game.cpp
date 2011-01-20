//----------------------------------------------------------------------------
/** @file Game.cpp */
//----------------------------------------------------------------------------

#include "SgSystem.h"

#include "Game.hpp"
#include "Groups.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

Game::Game(StoneBoard& board)
    : m_board(&board),
      m_allow_swap(false),
      m_game_time(1800)
{
    NewGame();
}

void Game::NewGame()
{
    LogFine() << "Game::NewGame()\n";
    m_board->StartNewGame();
    m_time_remaining[BLACK] = m_game_time;
    m_time_remaining[WHITE] = m_game_time;
    m_history.clear();
}

void Game::SetGameTime(double time)
{
    m_game_time = time;
    m_time_remaining[BLACK] = m_game_time;
    m_time_remaining[WHITE] = m_game_time;
}

Game::ReturnType Game::PlayMove(HexColor color, HexPoint cell)
{
    if (cell < 0 || cell >= FIRST_INVALID || !m_board->Const().IsValid(cell))
        return INVALID_MOVE;
    if (color == EMPTY) 
        return INVALID_MOVE;
    if (HexPointUtil::isSwap(cell))
    {
        if (!m_allow_swap || m_history.size() != 1)
            return INVALID_MOVE;
    }
    if (m_board->IsPlayed(cell))
        return OCCUPIED_CELL;

    m_board->PlayMove(color, cell);
    m_history.push_back(Move(color, cell));

    return VALID_MOVE;
}

void Game::UndoMove()
{
    if (m_history.empty())
        return;
    m_board->UndoMove(m_history.back().Point());
    m_history.pop_back();
}

//----------------------------------------------------------------------------

bool GameUtil::IsGameOver(const Game& game)
{
    Groups groups;
    GroupBuilder::Build(game.Board(), groups);
    return groups.IsGameOver();
}

bool GameUtil::SequenceFromPosition(const Game& game, const StoneBoard& pos, 
                                    MoveSequence& seq)
{
    if (game.Board().Const() != pos.Const())
        return false;
    StoneBoard cur(pos);
    cur.StartNewGame();
    if (cur == pos)
    {
        seq = game.History();
        return true;
    }
    const MoveSequence& history = game.History();
    for (MoveSequence::const_iterator it = history.begin(); 
         it != history.end(); ++it)
    {    
        const Move& move = *it;
        cur.PlayMove(move.Color(), move.Point());
        if (cur == pos)
        {
            LogInfo() << "Position matched!\n";
            seq.assign(++it, history.end());
            return true;
        }
    }
    return false;
}

void GameUtil::HistoryToSequence(const MoveSequence& history, 
                                 PointSequence& sequence)
{
    sequence.clear();
    for (MoveSequence::const_iterator it = history.begin(); 
         it != history.end(); ++it)
    {    
        const Move& move = *it;
        sequence.push_back(move.Point());
    }
}

//----------------------------------------------------------------------------
