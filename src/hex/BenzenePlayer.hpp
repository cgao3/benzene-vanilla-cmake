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
    */
    HexPoint GenMove(const HexState& state, const Game& game, 
                     HexBoard& brd, double maxTime, double& score);

    /** Search states with only a single move? */
    bool SearchSingleton() const;

    /** See SetSearchSingleton() */
    void SetSearchSingleton(bool flag);
    
protected:
    bool m_fillinCausedWin;

    HexColor m_fillinWinner;
    
    bool FillinCausedWin() const;

    /** Generates a move in the given gamestate. Derived players
        must implement this method. Score can be stored in score.
        @param state Position and color to play.
        @param game Game history up to this point.
        @param brd Board to use for work.
        @param consider Moves to consider in this state. 
        @param maxTime Max time available for move.
        @param score Score of the move to play.
        @return The move to play.
    */
    virtual HexPoint Search(const HexState& state, const Game& game,
                            HexBoard& brd, const bitset_t& consider,
                            double maxTime, double& score) = 0;
    
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

inline bool BenzenePlayer::FillinCausedWin() const
{
    return m_fillinCausedWin;
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // BENZENEPLAYER_HPP
