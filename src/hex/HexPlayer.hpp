//----------------------------------------------------------------------------
/** @file
 */
//----------------------------------------------------------------------------

#ifndef HEXPLAYER_HPP
#define HEXPLAYER_HPP

#include "Game.hpp"
#include "HexBoard.hpp"

_BEGIN_BENZENE_NAMESPACE_

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
        next.  If max_time is negative, then there is no
        timelimit. Otherwise, max_time is the number of seconds the
        player has to generate this move. If game is already over,
        player should return RESIGN. The player can store a score for
        the move it generated in score.

        @param brd HexBoard to do work on. Board position is set to
               the board position as that of the game board. 
        @param game_state Game history up to this position.
        @param color Color to move in this position.
        @param max_time Time in which to return the move. 
        @param score Return score of move here. 
    */
    virtual HexPoint genmove(HexBoard& brd, 
                             const Game& game_state, HexColor color,
                             double max_time, double& score)=0;

};

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // HEXPLAYER_HPP
