//----------------------------------------------------------------------------
/** @file SwapCheck.cpp
 *
 * @todo WHAT THE HELL IS THIS GARBAGE?!?!?! REMOVE THIS!!
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
			       double max_time, double& score)
{
    if (game_state.AllowSwap()
        && (1 == game_state.History().size())
        && (!FIRST_TO_PLAY == color))
    {
	HexAssert(1 == brd.GetState().NumStones());
	LogInfo() << "Performing swap pre-check...\n";
	
	/** If board has unequal dimensions, we want to traverse the
	    shorter distance. */
	if (brd.Width() != brd.Height()) 
        {
	    if ((brd.Width() > brd.Height() && color == !VERTICAL_COLOR) ||
		(brd.Width() < brd.Height() && color == VERTICAL_COLOR))
            {
                LogInfo() << "Non-square board: " 
                          << "Swapping to obtain shorter side!\n";
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
	    if (1 == brd.Width()) {
		HexAssert(firstMove == HEX_CELL_A1);
		return SWAP_PIECES;
	    } else if (2 == brd.Width()) {
		if (firstMove == HEX_CELL_B1 ||
		    firstMove == HEX_CELL_A2)
		    return SWAP_PIECES;
	    } else if (3 == brd.Width()) {
		if (firstMove == HEX_CELL_B1 ||
		    firstMove == HEX_CELL_C1 ||
		    firstMove == HEX_CELL_B2 ||
		    firstMove == HEX_CELL_A3 ||
		    firstMove == HEX_CELL_B3)
		    return SWAP_PIECES;
	    } else if (4 == brd.Width()) {
		if (firstMove == HEX_CELL_D1 ||
		    firstMove == HEX_CELL_C2 ||
		    firstMove == HEX_CELL_B3 ||
		    firstMove == HEX_CELL_A4)
		    return SWAP_PIECES;
	    } else if (5 == brd.Width()) {
		if (firstMove == HEX_CELL_D1 ||
		    firstMove == HEX_CELL_E1 ||
		    firstMove == HEX_CELL_B2 ||
		    firstMove == HEX_CELL_C2 ||
		    firstMove == HEX_CELL_D2 ||
		    firstMove == HEX_CELL_B3 ||
		    firstMove == HEX_CELL_C3 ||
		    firstMove == HEX_CELL_D3 ||
		    firstMove == HEX_CELL_B4 ||
		    firstMove == HEX_CELL_C4 ||
		    firstMove == HEX_CELL_D4 ||
		    firstMove == HEX_CELL_A5 ||
		    firstMove == HEX_CELL_B5)
		    return SWAP_PIECES;
	    } else if (6 == brd.Width()) {
		if (firstMove == HEX_CELL_C1 ||
		    firstMove == HEX_CELL_D1 ||
		    firstMove == HEX_CELL_E1 ||
		    firstMove == HEX_CELL_F1 ||
		    firstMove == HEX_CELL_B2 ||
		    firstMove == HEX_CELL_C2 ||
		    firstMove == HEX_CELL_D2 ||
		    firstMove == HEX_CELL_E2 ||
		    firstMove == HEX_CELL_B3 ||
		    firstMove == HEX_CELL_C3 ||
		    firstMove == HEX_CELL_D3 ||
		    firstMove == HEX_CELL_E3 ||
		    firstMove == HEX_CELL_B4 ||
		    firstMove == HEX_CELL_C4 ||
		    firstMove == HEX_CELL_D4 ||
		    firstMove == HEX_CELL_E4 ||
		    firstMove == HEX_CELL_B5 ||
		    firstMove == HEX_CELL_C5 ||
		    firstMove == HEX_CELL_D5 ||
		    firstMove == HEX_CELL_E5 ||
		    firstMove == HEX_CELL_A6 ||
		    firstMove == HEX_CELL_B6 ||
		    firstMove == HEX_CELL_C6 ||
		    firstMove == HEX_CELL_D6)
		    return SWAP_PIECES;
	    } else if (7 == brd.Width()) {
		if (firstMove == HEX_CELL_D1 ||
		    firstMove == HEX_CELL_F1 ||
		    firstMove == HEX_CELL_G1 ||
		    firstMove == HEX_CELL_C2 ||
		    firstMove == HEX_CELL_D2 ||
		    firstMove == HEX_CELL_E2 ||
		    firstMove == HEX_CELL_F2 ||
		    firstMove == HEX_CELL_B3 ||
		    firstMove == HEX_CELL_C3 ||
		    firstMove == HEX_CELL_D3 ||
		    firstMove == HEX_CELL_E3 ||
		    firstMove == HEX_CELL_F3 ||
		    firstMove == HEX_CELL_C4 ||
		    firstMove == HEX_CELL_D4 ||
		    firstMove == HEX_CELL_E4 ||
		    firstMove == HEX_CELL_B5 ||
		    firstMove == HEX_CELL_C5 ||
		    firstMove == HEX_CELL_D5 ||
		    firstMove == HEX_CELL_E5 ||
		    firstMove == HEX_CELL_F5 ||
		    firstMove == HEX_CELL_B6 ||
		    firstMove == HEX_CELL_C6 ||
		    firstMove == HEX_CELL_D6 ||
		    firstMove == HEX_CELL_E6 ||
		    firstMove == HEX_CELL_A7 ||
		    firstMove == HEX_CELL_B7 ||
		    firstMove == HEX_CELL_D7)
		    return SWAP_PIECES;
	    } else if (8 == brd.Width()) {
		if (firstMove == HEX_CELL_H1 ||
		    firstMove == HEX_CELL_G2 ||
		    firstMove == HEX_CELL_H2 ||
		    firstMove == HEX_CELL_B3 ||
		    firstMove == HEX_CELL_C3 ||
		    firstMove == HEX_CELL_D3 ||
		    firstMove == HEX_CELL_E3 ||
		    firstMove == HEX_CELL_F3 ||
		    firstMove == HEX_CELL_G3 ||
		    firstMove == HEX_CELL_B4 ||
		    firstMove == HEX_CELL_C4 ||
		    firstMove == HEX_CELL_D4 ||
		    firstMove == HEX_CELL_E4 ||
		    firstMove == HEX_CELL_F4 ||
		    firstMove == HEX_CELL_G4 ||
		    firstMove == HEX_CELL_H4 ||
		    firstMove == HEX_CELL_A5 ||
		    firstMove == HEX_CELL_B5 ||
		    firstMove == HEX_CELL_C5 ||
		    firstMove == HEX_CELL_D5 ||
		    firstMove == HEX_CELL_E5 ||
		    firstMove == HEX_CELL_F5 ||
		    firstMove == HEX_CELL_G5 ||
		    firstMove == HEX_CELL_B6 ||
		    firstMove == HEX_CELL_C6 ||
		    firstMove == HEX_CELL_D6 ||
		    firstMove == HEX_CELL_E6 ||
		    firstMove == HEX_CELL_F6 ||
		    firstMove == HEX_CELL_G6 ||
		    firstMove == HEX_CELL_A7 ||
		    firstMove == HEX_CELL_B7 ||
		    firstMove == HEX_CELL_A8)
		    return SWAP_PIECES;
	    }
	    /** On 9x9 and larger we can only make heuristic-based decisions.
                @note: we specify non-swapped from now on for shorter lists.
            */
	    else if (9 == brd.Width()) {
		/** @todo: choose openings to swap on 9x9. */
	    } else if (10 == brd.Width()) {
		/** @todo: choose openings to swap on 10x10. */
	    } else if (11 == brd.Width()) {
		if (m_player->name() == "wolve") {
		    // Choices based on recent tournament performances
		    if (firstMove != HEX_CELL_A1 &&
			firstMove != HEX_CELL_B1 &&
			firstMove != HEX_CELL_C1 &&
			firstMove != HEX_CELL_D1 &&
			firstMove != HEX_CELL_E1 &&
			firstMove != HEX_CELL_F1 &&
			firstMove != HEX_CELL_G1 &&
			firstMove != HEX_CELL_H1 &&
			firstMove != HEX_CELL_I1 &&
			firstMove != HEX_CELL_J1 &&
			firstMove != HEX_CELL_K1 &&
			firstMove != HEX_CELL_A11 &&
			firstMove != HEX_CELL_B11 &&
			firstMove != HEX_CELL_C11 &&
			firstMove != HEX_CELL_D11 &&
			firstMove != HEX_CELL_E11 &&
			firstMove != HEX_CELL_F11 &&
			firstMove != HEX_CELL_G11 &&
			firstMove != HEX_CELL_H11 &&
			firstMove != HEX_CELL_I11 &&
			firstMove != HEX_CELL_J11 &&
			firstMove != HEX_CELL_K11 &&
			firstMove != HEX_CELL_A2 &&
			firstMove != HEX_CELL_B2 &&
			firstMove != HEX_CELL_K10 &&
			firstMove != HEX_CELL_J10) {
			return SWAP_PIECES;
		    }
		} else if (m_player->name() == "mohex") {
		    // Choices based on mohex-built opening book
		    if (firstMove != HEX_CELL_A1 &&
			firstMove != HEX_CELL_B1 &&
			firstMove != HEX_CELL_C1 &&
			firstMove != HEX_CELL_D1 &&
			firstMove != HEX_CELL_E1 &&
			firstMove != HEX_CELL_F1 &&
			firstMove != HEX_CELL_G1 &&
			firstMove != HEX_CELL_H1 &&
			firstMove != HEX_CELL_I1 &&
			firstMove != HEX_CELL_J1 &&
			firstMove != HEX_CELL_K1 &&
			firstMove != HEX_CELL_A11 &&
			firstMove != HEX_CELL_B11 &&
			firstMove != HEX_CELL_C11 &&
			firstMove != HEX_CELL_D11 &&
			firstMove != HEX_CELL_E11 &&
			firstMove != HEX_CELL_F11 &&
			firstMove != HEX_CELL_G11 &&
			firstMove != HEX_CELL_H11 &&
			firstMove != HEX_CELL_I11 &&
			firstMove != HEX_CELL_J11 &&
			firstMove != HEX_CELL_K11 &&
			firstMove != HEX_CELL_A2 &&
			firstMove != HEX_CELL_B2 &&
			firstMove != HEX_CELL_K10 &&
			firstMove != HEX_CELL_J10 &&
			firstMove != HEX_CELL_A3 &&
			firstMove != HEX_CELL_K9 &&
			firstMove != HEX_CELL_A4 &&
			firstMove != HEX_CELL_K8 &&
			firstMove != HEX_CELL_A5 &&
			firstMove != HEX_CELL_K7 &&
			firstMove != HEX_CELL_A6 &&
			firstMove != HEX_CELL_K6 &&
			firstMove != HEX_CELL_A7 &&
			firstMove != HEX_CELL_K5 &&
			firstMove != HEX_CELL_A8 &&
			firstMove != HEX_CELL_K4 &&
			firstMove != HEX_CELL_A9 &&
			firstMove != HEX_CELL_K3) {
			return SWAP_PIECES;
		    }
		} else {
		    // Philip's guess as to what are losing opening moves.
		    if (firstMove != HEX_CELL_A1 &&
			firstMove != HEX_CELL_A2 &&
			firstMove != HEX_CELL_A3 &&
			firstMove != HEX_CELL_B1 &&
			firstMove != HEX_CELL_B2 &&
			firstMove != HEX_CELL_B11 &&
			firstMove != HEX_CELL_C1 &&
			firstMove != HEX_CELL_C2 &&
			firstMove != HEX_CELL_C11 &&
			firstMove != HEX_CELL_D1 &&
			firstMove != HEX_CELL_D2 &&
			firstMove != HEX_CELL_D11 &&
			firstMove != HEX_CELL_E1 &&
			firstMove != HEX_CELL_E11 &&
			firstMove != HEX_CELL_F1 &&
			firstMove != HEX_CELL_F11 &&
			firstMove != HEX_CELL_G1 &&
			firstMove != HEX_CELL_G11 &&
			firstMove != HEX_CELL_H1 &&
			firstMove != HEX_CELL_H10 &&
			firstMove != HEX_CELL_H11 &&
			firstMove != HEX_CELL_I1 &&
			firstMove != HEX_CELL_I10 &&
			firstMove != HEX_CELL_I11 &&
			firstMove != HEX_CELL_J1 &&
			firstMove != HEX_CELL_J10 &&
			firstMove != HEX_CELL_J11 &&
			firstMove != HEX_CELL_K9 &&
			firstMove != HEX_CELL_K10 &&
			firstMove != HEX_CELL_K11) 
                    {
			return SWAP_PIECES;
		    }
		}
	    }
        }
        LogWarning() << "Opted not to swap.\n";
    }
    return m_player->pre_search(brd, game_state, color, consider,
				max_time, score);
}

//----------------------------------------------------------------------------
