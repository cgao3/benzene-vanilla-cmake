//----------------------------------------------------------------------------
/** @file Game.hpp
 */
//----------------------------------------------------------------------------

#ifndef GAME_HPP
#define GAME_HPP

#include "Hex.hpp"
#include "Move.hpp"
#include "StoneBoard.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** A Game of Hex. 
    
    @todo Store time-info in history, so that undoing moves fixes the
    time remaining.
*/
class Game
{
public:
    
    /** Creates a new game on the given board. Calls NewGame(). */
    explicit Game(StoneBoard& brd);

    //----------------------------------------------------------------------

    /** Starts a new game. The board and move history are cleared. */
    void NewGame();

    //----------------------------------------------------------------------

    typedef enum { INVALID_MOVE, OCCUPIED_CELL, VALID_MOVE } ReturnType;

    /** If move is invalid (color is not BLACK or WHITE, point is an
        invalid point, point is swap when swap not available) then
        INVALID_MOVE is returned and game/board is not changed. If
        point is occupied, returns CELL_OCCUPIED. Otherwise, returns
        VALID_MOVE and plays the move to the board and adds it to the
        game's history.  */
    ReturnType PlayMove(HexColor color, HexPoint point);

    /** The last move is cleared from the board and removed from the
        game history.  Does nothing if history is empty. */
    void UndoMove();

    //----------------------------------------------------------------------

    /** Returns the time remaining for color. */
    double TimeRemaining(HexColor color) const;

    /** Sets the time remaining for the given player. */
    void SetTimeRemaining(HexColor color, double time);

    //----------------------------------------------------------------------

    /** Returns the game board. */
    StoneBoard& Board();

    /** Returns a constant reference to the game board. */
    const StoneBoard& Board() const;

    /** Change board game is played on. */
    void SetBoard(StoneBoard& brd);
    
    /** Returns the history of moves. */
    const MoveSequence& History() const;

    /** Whether swap move is allowed to be played or not. */
    bool AllowSwap() const;

    /** See SwapAllowed() */
    void SetAllowSwap(bool enable);

    /** Amount of time given to each player at start of game. */
    double GameTime() const;

    /** See GameTime() 
        Can only be called if no moves have been played yet.
    */
    void SetGameTime(double time);

    //----------------------------------------------------------------------

private:

    StoneBoard* m_board;

    MoveSequence m_history;

    double m_time_remaining[BLACK_AND_WHITE];

    /** See SwapAllowed() */
    bool m_allow_swap;

    /** See GameTime() */
    double m_game_time;
};

inline StoneBoard& Game::Board() 
{
    return *m_board;
}

inline const StoneBoard& Game::Board() const
{
    return *m_board;
}

inline void Game::SetBoard(StoneBoard& brd)
{
    m_board = &brd;
}

inline const MoveSequence& Game::History() const
{
    return m_history;
}

inline double Game::TimeRemaining(HexColor color) const
{
    return m_time_remaining[color];
}

inline void Game::SetTimeRemaining(HexColor color, double time)
{
    m_time_remaining[color] = time;
}

inline bool Game::AllowSwap() const
{
    return m_allow_swap;
}

inline void Game::SetAllowSwap(bool enable)
{
    m_allow_swap = enable;
}

inline double Game::GameTime() const
{
    return m_game_time;
}

//----------------------------------------------------------------------------

/** Utilities on Games. */
namespace GameUtil
{
    /** Returns true if the game is over, that is, if the current
        position contains a solid connection for one player. */
    bool IsGameOver(const Game& game);

    /** If game contains the given position, returns the move history
        from that position to the current end of the game. */
    bool SequenceFromPosition(const Game& game, const StoneBoard& pos, 
                              MoveSequence& seq);

    /** Converts a game history into a sequence of points played. */
    void HistoryToSequence(const MoveSequence& history, 
                           PointSequence& sequence);
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif  // GAME_HPP
