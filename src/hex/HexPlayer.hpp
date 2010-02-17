//----------------------------------------------------------------------------
/** @file HexPlayer.hpp
 */
//----------------------------------------------------------------------------

#ifndef HEXPLAYER_HPP
#define HEXPLAYER_HPP

#include "Game.hpp"
#include "HexBoard.hpp"
#include "HexState.hpp"

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
    virtual std::string Name() const = 0;

    /** Generates a move from this game position. If maxTime is
        negative, then there is no timelimit. Otherwise, maxTime is
        the number of seconds the player has to generate this move. If
        game is already over, player should return RESIGN. The player
        can store a score for the move it generated in score.

        @param state Position and color to play.
        @param game Game history up to this position.
        @param brd HexBoard to do work on.
        @param maxTime Time in which to return the move. 
        @param score Return score of move here. 
    */
    virtual HexPoint GenMove(const HexState& state, const Game& game, 
                             HexBoard& brd, double maxTime, 
                             double& score) = 0;
};

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // HEXPLAYER_HPP
