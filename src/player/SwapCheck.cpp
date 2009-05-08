//----------------------------------------------------------------------------
/** @file SwapCheck.cpp
 */
//----------------------------------------------------------------------------

#include "BoardUtils.hpp"
#include "SwapCheck.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

SwapCheck::SwapCheck(BenzenePlayer* player)
    : BenzenePlayerFunctionality(player),
      m_player(player)
{
}

SwapCheck::~SwapCheck()
{
}

HexPoint SwapCheck::pre_search(HexBoard& brd, const Game& game_state,
			       HexColor color, bitset_t& consider,
			       double time_remaining, double& score)
{
    if (game_state.AllowSwap()
        && (1 == game_state.History().size())
        && (!FIRST_TO_PLAY == color))
    {
	HexAssert(1 == brd.numStones());
	LogInfo() << "Performing swap pre-check..." << '\n';
	
	/** If board has unequal dimensions, we want to traverse the
	    shorter distance. */
	if (brd.width() != brd.height()) 
        {
	    if ((brd.width() > brd.height() && color == !VERTICAL_COLOR) ||
		(brd.width() < brd.height() && color == VERTICAL_COLOR))
            {
                LogInfo() << "Non-square board: " 
                         << "Swapping to obtain shorter side!"
                         << '\n';
		return SWAP_PIECES;
            }
	}
        else 
        {
	    // Board width equals board height.
	    HexPoint firstMove = game_state.History().back().point();
	    /** Swap decisions below assume VERTICAL_COLOR was FIRST_TO_PLAY,
		so we mirror the first move if this is not the case (i.e. to
		consider an equivalent decision). */
	    if (color == VERTICAL_COLOR)
		firstMove = BoardUtils::Mirror(brd.Const(), firstMove);
	    
	    // We can make optimal decisions up to 8x8.
	    if (1 == brd.width()) {
		HexAssert(firstMove == HexPointUtil::fromString("a1"));
		return SWAP_PIECES;
	    } else if (2 == brd.width()) {
		if (firstMove == HexPointUtil::fromString("b1") ||
		    firstMove == HexPointUtil::fromString("a2"))
		    return SWAP_PIECES;
	    } else if (3 == brd.width()) {
		if (firstMove == HexPointUtil::fromString("b1") ||
		    firstMove == HexPointUtil::fromString("c1") ||
		    firstMove == HexPointUtil::fromString("b2") ||
		    firstMove == HexPointUtil::fromString("a3") ||
		    firstMove == HexPointUtil::fromString("b3"))
		    return SWAP_PIECES;
	    } else if (4 == brd.width()) {
		if (firstMove == HexPointUtil::fromString("d1") ||
		    firstMove == HexPointUtil::fromString("c2") ||
		    firstMove == HexPointUtil::fromString("b3") ||
		    firstMove == HexPointUtil::fromString("a4"))
		    return SWAP_PIECES;
	    } else if (5 == brd.width()) {
		if (firstMove == HexPointUtil::fromString("d1") ||
		    firstMove == HexPointUtil::fromString("e1") ||
		    firstMove == HexPointUtil::fromString("b2") ||
		    firstMove == HexPointUtil::fromString("c2") ||
		    firstMove == HexPointUtil::fromString("d2") ||
		    firstMove == HexPointUtil::fromString("b3") ||
		    firstMove == HexPointUtil::fromString("c3") ||
		    firstMove == HexPointUtil::fromString("d3") ||
		    firstMove == HexPointUtil::fromString("b4") ||
		    firstMove == HexPointUtil::fromString("c4") ||
		    firstMove == HexPointUtil::fromString("d4") ||
		    firstMove == HexPointUtil::fromString("a5") ||
		    firstMove == HexPointUtil::fromString("b5"))
		    return SWAP_PIECES;
	    } else if (6 == brd.width()) {
		if (firstMove == HexPointUtil::fromString("c1") ||
		    firstMove == HexPointUtil::fromString("d1") ||
		    firstMove == HexPointUtil::fromString("e1") ||
		    firstMove == HexPointUtil::fromString("f1") ||
		    firstMove == HexPointUtil::fromString("b2") ||
		    firstMove == HexPointUtil::fromString("c2") ||
		    firstMove == HexPointUtil::fromString("d2") ||
		    firstMove == HexPointUtil::fromString("e2") ||
		    firstMove == HexPointUtil::fromString("b3") ||
		    firstMove == HexPointUtil::fromString("c3") ||
		    firstMove == HexPointUtil::fromString("d3") ||
		    firstMove == HexPointUtil::fromString("e3") ||
		    firstMove == HexPointUtil::fromString("b4") ||
		    firstMove == HexPointUtil::fromString("c4") ||
		    firstMove == HexPointUtil::fromString("d4") ||
		    firstMove == HexPointUtil::fromString("e4") ||
		    firstMove == HexPointUtil::fromString("b5") ||
		    firstMove == HexPointUtil::fromString("c5") ||
		    firstMove == HexPointUtil::fromString("d5") ||
		    firstMove == HexPointUtil::fromString("e5") ||
		    firstMove == HexPointUtil::fromString("a6") ||
		    firstMove == HexPointUtil::fromString("b6") ||
		    firstMove == HexPointUtil::fromString("c6") ||
		    firstMove == HexPointUtil::fromString("d6"))
		    return SWAP_PIECES;
	    } else if (7 == brd.width()) {
		if (firstMove == HexPointUtil::fromString("d1") ||
		    firstMove == HexPointUtil::fromString("f1") ||
		    firstMove == HexPointUtil::fromString("g1") ||
		    firstMove == HexPointUtil::fromString("c2") ||
		    firstMove == HexPointUtil::fromString("d2") ||
		    firstMove == HexPointUtil::fromString("e2") ||
		    firstMove == HexPointUtil::fromString("f2") ||
		    firstMove == HexPointUtil::fromString("b3") ||
		    firstMove == HexPointUtil::fromString("c3") ||
		    firstMove == HexPointUtil::fromString("d3") ||
		    firstMove == HexPointUtil::fromString("e3") ||
		    firstMove == HexPointUtil::fromString("f3") ||
		    firstMove == HexPointUtil::fromString("c4") ||
		    firstMove == HexPointUtil::fromString("d4") ||
		    firstMove == HexPointUtil::fromString("e4") ||
		    firstMove == HexPointUtil::fromString("b5") ||
		    firstMove == HexPointUtil::fromString("c5") ||
		    firstMove == HexPointUtil::fromString("d5") ||
		    firstMove == HexPointUtil::fromString("e5") ||
		    firstMove == HexPointUtil::fromString("f5") ||
		    firstMove == HexPointUtil::fromString("b6") ||
		    firstMove == HexPointUtil::fromString("c6") ||
		    firstMove == HexPointUtil::fromString("d6") ||
		    firstMove == HexPointUtil::fromString("e6") ||
		    firstMove == HexPointUtil::fromString("a7") ||
		    firstMove == HexPointUtil::fromString("b7") ||
		    firstMove == HexPointUtil::fromString("d7"))
		    return SWAP_PIECES;
	    } else if (8 == brd.width()) {
		if (firstMove == HexPointUtil::fromString("h1") ||
		    firstMove == HexPointUtil::fromString("g2") ||
		    firstMove == HexPointUtil::fromString("h2") ||
		    firstMove == HexPointUtil::fromString("b3") ||
		    firstMove == HexPointUtil::fromString("c3") ||
		    firstMove == HexPointUtil::fromString("d3") ||
		    firstMove == HexPointUtil::fromString("e3") ||
		    firstMove == HexPointUtil::fromString("f3") ||
		    firstMove == HexPointUtil::fromString("g3") ||
		    firstMove == HexPointUtil::fromString("b4") ||
		    firstMove == HexPointUtil::fromString("c4") ||
		    firstMove == HexPointUtil::fromString("d4") ||
		    firstMove == HexPointUtil::fromString("e4") ||
		    firstMove == HexPointUtil::fromString("f4") ||
		    firstMove == HexPointUtil::fromString("g4") ||
		    firstMove == HexPointUtil::fromString("h4") ||
		    firstMove == HexPointUtil::fromString("a5") ||
		    firstMove == HexPointUtil::fromString("b5") ||
		    firstMove == HexPointUtil::fromString("c5") ||
		    firstMove == HexPointUtil::fromString("d5") ||
		    firstMove == HexPointUtil::fromString("e5") ||
		    firstMove == HexPointUtil::fromString("f5") ||
		    firstMove == HexPointUtil::fromString("g5") ||
		    firstMove == HexPointUtil::fromString("b6") ||
		    firstMove == HexPointUtil::fromString("c6") ||
		    firstMove == HexPointUtil::fromString("d6") ||
		    firstMove == HexPointUtil::fromString("e6") ||
		    firstMove == HexPointUtil::fromString("f6") ||
		    firstMove == HexPointUtil::fromString("g6") ||
		    firstMove == HexPointUtil::fromString("a7") ||
		    firstMove == HexPointUtil::fromString("b7") ||
		    firstMove == HexPointUtil::fromString("a8"))
		    return SWAP_PIECES;
	    }
	    /** On 9x9 and larger we can only make heuristic-based decisions.
                @note: we specify non-swapped from now on for shorter lists.
            */
	    else if (9 == brd.width()) {
		/** @todo: choose openings to swap on 9x9. */
	    } else if (10 == brd.width()) {
		/** @todo: choose openings to swap on 10x10. */
	    } else if (11 == brd.width()) {
		if (m_player->name() == "wolve") {
		    // Choices based on recent tournament performances
		    if (firstMove != HexPointUtil::fromString("a1") &&
			firstMove != HexPointUtil::fromString("b1") &&
			firstMove != HexPointUtil::fromString("c1") &&
			firstMove != HexPointUtil::fromString("d1") &&
			firstMove != HexPointUtil::fromString("e1") &&
			firstMove != HexPointUtil::fromString("f1") &&
			firstMove != HexPointUtil::fromString("g1") &&
			firstMove != HexPointUtil::fromString("h1") &&
			firstMove != HexPointUtil::fromString("i1") &&
			firstMove != HexPointUtil::fromString("j1") &&
			firstMove != HexPointUtil::fromString("k1") &&
			firstMove != HexPointUtil::fromString("a11") &&
			firstMove != HexPointUtil::fromString("b11") &&
			firstMove != HexPointUtil::fromString("c11") &&
			firstMove != HexPointUtil::fromString("d11") &&
			firstMove != HexPointUtil::fromString("e11") &&
			firstMove != HexPointUtil::fromString("f11") &&
			firstMove != HexPointUtil::fromString("g11") &&
			firstMove != HexPointUtil::fromString("h11") &&
			firstMove != HexPointUtil::fromString("i11") &&
			firstMove != HexPointUtil::fromString("j11") &&
			firstMove != HexPointUtil::fromString("k11") &&
			firstMove != HexPointUtil::fromString("a2") &&
			firstMove != HexPointUtil::fromString("b2") &&
			firstMove != HexPointUtil::fromString("k10") &&
			firstMove != HexPointUtil::fromString("j10")) {
			return SWAP_PIECES;
		    }
		} else if (m_player->name() == "mohex") {
		    // Choices based on mohex-built opening book
		    if (firstMove != HexPointUtil::fromString("a1") &&
			firstMove != HexPointUtil::fromString("b1") &&
			firstMove != HexPointUtil::fromString("c1") &&
			firstMove != HexPointUtil::fromString("d1") &&
			firstMove != HexPointUtil::fromString("e1") &&
			firstMove != HexPointUtil::fromString("f1") &&
			firstMove != HexPointUtil::fromString("g1") &&
			firstMove != HexPointUtil::fromString("h1") &&
			firstMove != HexPointUtil::fromString("i1") &&
			firstMove != HexPointUtil::fromString("j1") &&
			firstMove != HexPointUtil::fromString("k1") &&
			firstMove != HexPointUtil::fromString("a11") &&
			firstMove != HexPointUtil::fromString("b11") &&
			firstMove != HexPointUtil::fromString("c11") &&
			firstMove != HexPointUtil::fromString("d11") &&
			firstMove != HexPointUtil::fromString("e11") &&
			firstMove != HexPointUtil::fromString("f11") &&
			firstMove != HexPointUtil::fromString("g11") &&
			firstMove != HexPointUtil::fromString("h11") &&
			firstMove != HexPointUtil::fromString("i11") &&
			firstMove != HexPointUtil::fromString("j11") &&
			firstMove != HexPointUtil::fromString("k11") &&
			firstMove != HexPointUtil::fromString("a2") &&
			firstMove != HexPointUtil::fromString("b2") &&
			firstMove != HexPointUtil::fromString("k10") &&
			firstMove != HexPointUtil::fromString("j10") &&
			firstMove != HexPointUtil::fromString("a3") &&
			firstMove != HexPointUtil::fromString("k9") &&
			firstMove != HexPointUtil::fromString("a4") &&
			firstMove != HexPointUtil::fromString("k8") &&
			firstMove != HexPointUtil::fromString("a5") &&
			firstMove != HexPointUtil::fromString("k7") &&
			firstMove != HexPointUtil::fromString("a6") &&
			firstMove != HexPointUtil::fromString("k6") &&
			firstMove != HexPointUtil::fromString("a7") &&
			firstMove != HexPointUtil::fromString("k5") &&
			firstMove != HexPointUtil::fromString("a8") &&
			firstMove != HexPointUtil::fromString("k4") &&
			firstMove != HexPointUtil::fromString("a9") &&
			firstMove != HexPointUtil::fromString("k3")) {
			return SWAP_PIECES;
		    }
		} else {
		    // Philip's guess as to what are losing opening moves.
		    if (firstMove != HexPointUtil::fromString("a1") &&
			firstMove != HexPointUtil::fromString("a2") &&
			firstMove != HexPointUtil::fromString("a3") &&
			firstMove != HexPointUtil::fromString("b1") &&
			firstMove != HexPointUtil::fromString("b2") &&
			firstMove != HexPointUtil::fromString("b11") &&
			firstMove != HexPointUtil::fromString("c1") &&
			firstMove != HexPointUtil::fromString("c2") &&
			firstMove != HexPointUtil::fromString("c11") &&
			firstMove != HexPointUtil::fromString("d1") &&
			firstMove != HexPointUtil::fromString("d2") &&
			firstMove != HexPointUtil::fromString("d11") &&
			firstMove != HexPointUtil::fromString("e1") &&
			firstMove != HexPointUtil::fromString("e11") &&
			firstMove != HexPointUtil::fromString("f1") &&
			firstMove != HexPointUtil::fromString("f11") &&
			firstMove != HexPointUtil::fromString("g1") &&
			firstMove != HexPointUtil::fromString("g11") &&
			firstMove != HexPointUtil::fromString("h1") &&
			firstMove != HexPointUtil::fromString("h10") &&
			firstMove != HexPointUtil::fromString("h11") &&
			firstMove != HexPointUtil::fromString("i1") &&
			firstMove != HexPointUtil::fromString("i10") &&
			firstMove != HexPointUtil::fromString("i11") &&
			firstMove != HexPointUtil::fromString("j1") &&
			firstMove != HexPointUtil::fromString("j10") &&
			firstMove != HexPointUtil::fromString("j11") &&
			firstMove != HexPointUtil::fromString("k9") &&
			firstMove != HexPointUtil::fromString("k10") &&
			firstMove != HexPointUtil::fromString("k11")) {
			return SWAP_PIECES;
		    }
		}
	    }
	}
	
	LogWarning() << "Opted not to swap." << '\n';
    }
    
    return m_player->pre_search(brd, game_state, color, consider,
				time_remaining, score);
}

//----------------------------------------------------------------------------
