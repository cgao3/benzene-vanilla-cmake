//----------------------------------------------------------------------------
/** @file EndgameUtil.hpp */
//----------------------------------------------------------------------------

#ifndef ENDGAMEUTIL_HPP
#define ENDGAMEUTIL_HPP

#include "Hex.hpp"
#include "HexBoard.hpp"
#include "HexEval.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Utilities on endgames: detecting, playing, etc. */
namespace EndgameUtil
{

    /** Returns true if color wins in this state. This checks
        for solid chains and for winning scs/vcs. */
    bool IsWonGame(const HexBoard& brd, HexColor color, bitset_t& proof);

    bool IsWonGame(const HexBoard& brd, HexColor color);

    /** Returns true if color loses in this state. This checks for
        solid chains and for winning scs/vcs. */
    bool IsLostGame(const HexBoard& brd, HexColor color, bitset_t& proof);

    bool IsLostGame(const HexBoard& brd, HexColor color);

    /** Returns true if this is a winning/losing state for color (as
        defined by IsWonGame() and IsLostGame()), score is set to
        IMMEDIATE_WIN on win, IMMEDIATE_LOSS on a loss, or 0
        otherwise. */
    bool IsDeterminedState(const HexBoard& brd, HexColor color,
                           HexEval& score, bitset_t& proof);
    
    bool IsDeterminedState(const HexBoard& brd, HexColor color,
                           HexEval& score);

    bool IsDeterminedState(const HexBoard& brd, HexColor color);

    /** Plays the "best" move in a determined state.  Assumes
        IsDetermined() returns true, but requires that
        brd.isGameOver() is false. That is, is, it cannot play a move
        if a solid chain exists on this board.

        @see @ref playingdeterminedstates
    */
    HexPoint PlayDeterminedState(const HexBoard& brd, HexColor color);

    /** Returns the set of moves that need to be considered from the
        given boardstate; that is, without the moves that we can
        provably ignore. Returned set of moves to consider is
        guaranteed to be non-empty. This assumes IsDeterminedState()
        returns false.

        @todo MOVE THIS OUT OF HERE!

        @see @ref computingmovestoconsider
    */
    bitset_t MovesToConsider(const HexBoard& brd, HexColor color);
}

//----------------------------------------------------------------------------

inline bool EndgameUtil::IsDeterminedState(const HexBoard& brd, 
                                           HexColor color, HexEval& eval)
                                          
{
    bitset_t proof;
    return IsDeterminedState(brd, color, eval, proof);
}

inline bool EndgameUtil::IsDeterminedState(const HexBoard& brd, HexColor color)
{
    HexEval eval;
    bitset_t proof;
    return IsDeterminedState(brd, color, eval, proof);
}

inline bool EndgameUtil::IsLostGame(const HexBoard& brd, HexColor color)
{
    bitset_t proof;
    return IsLostGame(brd, color, proof);
}

inline bool EndgameUtil::IsWonGame(const HexBoard& brd, HexColor color)
{
    bitset_t proof;
    return IsWonGame(brd, color, proof);
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // ENDGAMEUTIL_HPP
