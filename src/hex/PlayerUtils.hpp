//----------------------------------------------------------------------------
// $Id: PlayerUtils.hpp 1657 2008-09-15 23:32:09Z broderic $
//----------------------------------------------------------------------------

#ifndef PLAYERUTILS_HPP
#define PLAYERUTILS_HPP

#include "Hex.hpp"
#include "HexBoard.hpp"
#include "HexEval.hpp"

//----------------------------------------------------------------------------

/** Utilities used by UofAPlayers and their engines. */
namespace PlayerUtils
{

    /** Returns true if color wins in this state. This checks
        for solid chains and for winning scs/vcs. */
    bool IsWonGame(const HexBoard& brd, HexColor color);

    /** Returns true if color loses in this state. This checks for
        solid chains and for winning scs/vcs. */
    bool IsLostGame(const HexBoard& brd, HexColor color);

    /** Returns true if this is a winning/losing state for color (as
        defined by IsWonGame() and IsLostGame()), score is set to
        IMMEDIATE_WIN on win, IMMEDIATE_LOSS on a loss, or 0
        otherwise. */
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
        provably ignore. Mustplay must not be empty.  Returned set of
        moves to consider is guaranteed to be non-empty. This assumes
        IsDeterminedState() returns false.

        @see @ref computingmovestoconsider
    */
    bitset_t MovesToConsider(const HexBoard& brd, HexColor color);

    /** Returns the set of moves that need to be considered from a
        losing boardstate; that is, without the moves that we can
        provably ignore.
        
        This is useful in the 1-ply pre-search of MoHex; there the
        root is not a determined state, but it is possible that all
        children returned by MovesToConsider() are actually losing
        (discovered after playing them).  In this case, MoHex should
        return the "best" possible losing move, but we cannot call
        PlayDeterminedState() because the root is not determined.  To
        find the best possible move, MoHex just searches. This method
        is called to determine the mustplay of the losing moves.
        
        Mustplay should be empty.  Returned set of moves to consider
        is guaranteed to be non-empty. This assumes IsLosGame()
        returns true.
    */
    bitset_t MovesToConsiderInLosingState(const HexBoard& brd, HexColor color);

}

//----------------------------------------------------------------------------

#endif // PLAYERUTILS_HPP
