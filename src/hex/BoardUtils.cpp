//----------------------------------------------------------------------------
/** @file BoardUtils.cpp
 */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "SgRandom.h"

#include "BoardUtils.hpp"
#include "BitsetIterator.hpp"
#include "VCSet.hpp"
#include "GraphUtils.hpp"
#include "HexBoard.hpp"
#include "Pattern.hpp"
#include "HashedPatternSet.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

namespace {

//----------------------------------------------------------------------------

bool g_BoardUtilsDecompsInitialized = false;
std::vector<Pattern> g_oppmiai[BLACK_AND_WHITE];
HashedPatternSet g_hash_oppmiai[BLACK_AND_WHITE];

/** Initialize the miai pattern. */
void InitializeOppMiai()
{
    if (g_BoardUtilsDecompsInitialized) return;

    LogFine() << "--InitializeOppMiai" << '\n';

    //
    // Miai between groups of opposite color.
    // W is marked; so if you use this pattern on the black members of 
    // group, it will tell you the white groups that are adjacent to it. 
    // Used in the decomposition stuff. 
    //
    //               . W  
    //              * .                        [oppmiai/0]
    // m:5,0,4,4,0;1,0,0,0,0;0,0,0,0,0;0,0,0,0,0;0,0,0,0,0;0,0,0,0,0;1
    std::string oppmiai = "m:5,0,4,4,0;1,0,0,0,0;0,0,0,0,0;0,0,0,0,0;0,0,0,0,0;0,0,0,0,0;1";

    Pattern pattern;
    if (!pattern.unserialize(oppmiai)) {
        HexAssert(false);
    }
    pattern.setName("oppmiai");

    g_oppmiai[BLACK].push_back(pattern);
    pattern.flipColors();
    g_oppmiai[WHITE].push_back(pattern);
        
    for (BWIterator c; c; ++c)
        g_hash_oppmiai[*c].hash(g_oppmiai[*c]);
}

/** @todo Is it possible to speed this up? */
void ComputeAdjacentByMiai(const HexBoard& brd, PointToBitset& adjByMiai)
{
    HexAssert(g_BoardUtilsDecompsInitialized);

    adjByMiai.clear();
    for (BWIterator color; color; ++color) 
    {
        for (BitsetIterator p(brd.GetPosition().GetColor(*color) 
                              & brd.GetPosition().Const().GetCells()); 
             p; ++p) 
        {
            PatternHits hits;
            brd.GetPatternState().MatchOnCell(g_hash_oppmiai[*color], *p, 
                                              PatternState::MATCH_ALL, hits);
            HexPoint cp = brd.GetGroups().CaptainOf(*p);
            for (unsigned j=0; j<hits.size(); ++j) 
            {
                HexPoint cj = brd.GetGroups().CaptainOf(hits[j].moves1()[0]);
                adjByMiai[cj].set(cp);
                adjByMiai[cp].set(cj);
            }
        }
    }
}

} // namespace

//----------------------------------------------------------------------------

HexPoint BoardUtils::CoordsToPoint(const ConstBoard& brd, int x, int y)
{
    if (x <= -2 || x > brd.Width())      return INVALID_POINT;
    if (y <= -2 || y > brd.Height())     return INVALID_POINT;
    if ((x == -1 || x == brd.Width()) &&
	(y == -1 || y == brd.Height()))  return INVALID_POINT;

    if (y == -1)       return NORTH;
    if (y == brd.Height()) return SOUTH;
    if (x == -1)       return WEST;
    if (x == brd.Width())  return EAST;

    return HexPointUtil::coordsToPoint(x, y);
}

HexPoint BoardUtils::PointInDir(const ConstBoard& brd, 
                                HexPoint point, HexDirection dir)
{
    if (HexPointUtil::isEdge(point))
        return point;

    int x, y;
    HexAssert(HexPointUtil::isInteriorCell(point));
    HexPointUtil::pointToCoords(point, x, y);
    x += HexPointUtil::DeltaX(dir);
    y += HexPointUtil::DeltaY(dir);
    return BoardUtils::CoordsToPoint(brd, x, y);
}

HexPoint BoardUtils::Rotate(const ConstBoard& brd, HexPoint p)
{
    HexAssert(brd.IsValid(p));
    
    if (!brd.IsLocation(p)) return p;
    if (HexPointUtil::isEdge(p)) return HexPointUtil::oppositeEdge(p);
    
    int x, y;
    HexPointUtil::pointToCoords(p, x, y);
    return HexPointUtil::coordsToPoint(brd.Width()-1-x, brd.Height()-1-y);
}

HexPoint BoardUtils::Mirror(const ConstBoard& brd, HexPoint p)
{
    HexAssert(brd.IsValid(p));
    HexAssert(brd.Width() == brd.Height());
    
    if (!brd.IsLocation(p)) return p;
    
    if (HexPointUtil::isEdge(p)) {
	if (HexPointUtil::isColorEdge(p, VERTICAL_COLOR))
	    return HexPointUtil::rightEdge(p);
	else
	    return HexPointUtil::leftEdge(p);
    }
    
    int x, y;
    HexPointUtil::pointToCoords(p, x, y);
    return HexPointUtil::coordsToPoint(y, x);
}

HexPoint BoardUtils::CenterPoint(const ConstBoard& brd)
{
    HexAssert((brd.Width() & 1) && (brd.Height() & 1));
    return CenterPointRight(brd);
}

HexPoint BoardUtils::CenterPointRight(const ConstBoard& brd)
{
    int x = brd.Width() / 2;
    int y = brd.Height() / 2;

    if (!(brd.Width() & 1) && !(brd.Height() & 1)) y--;

    return HexPointUtil::coordsToPoint(x, y);
}

HexPoint BoardUtils::CenterPointLeft(const ConstBoard& brd)
{
    int x = brd.Width() / 2;
    int y = brd.Height() / 2;

    if (!(brd.Width() & 1)) x--;
    if ((brd.Width() & 1) && !(brd.Height() & 1)) y--;

    return HexPointUtil::coordsToPoint(x, y);
}

HexPoint BoardUtils::RandomEmptyCell(const StoneBoard& brd)
{
    bitset_t moves = brd.GetEmpty() & brd.Const().GetCells();
    int count = moves.count();
    if (count == 0) 
        return INVALID_POINT;
    
    int randMove = SgRandom::Global().Int(count) + 1;
    for (BitsetIterator p(moves); p; ++p) 
        if (--randMove==0) return *p;

    HexAssert(false);
    return INVALID_POINT;
}

//----------------------------------------------------------------------------

bitset_t BoardUtils::PackBitset(const ConstBoard& brd, 
                                const bitset_t& in)
{
    int j=0;
    bitset_t ret;
    for (BoardIterator it(brd.Interior()); it; ++it, ++j) {
        if (in.test(*it)) 
            ret.set(j);
    }
    return ret;
}

bitset_t BoardUtils::UnpackBitset(const ConstBoard& brd, 
                                  const bitset_t& in)
{
    int j=0;
    bitset_t ret;
    for (BoardIterator it(brd.Interior()); it; ++it, ++j) {
        if (in.test(j))
            ret.set(*it);
    }
    return ret;
}

bitset_t BoardUtils::Rotate(const ConstBoard& brd, 
                            const bitset_t& bs)
{
    bitset_t ret;
    for (BitsetIterator it(bs); it; ++it) {
        ret.set(Rotate(brd, *it));
    }
    return ret;
}

bitset_t BoardUtils::Mirror(const ConstBoard& brd, 
                            const bitset_t& bs)
{
    bitset_t ret;
    for (BitsetIterator it(bs); it; ++it) {
        ret.set(Mirror(brd, *it));
    }
    return ret;
}

bool BoardUtils::ShiftBitset(const ConstBoard& brd, const bitset_t& bs, 
                             HexDirection dir, bitset_t& out)
{
    out.reset();
    bool still_inside = true;
    for (BitsetIterator p(bs); p; ++p) {
        HexPoint s = PointInDir(brd, *p, dir);
        if (!HexPointUtil::isEdge(*p) && HexPointUtil::isEdge(s))
            still_inside = false;
        out.set(s);
    }
    return still_inside;
}

bool BoardUtils::ConnectedOnBitset(const ConstBoard& brd, 
                                   const bitset_t& carrier,
                                   HexPoint p1, HexPoint p2)
{
    HexAssert(carrier.test(p1));
    HexAssert(carrier.test(p2));
    bitset_t seen = ReachableOnBitset(brd, carrier, EMPTY_BITSET, p1);
    return seen.test(p2);
}

bitset_t BoardUtils::ReachableOnBitset(const ConstBoard& brd, 
                                       const bitset_t& carrier,
                                       const bitset_t& stopset,
                                       HexPoint start)
{
    HexAssert(carrier.test(start));
    bitset_t seen;
    std::queue<HexPoint> q;
    q.push(start);
    seen.set(start);
    while (!q.empty()) 
    {
        HexPoint p = q.front();
        q.pop();
        if (stopset.test(p)) 
            continue;
        for (BoardIterator nb(brd.Nbs(p)); nb; ++nb) 
        {
            if (carrier.test(*nb) && !seen.test(*nb)) 
            {
                q.push(*nb);
                seen.set(*nb);
            }
        }
    }
    return seen;
}

//----------------------------------------------------------------------------

void BoardUtils::InitializeDecompositions()
{
    InitializeOppMiai();
    g_BoardUtilsDecompsInitialized = true;
}

bool BoardUtils::FindCombinatorialDecomposition(const HexBoard& brd,
						HexColor color,
						bitset_t& captured)
{
    // If game is over or decided, don't do any work.
    HexPoint edge1 = HexPointUtil::colorEdge1(color);
    HexPoint edge2 = HexPointUtil::colorEdge2(color);
    const VCSet& cons = brd.Cons(color);
    if (brd.GetGroups().IsGameOver() || cons.Exists(edge1, edge2, VC::FULL)) 
    {
	captured.reset();
	return false;
    }
    
    /** Compute neighbouring groups of opposite color.

        @note Assumes that edges that touch are adjacent. See
        ConstBoard for more details. 
    */
    PointToBitset adjTo;
    PointToBitset adjByMiai;
    ComputeAdjacentByMiai(brd, adjByMiai);
    for (GroupIterator g(brd.GetGroups(), color); g; ++g) 
    {
	bitset_t opptNbs = adjByMiai[g->Captain()] 
            | (g->Nbs() & brd.GetPosition().GetColor(!color));
	if (opptNbs.count() >= 2)
	    adjTo[g->Captain()] = opptNbs;
    }
    // The two color edges are in the list. If no other groups are, then quit.
    HexAssert(adjTo.size() >= 2);
    if (adjTo.size() == 2) 
    {
	captured.reset();
	return false;
    }
    
    // Compute graph representing board from color's perspective.
    PointToBitset graphNbs;
    GraphUtils::ComputeDigraph(brd.GetGroups(), color, graphNbs);
    
    // Find (ordered) pairs of color groups that are VC-connected and have at
    // least two adjacent opponent groups in common.
    PointToBitset::iterator g1, g2;
    for (g1 = adjTo.begin(); g1 != adjTo.end(); ++g1) {
	for (g2 = adjTo.begin(); g2 != g1; ++g2) {
	    if ((g1->second & g2->second).count() < 2) continue;
	    if (!cons.Exists(g1->first, g2->first, VC::FULL)) continue;
	    
	    // This is such a pair, so at least one of the two is not an edge.
	    // Find which color edges are not equal to either of these groups.
	    HexAssert(!HexPointUtil::isEdge(g1->first) ||
		      !HexPointUtil::isEdge(g2->first));
	    bool edge1Free = (g1->first != edge1 && g2->first != edge1);
	    bool edge2Free = (g1->first != edge2 && g2->first != edge2);
	    
	    // Find the set of empty cells bounded by these two groups.
	    bitset_t stopSet = graphNbs[g1->first] | graphNbs[g2->first];
	    bitset_t decompArea;
	    decompArea.reset();
	    if (edge1Free)
		decompArea |= GraphUtils::BFS(edge1, graphNbs, stopSet);
	    if (edge2Free)
		decompArea |= GraphUtils::BFS(edge2, graphNbs, stopSet);
	    decompArea.flip();
	    decompArea &= brd.GetPosition().GetEmpty();
	    
	    // If the pair has a VC confined to these cells, then we have
	    // a decomposition - return it.
	    const VCList& vl = cons.GetList(VC::FULL, g1->first, g2->first);
	    for (VCList::const_iterator vi=vl.begin(); vi!=vl.end(); ++vi) {
		if (BitsetUtil::IsSubsetOf(vi->carrier(), decompArea)) {
		    captured = vi->carrier();
		    return true;
		}
	    }
	}
    }
    
    // No combinatorial decomposition with a VC was found.
    captured.reset();
    return false;
}

bool BoardUtils::FindSplittingDecomposition(const HexBoard& brd,
					    HexColor color,
					    HexPoint& group)
{
    // compute nbrs of color edges
    PointToBitset adjByMiai;
    ComputeAdjacentByMiai(brd, adjByMiai);
    const Groups& groups = brd.GetGroups();
    HexPoint edge1 = HexPointUtil::colorEdge1(!color);
    HexPoint edge2 = HexPointUtil::colorEdge2(!color);
    bitset_t adjto1 = adjByMiai[edge1] | groups.Nbs(edge1, color);
    bitset_t adjto2 = adjByMiai[edge2] | groups.Nbs(edge2, color);

    // @note must & with getCells() because we want non-edge groups;
    // this assumes that edges are always captains. 
    bitset_t adjToBothEdges = adjto1 & adjto2 & brd.Const().GetCells();

    // if there is a group adjacent to both opponent edges, return it.
    if (adjToBothEdges.any()) 
    {
        group = groups.CaptainOf(static_cast<HexPoint>
                                 (BitsetUtil::FirstSetBit(adjToBothEdges)));
	return true;
    }
    return false;
}

//----------------------------------------------------------------------------

std::string BoardUtils::GuiDumpOutsideConsiderSet(const StoneBoard& brd, 
                                                  const bitset_t& consider,
                                                  const bitset_t& remove)
{
    std::ostringstream os;
    bitset_t outside = brd.GetEmpty() - (remove | consider);
    for (BitsetIterator p(outside); p; ++p) 
        os << " " << *p << " x";
    return os.str();
}

//----------------------------------------------------------------------------
