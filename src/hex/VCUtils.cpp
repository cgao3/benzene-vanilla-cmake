//----------------------------------------------------------------------------
/** @file VCUtils.cpp
 */
//----------------------------------------------------------------------------

#include "VCUtils.hpp"
#include "VCSet.hpp"
#include "HexBoard.hpp"
#include "BoardUtils.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

bitset_t VCUtils::GetMustplay(const HexBoard& brd, HexColor color)
{
    HexColor other = !color;
    HexPoint edge1 = HexPointUtil::colorEdge1(other);
    HexPoint edge2 = HexPointUtil::colorEdge2(other);

    if (brd.Cons(other).Exists(edge1, edge2, VC::FULL))
        return EMPTY_BITSET;

    const VCList& semi = brd.Cons(other).GetList(VC::SEMI, edge1, edge2);
    bitset_t intersection = semi.hardIntersection();
    intersection &= brd.GetPosition().GetEmpty(); // FIXME: need this line?

    return intersection;
}

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
    if ((brd.GetOccupied() & carrier).any()) return false;
    
    // find the two cells in the VC's carrier...
    std::vector<HexPoint> miai;
    BitsetUtil::BitsetToVector(carrier, miai);
    
    // carrier cells must be neighbours to qualify as bridge
    if (!brd.Const().Adjacent(miai[0], miai[1])) 
        return false;

    // find the two cells adjacent to both
    std::vector<HexPoint> adj;
    for (BoardIterator n1(brd.Const().Nbs(miai[0])); n1; ++n1)
        for (BoardIterator n2(brd.Const().Nbs(miai[1])); n2; ++n2)
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
