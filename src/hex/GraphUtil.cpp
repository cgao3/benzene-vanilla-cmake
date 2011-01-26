//----------------------------------------------------------------------------
/** @file GraphUtil.cpp */
//----------------------------------------------------------------------------

#include "BoardUtil.hpp"
#include "BitsetIterator.hpp"
#include "GraphUtil.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

void GraphUtil::ComputeDigraph(const Groups& groups, HexColor color, 
                               PointToBitset& nbs)
{
    nbs.clear();

    // Copy adjacent nbs
    HexColorSet not_other = HexColorSetUtil::ColorOrEmpty(color);
    for (GroupIterator g(groups, not_other); g; ++g)
        nbs[g->Captain()] = groups.Nbs(*g, EMPTY);

    // Go through groups of color
    for (GroupIterator g(groups, EMPTY); g; ++g) 
    {
        for (BitsetIterator nb(groups.Nbs(*g, color)); nb; ++nb) 
        {
            nbs[g->Captain()] |= nbs[groups.CaptainOf(*nb)];
            nbs[g->Captain()].reset(g->Captain());
        }
    }
}

//----------------------------------------------------------------------------

bitset_t GraphUtil::BFS(HexPoint p, PointToBitset& group_nbs,
                        bitset_t stopSet, int* distFromStart,
                        int* numShortestPathsThru)
{
    // Initialize distances from BFS starting point and number of shortest
    // paths through a location (if these values are desired).
    bool recordDistance = (distFromStart != NULL);
    bool computeFrequency = (numShortestPathsThru != NULL);
    BenzeneAssert(recordDistance || !computeFrequency);
    
    if (recordDistance) {
	for (int i=0; i<BITSETSIZE; i++)
	    distFromStart[i] = NOT_REACHED;
	distFromStart[p] = 0;
	
	if (computeFrequency) {
	    for (int i=0; i<BITSETSIZE; i++)
		numShortestPathsThru[i] = 0;
	    numShortestPathsThru[p] = 1;
	}
    }
    
    // Initialize queue to starting point and alter stop set to exclude start.
    bitset_t visited;
    std::queue<HexPoint> q;
    q.push(p);
    visited.set(p);
    bitset_t stop = stopSet;
    stop.reset(p);
    
    // Continue BFS until all reachable cells have been visited.
    while (!q.empty()) {
	// Get next cell on queue
	HexPoint curCell = q.front();
	q.pop();
	
	// Do not add this cell's neighbours if it is marked as a stop cell.
	if (stop.test(curCell)) continue;
	
	// Compute current cell's neighbours, and update number of paths this
	// cell's neighbours are on.
	bitset_t nbs = group_nbs[curCell];
	if (computeFrequency) {
	    for (int i=0; i<BITSETSIZE; i++) {
		if (!nbs.test(i)) continue;
		if (distFromStart[i] == NOT_REACHED ||
		    distFromStart[i] > distFromStart[curCell])
		    numShortestPathsThru[i] += numShortestPathsThru[curCell];
	    }
	}
	
	// Add previously-unvisited neighbours to queue and mark as visited.
	nbs = nbs - visited;
	visited |= nbs;
        for (BitsetIterator i(nbs); i; ++i) {
	    q.push(*i);
	    if (recordDistance)
		distFromStart[*i] = distFromStart[curCell] + 1;
	}
    }
    
    return visited;
}

//----------------------------------------------------------------------------
