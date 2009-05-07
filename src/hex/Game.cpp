//----------------------------------------------------------------------------
/** @file
 */
//----------------------------------------------------------------------------

#include "Game.hpp"

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
    LogFine() << "Game::NewGame()" << '\n';
    m_board->startNewGame();
    m_time_remaining[BLACK] = m_game_time;
    m_time_remaining[WHITE] = m_game_time;
    m_history.clear();
}

Game::ReturnType Game::PlayMove(HexColor color, HexPoint cell)
{
    if (cell < 0 || cell >= FIRST_INVALID || !m_board->isValid(cell))
        return INVALID_MOVE;
    if (color == EMPTY) 
        return INVALID_MOVE;
    if (HexPointUtil::isSwap(cell))
    {
        if (!m_allow_swap || m_history.size() != 1)
            return INVALID_MOVE;
    }
    if (m_board->isPlayed(cell))
        return OCCUPIED_CELL;

    m_board->playMove(color, cell);
    m_history.push_back(Move(color, cell));

    return VALID_MOVE;
}

void Game::UndoMove()
{
    if (m_history.empty())
        return;
    m_board->undoMove(m_history.back().point());
    m_history.pop_back();
}

//----------------------------------------------------------------------------

bool GameUtil::SequenceFromPosition(const Game& game, const StoneBoard& pos, 
                                    MoveSequence& seq)
{
    if (game.Board().Const() != pos.Const())
        return false;
    StoneBoard cur(pos);
    cur.startNewGame();
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
        cur.playMove(move.color(), move.point());
        if (cur == pos)
        {
            LogInfo() << "Position matched!" << '\n';
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
        sequence.push_back(move.point());
    }
}

//----------------------------------------------------------------------------
