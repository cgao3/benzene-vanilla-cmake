//----------------------------------------------------------------------------
/** @file GraphUtil.hpp */
//----------------------------------------------------------------------------

#ifndef GRAPHUTIL_HPP
#define GRAPHUTIL_HPP

#include "Hex.hpp"
#include "Groups.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

class HexBoard;

/** Utilities on Graphs. */
namespace GraphUtil
{
    /** Computes neighbours of all empty cells going through groups of
        color. Neighbours of groups of color are also included in
        nbs. */
    void ComputeDigraph(const Groups& brd, HexColor color, 
                        PointToBitset& nbs);

    //----------------------------------------------------------------------

    /** The distance from the start cell to all unreachable cells. */
    static const int NOT_REACHED  = -1;

    /** Performs BFS starting at the given point. Distances from this
	point are stored in distFromEdge (whose memory has been
	already initialized, it is assumed). Bitset returned contains
	all empty cells reachable from p. The stopSet is a set of
	empty cells that may be visited but from which the BFS is not
	expanded. Note that the starting point will never be stopped,
	regardless of the stopSet.
     */
    bitset_t BFS(HexPoint p, PointToBitset& group_nbs, bitset_t stopSet,
		 int* distFromEdge=NULL, int* numShortestPathsThrough=NULL);

}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // GRAPHUTIL_HPP
