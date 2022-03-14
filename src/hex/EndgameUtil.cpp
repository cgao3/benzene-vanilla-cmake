//----------------------------------------------------------------------------
/** @file EndgameUtil.cpp */
//----------------------------------------------------------------------------

#include "EndgameUtil.hpp"
#include "BitsetIterator.hpp"
#include "BoardUtil.hpp"
#include "VCS.hpp"
#include "VCUtil.hpp"

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

    The set of moves to consider is defined as the mustplay minus the
    inferior cells minus cells that create states that are mirrors of
    themselves (these are losing via the strategy stealing argument)
    minus any cells that are rotations of other cells (if the state is
    a rotation of itself). This set can never be empty, because
    IsLostGame() detects such states and reports them as losing (these
    states will be handled by PlayDeterminedState()).
*/

//----------------------------------------------------------------------------

/** Local functions. */
namespace {

bitset_t ComputeLossesViaStrategyStealingArgument(const StoneBoard& brd,
                                                  HexColor color)
{
    bitset_t ret;
    if ((brd.Width() == brd.Height())
        && (brd.GetPlayed(!color).count() - brd.GetPlayed(color).count() == 1))
    {
        bitset_t mirror1 
            = BoardUtil::Mirror(brd.Const(), brd.GetPlayed(!color))
            - brd.GetPlayed(color);
        if (mirror1.count() == 1)
            ret |= mirror1;
        bitset_t mirror2 =
            BoardUtil::Mirror(brd.Const(), BoardUtil::Rotate(brd.Const(), 
                                                  brd.GetPlayed(!color)))
            - brd.GetPlayed(color);
        if (mirror2.count() == 1)
            ret |= mirror2;
        ret &= brd.GetEmpty();
    }
    return ret;
}

bitset_t RemoveRotations(const StoneBoard& brd, const bitset_t& consider)
{
    bitset_t ret;
    for (BitsetIterator it(consider); it; ++it)
    {
        HexPoint rotated = BoardUtil::Rotate(brd.Const(), *it);
        if (!ret.test(rotated))
            ret.set(*it);
    }
    return ret;
}

bitset_t ComputeConsiderSet(const HexBoard& brd, HexColor color)
{
    bitset_t consider = VCUtil::GetMustplay(brd, color);
    const InferiorCells& inf = brd.GetInferiorCells();
    consider = consider - inf.All()
          - ComputeLossesViaStrategyStealingArgument(brd.GetPosition(), color);
    if (brd.GetPosition().IsSelfRotation())
        consider = RemoveRotations(brd.GetPosition(), consider);
    return consider;
}

bitset_t StonesInProof(const HexBoard& brd, HexColor color)
{
    return brd.GetPosition().GetColor(color) 
        | brd.GetInferiorCells().Fillin(color);
}

/** Priority is given to eliminating the most easily-answered
    moves first (i.e. dead cells require no answer, answering
    reversible plays only requires knowledge of local adjacencies,
    etc.) */
void TightenMoveBitset(bitset_t& moveBitset, const InferiorCells& inf)
{
    BitsetUtil::SubtractIfLeavesAny(moveBitset, inf.Fillin(ARBITRARY_COLOR));
    BitsetUtil::SubtractIfLeavesAny(moveBitset, inf.Fillin(!ARBITRARY_COLOR));
    BitsetUtil::SubtractIfLeavesAny(moveBitset, inf.Vulnerable());
    BitsetUtil::SubtractIfLeavesAny(moveBitset, inf.SReversible());
    BitsetUtil::SubtractIfLeavesAny(moveBitset, inf.Inferior());
    BenzeneAssert(moveBitset.any());
}
    
/** Intersects as many of the smallest connections as possible. Then,
    subject to that restriction, tries to be a non-inferior move (using
    the member variables), and then to overlap as many other connections
    as possible. */
HexPoint MostOverlappingMove(const StoneBoard& brd,
			     const CarrierList& carriers,
                             const InferiorCells& inf)
{
    bitset_t intersectSmallest = brd.Const().GetLocations() - brd.GetPlayed();

    // compute intersection of smallest until next one makes it empty
    for (CarrierList::Iterator it(carriers); it; ++it)
    {
        if ((it.Carrier() & intersectSmallest).none())
	    break;
	intersectSmallest &= it.Carrier();
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
    for (CarrierList::Iterator it(carriers); it; ++it)
    {
	for (int i = 0; i < BITSETSIZE; i++) 
	    if (intersectSmallest.test(i) && it.Carrier().test(i))
		numHits[i]++;
    }

    BenzeneAssert(intersectSmallest.any());
    int curBestMove = BitsetUtil::FirstSetBit(intersectSmallest);
    int curMostHits = 0;
    for (int i = 0; i < BITSETSIZE; i++) 
    {
	if (numHits[i] > curMostHits) 
        {
	    BenzeneAssert(intersectSmallest.test(i));
	    curMostHits = numHits[i];
	    curBestMove = i;
	}
    }
    
    return (HexPoint)curBestMove;
}
    
/** Returns best winning move. */
HexPoint PlayWonGame(const HexBoard& brd, HexColor color)
{
    BenzeneAssert(EndgameUtil::IsWonGame(brd, color));

    // If we have a winning SC, then play in the key of the smallest one
    HexPoint semi_key = brd.Cons(color).SmallestSemiKey();
    if (semi_key != INVALID_POINT)
    {
        LogInfo() << "Winning SC.\n";
        return semi_key;
    }
    
    // If instead we have a winning VC, then play best move in its carrier set
    if (brd.Cons(color).FullExists())
    {
        LogFine() << "Winning VC.\n";
        return MostOverlappingMove(brd.GetPosition(),
				   brd.Cons(color).GetFullCarriers(),
                                   brd.GetInferiorCells());
    }
    // Should never get here!
    BenzeneAssert(false);
    return INVALID_POINT;
}

/** Returns most blocking (ie, the "best") losing move. */
HexPoint PlayLostGame(const HexBoard& brd, HexColor color)
{
    BenzeneAssert(EndgameUtil::IsLostGame(brd, color));

    // Determine if color's opponent has guaranteed win
    HexColor other = !color;
    
    LogInfo() << "Opponent has won; playing most blocking move.\n";
    
    /** Uses semi-connections. 
        @see @ref playingdeterminedstates 
    */
    return MostOverlappingMove(
	brd.GetPosition(),
        brd.Cons(other).SemiExists() ?
        brd.Cons(other).GetSemiCarriers() : brd.Cons(other).GetFullCarriers(),
        brd.GetInferiorCells());
}

} // anonymous namespace

//----------------------------------------------------------------------------

bool EndgameUtil::IsWonGame(const HexBoard& brd, HexColor color, 
                            bitset_t& proof)
{
    if (brd.GetGroups().GetWinner() == color)
    {
        proof = StonesInProof(brd, color);
        return true;
    }
    bitset_t carrier;
    if (brd.Cons(color).SmallestSemiCarrier(carrier))
    {        
        proof = carrier | StonesInProof(brd, color);
	return true;
    }
    if (brd.Cons(color).SmallestFullCarrier(carrier))
    {
        proof = carrier | StonesInProof(brd, color);
	return true;
    }
    return false;
}

bool EndgameUtil::IsLostGame(const HexBoard& brd, HexColor color, 
                             bitset_t& proof)
{
    if (brd.GetGroups().GetWinner() == !color)
    {
        proof = StonesInProof(brd, !color);
        return true;
    }
    bitset_t carrier;
    if (brd.Cons(!color).SmallestFullCarrier(carrier))
    {
        proof = carrier | StonesInProof(brd, !color);
	return true;
    }
    if (ComputeConsiderSet(brd, color).none())
    {
        proof = brd.GetPosition().GetEmpty() | StonesInProof(brd, !color);
        return true;
    }
    return false;
}

bool EndgameUtil::IsDeterminedState(const HexBoard& brd, HexColor color, 
                                    HexEval& score, bitset_t& proof)
{
    score = 0;
    if (IsWonGame(brd, color, proof))
    {
        score = IMMEDIATE_WIN;
        return true;
    }
    if (IsLostGame(brd, color, proof))
    {
        score = IMMEDIATE_LOSS;
        return true;
    }
    return false;
}

HexPoint EndgameUtil::PlayDeterminedState(const HexBoard& brd, HexColor color)
                                          
{
    BenzeneAssert(HexColorUtil::isBlackWhite(color));
    BenzeneAssert(IsDeterminedState(brd, color));
    BenzeneAssert(!brd.GetGroups().IsGameOver());

    if (IsWonGame(brd, color))
        return PlayWonGame(brd, color);

    BenzeneAssert(IsLostGame(brd, color));
    return PlayLostGame(brd, color);
}

bitset_t EndgameUtil::MovesToConsider(const HexBoard& brd, HexColor color)
{
    BenzeneAssert(HexColorUtil::isBlackWhite(color));
    BenzeneAssert(!IsDeterminedState(brd, color));
    
    bitset_t consider = ComputeConsiderSet(brd, color);
    BenzeneAssert(consider.any());

    LogFine() << "Moves to consider for " << color << ":" 
              << brd.Write(consider) << '\n';
    return consider;
}

//----------------------------------------------------------------------------
