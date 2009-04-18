//----------------------------------------------------------------------------
/** @file
 */
//----------------------------------------------------------------------------

#ifndef HEXPLAYER_HPP
#define HEXPLAYER_HPP

#include "Game.hpp"
#include "HexBoard.hpp"

//----------------------------------------------------------------------------

/** Abstract base class for all players.
    
    Players are responsible for generating moves from a given
    boardstate.
*/
class HexPlayer
{
public:

    /** Destructor. */
    virtual ~HexPlayer() { };

    /** Returns a unique identifier for this player. */
    virtual std::string name() const=0;

    /** Generates a move from this game position with color to move
        next.  If time_remaining is negative, then there is no
        timelimit. Otherwise, time_remaining is the number of seconds
        the player has to determine all future moves in this game.  If
        game is already over, player should return RESIGN. The player
        can store a score for the move it generated in score (default
        is 0).

        @param brd HexBoard to do work on. Board position is set to
               the board position as that of the game board. 
        @param game_state Game history up to this position.
        @param color Color to move in this position.
        @param time_remaining Time in minutes remaining in game.
        @param score Return score of move here. 
    */
    virtual HexPoint genmove(HexBoard& brd, 
                             const Game& game_state, HexColor color,
                             double time_remaining, double& score)=0;

};

//----------------------------------------------------------------------------

#endif // HEXPLAYER_HPP
