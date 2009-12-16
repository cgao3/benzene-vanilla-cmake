//----------------------------------------------------------------------------
/** @file PlayerUtils.cpp
 */
//----------------------------------------------------------------------------

#include "PlayerUtils.hpp"
#include "BitsetIterator.hpp"
#include "VCSet.hpp"
#include "VCUtils.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

/** @page playingdeterminedstates Playing in Determined States
    
    A determined state is defined as a state were one player has
    a winning semi/full connection.

    In a winning state, returns key of smallest semi connection,
    if one exists.  If no semi connection, plays move that overlaps
    the maximum number of full connections.

    In a losing state, returns move overlapping the most SCs (instead
    of VCs) since any winning SC still remaining on our opponent's
    next turn will allow them to win. Thus, we want to eliminate those
    winning SCs that are shortest/easiest to find. It is also possible
    that our opponent has winning VCs and yet no winning SCs. In this
    case, we just perform the overlap with the VCs.

    @bug It is possible our opponent has winning VCs that are not
    derived from the winning SCs in our list. Thus, we may want to
    consider overlapping the winning VCs as well.
*/

/** @page computingmovestoconsider Computing the set of moves to consider

    The set of moves to consider is defined as the mustplay minus
    as many inferior cells as possible.  

    @note We cannot remove all inferior cells since playing our
    own captured can be a winning move (if HexBoard is handling
    endgames and removed the winning captured stones). Thus we
    always ensure there is at least one move to play.
*/

//----------------------------------------------------------------------------

/** Local functions. */
namespace {

/** Priority is given to eliminating the most easily-answered
    moves first (i.e. dead cells require no answer, answering
    vulnerable plays only requires knowledge of local adjacencies,
    etc.) */
void TightenMoveBitset(bitset_t& moveBitset, const InferiorCells& inf)
{
    BitsetUtil::SubtractIfLeavesAny(moveBitset, inf.Dead());
    BitsetUtil::SubtractIfLeavesAny(moveBitset, inf.Vulnerable());
    BitsetUtil::SubtractIfLeavesAny(moveBitset, inf.Captured(BLACK));
    BitsetUtil::SubtractIfLeavesAny(moveBitset, inf.Captured(WHITE));
    BitsetUtil::SubtractIfLeavesAny(moveBitset, inf.Reversible());
    BitsetUtil::SubtractIfLeavesAny(moveBitset, inf.Dominated());
    HexAssert(moveBitset.any());
}
    
/** Intersects as many of the smallest connections as possible. Then,
    subject to that restriction, tries to be a non-inferior move (using
    the member variables), and then to overlap as many other connections
    as possible. */
HexPoint MostOverlappingMove(const std::vector<VC>& VClist,
                             const InferiorCells& inf)
{
    bitset_t intersectSmallest;
    intersectSmallest.flip();
    
    // compute intersection of smallest until next one makes it empty
    std::vector<VC>::const_iterator it;
    for (it = VClist.begin(); it != VClist.end(); ++it) 
    {
        bitset_t carrier = it->carrier();
        if ((carrier & intersectSmallest).none())
	    break;
	intersectSmallest &= carrier;
    }
    LogFine() << "Intersection of smallest set is:\n"
	      << HexPointUtil::ToString(intersectSmallest) << '\n';
    
    // remove as many inferior moves as possible from this intersection
    TightenMoveBitset(intersectSmallest, inf);
    
    LogFine() << "After elimination of inferior cells:\n"
	      << HexPointUtil::ToString(intersectSmallest) << '\n';
    
    // determine which of the remaining cells performs best with
    // regards to other connections
    int numHits[BITSETSIZE];
    memset(numHits, 0, sizeof(numHits));
    for (it = VClist.begin(); it != VClist.end(); ++it) 
    {
	bitset_t carrier = it->carrier();
	for (int i = 0; i < BITSETSIZE; i++) 
	    if (intersectSmallest.test(i) && carrier.test(i))
		numHits[i]++;
    }
    
    int curBestMove = -1;
    int curMostHits = 0;
    for (int i = 0; i < BITSETSIZE; i++) 
    {
	if (numHits[i] > curMostHits) 
        {
	    HexAssert(intersectSmallest.test(i));
	    curMostHits = numHits[i];
	    curBestMove = i;
	}
    }
    
    if (curMostHits == 0)
        LogWarning() << "curMostHits == 0!!\n";
    HexAssert(curMostHits > 0);
    return (HexPoint)curBestMove;
}
    
/** Returns best winning move. */
HexPoint PlayWonGame(const HexBoard& brd, HexColor color)
{
    HexAssert(PlayerUtils::IsWonGame(brd, color));

    // If we have a winning SC, then play in the key of the smallest one
    VC winningVC;
    if (brd.Cons(color).SmallestVC(HexPointUtil::colorEdge1(color), 
                                   HexPointUtil::colorEdge2(color), 
                                   VC::SEMI, winningVC)) 
    {
        LogInfo() << "Winning SC.\n";
        return winningVC.key();
    }
    
    // If instead we have a winning VC, then play best move in its carrier set
    if (brd.Cons(color).Exists(HexPointUtil::colorEdge1(color),
                               HexPointUtil::colorEdge2(color),
                               VC::FULL))
    {
        LogFine() << "Winning VC.\n";
        std::vector<VC> vcs;
        brd.Cons(color).VCs(HexPointUtil::colorEdge1(color),
                            HexPointUtil::colorEdge2(color),
                            VC::FULL, vcs);
	return MostOverlappingMove(vcs, brd.GetInferiorCells());
    }
    // Should never get here!
    HexAssert(false);
    return INVALID_POINT;
}

/** Returns most blocking (ie, the "best") losing move. */
HexPoint PlayLostGame(const HexBoard& brd, HexColor color)
{
    HexAssert(PlayerUtils::IsLostGame(brd, color));
    
    // determine if color's opponent has guaranteed win
    HexColor other = !color;
    HexPoint otheredge1 = HexPointUtil::colorEdge1(other);
    HexPoint otheredge2 = HexPointUtil::colorEdge2(other);
    
    LogInfo() << "Opponent has won; playing most blocking move.\n";
    
    /** Uses semi-connections. 
        @see @ref playingdeterminedstates 
    */
    bool connected = brd.Cons(other).Exists(otheredge1, otheredge2, VC::SEMI);
    std::vector<VC> vcs;
    brd.Cons(other).VCs(otheredge1, otheredge2, 
                        ((connected) ? VC::SEMI : VC::FULL), vcs);

    return MostOverlappingMove(vcs, brd.GetInferiorCells());
}

} // anonymous namespace

//----------------------------------------------------------------------------

bool PlayerUtils::IsWonGame(const HexBoard& brd, HexColor color)
{
    if (brd.GetGroups().IsGameOver())
        return (brd.GetGroups().GetWinner() == color);
    if (brd.Cons(color).Exists(HexPointUtil::colorEdge1(color), 
                               HexPointUtil::colorEdge2(color), 
                               VC::SEMI)) 
    {
	return true;
    }
    if (brd.Cons(color).Exists(HexPointUtil::colorEdge1(color),
                               HexPointUtil::colorEdge2(color),
                               VC::FULL))
    {
	return true;
    }
    return false;
}

bool PlayerUtils::IsLostGame(const HexBoard& brd, HexColor color)
{
    if (brd.GetGroups().IsGameOver())
        return (brd.GetGroups().GetWinner() != color);
    if (brd.Cons(!color).Exists(HexPointUtil::colorEdge1(!color),
                                HexPointUtil::colorEdge2(!color),
                                VC::FULL))
    {
	return true;
    }
    bitset_t mustplay = VCUtils::GetMustplay(brd, color);
    const InferiorCells& inf = brd.GetInferiorCells();
    bitset_t remaining = mustplay - inf.Vulnerable()
                                  - inf.Reversible()
                                  - inf.Dominated();
    return remaining.none();
}

bool PlayerUtils::IsDeterminedState(const HexBoard& brd, HexColor color, 
                                    HexEval& score)
{
    score = 0;
    if (IsWonGame(brd, color)) 
    {
        score = IMMEDIATE_WIN;
        return true;
    }
    if (IsLostGame(brd, color)) 
    {
        score = IMMEDIATE_LOSS;
        return true;
    }
    return false;
}

bool PlayerUtils::IsDeterminedState(const HexBoard& brd, HexColor color)
{
    HexEval eval;
    return IsDeterminedState(brd, color, eval);
}

HexPoint PlayerUtils::PlayDeterminedState(const HexBoard& brd, HexColor color)
                                          
{
    HexAssert(HexColorUtil::isBlackWhite(color));
    HexAssert(IsDeterminedState(brd, color));
    HexAssert(!brd.GetGroups().IsGameOver());

    if (IsWonGame(brd, color))
        return PlayWonGame(brd, color);

    HexAssert(IsLostGame(brd, color));
    return PlayLostGame(brd, color);
}

bitset_t PlayerUtils::MovesToConsider(const HexBoard& brd, HexColor color)
{
    HexAssert(HexColorUtil::isBlackWhite(color));
    HexAssert(!IsDeterminedState(brd, color));
    
    bitset_t consider = VCUtils::GetMustplay(brd, color);
    HexAssert(consider.any());
    
    const InferiorCells& inf = brd.GetInferiorCells();
    consider = consider - inf.Vulnerable()
                        - inf.Reversible()
                        - inf.Dominated();
    HexAssert(consider.any());

    if (consider.count() == 1) 
        LogFine() << "Mustplay is singleton.\n";

    LogFine() << "Moves to consider for " << color << ":" 
              << brd.Write(consider) << '\n';
    return consider;
}

//----------------------------------------------------------------------------
