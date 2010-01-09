//----------------------------------------------------------------------------
/** @file BenzenePlayer.hpp
 */
//----------------------------------------------------------------------------

#ifndef BENZENEPLAYER_HPP
#define BENZENEPLAYER_HPP

#include "HexBoard.hpp"
#include "HexEval.hpp"
#include "HexPlayer.hpp"
#include "ICEngine.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Abstract base class for all players using the Benzene systems. */
class BenzenePlayer: public HexPlayer
{
public:
    explicit BenzenePlayer();

    virtual ~BenzenePlayer();

    /** Generates a move from this board position. If the game is
        already over (somebody has won), returns RESIGN.

        Derived Benzene players that use different search algorithms
        should not extend this method, but the protected virtual
        method Search() below.

        If state is terminal (game over, vc/fill-in win/loss),
        returns "appropriate" move. Otherwise, calls Search().

        @param brd HexBoard to do work on. Board position is set to
               the board position as that of the game board. 
        @param game_state Game history up to this position.
        @param color Color to move in this position.
        @param maxTime Time in minutes remaining in game.
        @param score Score of returned move. 
    */
    HexPoint GenMove(HexBoard& brd, const Game& game_state, HexColor color,
                     double maxTime, double& score);

    /** Search states with only a single move? */
    bool SearchSingleton() const;

    /** See SetSearchSingleton() */
    void SetSearchSingleton(bool flag);
    

protected:
    /** Generates a move in the given gamestate. Derived players
        must implement this method. Score can be stored in score.
        @param brd
        @param game_state
        @param color
        @param consider Moves to consider in this state. 
        @param max_time
        @param score
        @return The move to play.
    */
    virtual HexPoint Search(HexBoard& brd, const Game& game_state,
			    HexColor color, const bitset_t& consider,
                            double max_time, double& score) = 0;
    
private:
    bool m_search_singleton;
          
    HexPoint InitSearch(HexBoard& brd, HexColor color, 
                        bitset_t& consider, double& score);

    HexPoint CheckEndgame(HexBoard& brd, HexColor color, 
                          bitset_t& consider, double& score);
};

inline bool BenzenePlayer::SearchSingleton() const
{
    return m_search_singleton;
}

inline void BenzenePlayer::SetSearchSingleton(bool flag)
{
    m_search_singleton = flag;
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // BENZENEPLAYER_HPP
