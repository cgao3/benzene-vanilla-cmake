//----------------------------------------------------------------------------
// $Id: VCUtils.cpp 1786 2008-12-14 01:55:45Z broderic $
//----------------------------------------------------------------------------

#include "VCUtils.hpp"
#include "Connections.hpp"
#include "HexBoard.hpp"
#include "BoardUtils.hpp"

//----------------------------------------------------------------------------

/** Number of cells in a MIAI (bridge). 
    @note Should always be 2! */
#define MIAI_SIZE              2

//----------------------------------------------------------------------------

bool VCUtils::ValidEdgeBridge(const StoneBoard& brd, 
                              const bitset_t& carrier, 
                              HexPoint& endpoint,
                              HexPoint& edge)
{
    // must have carrier of size two
    if (carrier.count() != MIAI_SIZE) return false;

    // the carrier must be of empty cells
    if ((brd.getOccupied() & carrier).any()) return false;
    
    // find the two cells in the VC's carrier...
    std::vector<HexPoint> miai;
    BitsetUtil::BitsetToVector(carrier, miai);
    
    // carrier cells must be neighbours to qualify as bridge
    if (!brd.Adjacent(miai[0], miai[1])) 
        return false;

    // find the two cells adjacent to both
    std::vector<HexPoint> adj;
    for (BoardIterator n1(brd.ConstNbs(miai[0])); n1; ++n1)
        for (BoardIterator n2(brd.ConstNbs(miai[1])); n2; ++n2)
            if (*n1 == *n2) adj.push_back(*n1);
    HexAssert(adj.size() == 2);

    // pick the edge and endpoint
    for (std::size_t i=0; i<2; ++i) 
    {
        if (HexPointUtil::isEdge(adj[i])) {
            edge = adj[i];
            endpoint = adj[i^1];
            return true;
        }
    }

    // did not touch an edge, so return false
    return false;
}

//----------------------------------------------------------------------------

/** Returns true iff the given VC is a bridge. That is, it has a
    carrier of size two with the carrier cells adjacent, and both
    cells adjacent to these carrier cells are of the required
    color.  This method is used in strengthenVCs in HexBoard, to
    ensure that bridges (which are supported by a rollout pattern)
    are not strengthened.
*/
bool isBridge(const StoneBoard& brd, HexColor color, const VC& vc)
{
    // must have carrier of size two
    if (vc.carrier().count() != MIAI_SIZE) return false;
    
    // find the two cells in the VC's carrier...
    std::vector<HexPoint> miai;
    BitsetUtil::BitsetToVector(vc.carrier(), miai);
    
    // carrier cells must be neighbours to qualify as bridge
    if (!brd.Adjacent(miai[0], miai[1])) 
        return false;
    
    // lastly, we need to check that the two cells adjacent
    // to both carrier cells are the right color (otherwise
    // the rollout pattern will not match)
    bitset_t commonNbs;
    bool bridgePatternMatches = true;
    for (BoardIterator it1 = brd.ConstNbs(miai[0]); it1; ++it1)  
    {
	for (BoardIterator it2 = brd.ConstNbs(miai[1]); it2; ++it2) 
        {
	    if (*it1 == *it2) 
            {
		commonNbs.set(*it1);
                bridgePatternMatches 
                    = bridgePatternMatches && brd.isColor(*it1, color);
	    }
	}
    }
    HexAssert(commonNbs.count() == 2);
    return bridgePatternMatches;
}

bool ValidMaintainableEndpoints(const HexBoard& brd, HexColor color, 
                                const VC& vc)
{
    std::vector<HexPoint> end, edge;
    end.push_back(vc.x());
    end.push_back(vc.y());
    edge.push_back(HexPointUtil::colorEdge1(color));
    edge.push_back(HexPointUtil::colorEdge2(color));

    // need these to avoid flowing through the edge group stones
    bitset_t edge_stones[2];
    for (int i=0; i<2; ++i)
    {
        edge_stones[i]
            = BoardUtils::ReachableOnBitset(brd.Const(), brd.getColor(color), 
                                            EMPTY_BITSET, edge[i]);
    }

    // can travel over color's stones and empty cells not in the vc's
    // carrier.
    bitset_t carrier = (brd.getEmpty() | brd.getColor(color)) - vc.carrier();

    // Ensure the following:
    //  1) each edge can reach at least one endpoint
    //  2) both endpoints are reachable from the edges
    bitset_t reachable[2];
    for (int i=0; i<2; ++i) {
        bitset_t our_carrier = carrier - edge_stones[i^1];
        reachable[i] 
            = BoardUtils::ReachableOnBitset(brd.Const(), our_carrier, 
                                            EMPTY_BITSET, edge[i]);
        if (!reachable[i].test(end[0]) && !reachable[i].test(end[1]))
            return false;
    }   

    bitset_t reached = reachable[0] | reachable[1];
    if (!reached.test(end[0]) || !reached.test(end[1]))
        return false;

    return true;
}

/** Returns true if vc is not a bridge and there does not exist a
    color group with smallest vcs to both of vc's endpoints which are
    non-intersecting and both subsets of vc. */
bool ValidMaintainable(const HexBoard& brd, HexColor color, const VC& vc)
{
    if (isBridge(brd, color, vc))
        return false;

    if (vc.rule() == VC_RULE_AND)
    {
        for (BoardIterator g(brd.Groups(color)); g; ++g) {
            if (*g == vc.x() || *g == vc.y()) continue;
		
            VC vc1, vc2;
            if (brd.Cons(color).SmallestVC(vc.x(), *g, VC::FULL, vc1) &&
                brd.Cons(color).SmallestVC(vc.y(), *g, VC::FULL, vc2)) 
            {
                // If the smaller VCs compose the larger one, mark it
                // for deletion.
                if ((vc1.carrier() & vc2.carrier()).none() &&
                    BitsetUtil::IsSubsetOf(vc1.carrier(), vc.carrier()) &&
                    BitsetUtil::IsSubsetOf(vc2.carrier(), vc.carrier())) 
                {
                    HexAssert((vc1.carrier() | vc2.carrier()) ==
                              vc.carrier());
                    return false;
                }
            }
        }
    }

    // ensure endpoints are valid
    return ValidMaintainableEndpoints(brd, color, vc);
}

void VCUtils::findMaintainableVCs(const HexBoard& brd, HexColor c,
				  std::vector<VC>& maintain,
                                  std::size_t max)
{
    HexAssert(HexColorUtil::isBlackWhite(c));
    
    // For all pairs of color c groups in brd
    for (BoardIterator g1(brd.Groups(c)); g1; ++g1) 
    {
	for (BoardIterator g2(brd.Groups(c)); *g2 != *g1; ++g2) 
        {
            VC vc;
	    if (maintain.size() < max 
                && brd.Cons(c).SmallestVC(*g1, *g2, VC::FULL, vc)
                && ValidMaintainable(brd, c, vc))
            {
		maintain.push_back(vc);
	    }
	}
    }
}

//----------------------------------------------------------------------------

