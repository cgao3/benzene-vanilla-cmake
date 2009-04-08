//----------------------------------------------------------------------------
// $Id: GraphUtils.hpp 1821 2008-12-22 04:03:08Z broderic $
//----------------------------------------------------------------------------

#ifndef GRAPHUTILS_HPP
#define GRAPHUTILS_HPP

#include "Hex.hpp"
#include "GroupBoard.hpp"

//----------------------------------------------------------------------------

class HexBoard;

/** Utilities on Graphs. */
namespace GraphUtils
{
    /** Computes neighbours of all empty cells going through groups of
        color. Neighbours of groups of color are also included in
        nbs. */
    void ComputeDigraph(const GroupBoard& brd, HexColor color, 
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

    //----------------------------------------------------------------------

    /** Compute which cells are on a shortest edge-to-edge path for
	the given colour using the adjacencies given. If no
	adjacencies are given, uses the default (direct and through
	groups of colour c).  If numPaths is passed in, the number of
	shortest paths each cell is on will be stored there.
     */
    bitset_t CellsOnShortestWinningPaths(const GroupBoard& brd, HexColor c);
    bitset_t CellsOnShortestWinningPaths(const GroupBoard& brd, HexColor c,
					 PointToBitset& nbs,
					 int distFromStart[2][BITSETSIZE],
					 int numPaths[2][BITSETSIZE]=NULL);


    //----------------------------------------------------------------------
    
    /** Computes the frequency with which each cell is on a shortest path
     */
    bitset_t FrequencyOnShortestWinningVCPaths(const HexBoard& brd, 
					       HexColor c,
					       bool inclEdges,
					       int numPaths[BITSETSIZE],
					       bool preferKeys=false);
    
    /** Computes which cells are on shortest edge-to-edge VC paths
	for the given colour. */
    bitset_t CellsOnShortestWinningVCPaths(const HexBoard& brd, 
                                           HexColor c, bool inclEdges);
    
    /** Computes move ordering based on frequency of cell on shortest
	winning VC paths for both players
     */
    void ComputeShortestVCPathMoveOrdering(const HexBoard& brd, HexColor c,
					   bitset_t cellsToOrder);

};

//----------------------------------------------------------------------------

#endif // GRAPHUTILS_HPP
