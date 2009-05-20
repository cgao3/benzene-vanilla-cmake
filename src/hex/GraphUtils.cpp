//----------------------------------------------------------------------------
/** @file
 */
//----------------------------------------------------------------------------

#include "BoardUtils.hpp"
#include "BitsetIterator.hpp"
#include "VCSet.hpp"
#include "HexEval.hpp"
#include "GraphUtils.hpp"
#include "HexBoard.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

void ComputeVCNeighbours(const HexBoard& brd, HexColor c, 
                         PointToBitset& group_nbs)
{
    bitset_t captains;
    HexColorSet not_other = HexColorSetUtil::ColorOrEmpty(c);
    for (BoardIterator i(brd.Groups(not_other)); i; ++i) {
	captains.set(*i);
    }
    for (BoardIterator i(brd.Groups(not_other)); i; ++i) {
	group_nbs[*i] 
            = captains & VCSetUtil::ConnectedTo(brd.Cons(c), brd, *i, 
                                                VC::FULL);
    }
}

//----------------------------------------------------------------------------

void GraphUtils::ComputeDigraph(const GroupBoard& brd, HexColor color, 
                                PointToBitset& nbs)
{
    nbs.clear();

    // Copy adjacent nbs
    HexColorSet not_other = HexColorSetUtil::ColorOrEmpty(color);
    for (BoardIterator g(brd.Groups(not_other)); g; ++g) {
        nbs[*g] = brd.Nbs(*g, EMPTY);
    }
   
    // Go through groups of color
    for (BitsetIterator p(brd.getEmpty()); p; ++p) {
        for (BoardIterator nb(brd.ConstNbs(*p)); nb; ++nb) {
            if (brd.getColor(*nb) == color) {
                nbs[*p] |= nbs[brd.getCaptain(*nb)];
                nbs[*p].reset(*p);  // ensure p doesn't point to p
            }
        }
    }
}

//----------------------------------------------------------------------------

bitset_t GraphUtils::BFS(HexPoint p, PointToBitset& group_nbs,
			 bitset_t stopSet, int* distFromStart,
			 int* numShortestPathsThru)
{
    // Initialize distances from BFS starting point and number of shortest
    // paths through a location (if these values are desired).
    bool recordDistance = (distFromStart != NULL);
    bool computeFrequency = (numShortestPathsThru != NULL);
    HexAssert(recordDistance || !computeFrequency);
    
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

bitset_t GraphUtils::CellsOnShortestWinningPaths(const GroupBoard& brd, 
                                                 HexColor c)
{
    PointToBitset nbs;
    GraphUtils::ComputeDigraph(brd, c, nbs);
    int distFromStart[2][BITSETSIZE];
    return CellsOnShortestWinningPaths(brd, c, nbs, distFromStart);
}

bitset_t GraphUtils::CellsOnShortestWinningPaths(const GroupBoard& brd,
						 HexColor c,
						 PointToBitset& nbs,
					 int distFromStart[2][BITSETSIZE],
					 int numPaths[2][BITSETSIZE])
{
    UNUSED(brd);

    // Perform BFS from each of the color c edges using the given adjacencies.
    bitset_t stop;
    stop.set(HexPointUtil::colorEdge1(c));
    stop.set(HexPointUtil::colorEdge2(c));
    bool computeFrequencies = (numPaths != NULL);
    if (computeFrequencies) {
	BFS(HexPointUtil::colorEdge1(c), nbs, stop, 
                        distFromStart[0], numPaths[0]);
	BFS(HexPointUtil::colorEdge2(c), nbs, stop, 
                        distFromStart[1], numPaths[1]);
    } else {
	BFS(HexPointUtil::colorEdge1(c), nbs, stop, 
                        distFromStart[0]);
        BFS(HexPointUtil::colorEdge2(c), nbs, stop, 
                        distFromStart[1]);
    }
    
    // Use distances from edges to find cells on shortest paths.
    bitset_t onShortest;
    int shortestDist = BITSETSIZE + 1;
    for (int i=0; i<BITSETSIZE; i++) {
	// must be reached by both edges to be a candidate
	if (distFromStart[0][i] == GraphUtils::NOT_REACHED) continue;
	if (distFromStart[1][i] == GraphUtils::NOT_REACHED) continue;
	
	int curCellDist = distFromStart[0][i] + distFromStart[1][i];
	if (curCellDist < shortestDist) {
	    shortestDist = curCellDist;
	    onShortest.reset();
	    onShortest.set(i);
	} else if (curCellDist == shortestDist) {
	    onShortest.set(i);
	}
    }
    
#if 1
    LogFine() << "Shortest Path Frequency Info:" << '\n';
    for (BitsetIterator i(onShortest); i; ++i) {
	LogFine() << *i << " = (" 
		  << distFromStart[0][*i] << ", " 
		  << distFromStart[1][*i] << ")";

	if (computeFrequencies) {
	    LogFine() << " -> (" << numPaths[0][*i] 
		      << " x " << numPaths[1][*i]
		      << " = " << (numPaths[0][*i] * numPaths[1][*i]) 
		      << ")";
	}
	LogFine() << '\n';
    }
    LogFine() << '\n';
#endif
    
    return onShortest;
}

//----------------------------------------------------------------------------

bitset_t GraphUtils::FrequencyOnShortestWinningVCPaths(const HexBoard& brd, 
						       HexColor c,
						       bool inclEdges,
					       int numPaths[BITSETSIZE],
						       bool preferKeys)
{
    // Compute the graph's edges using VC info.
    PointToBitset group_nbs;
    ComputeVCNeighbours(brd, c, group_nbs);
    
    // Compute the cells required to form a winning VC path in the fewest
    // number of steps, and store how many shortest paths they are on.
    int dist[2][BITSETSIZE];
    int freq[2][BITSETSIZE];
    bitset_t onShort;
    onShort = CellsOnShortestWinningPaths(brd, c, group_nbs, dist, freq);
    
    for (int i=0; i<BITSETSIZE; i++) {
	if (onShort.test(i)) {
	    HexAssert(freq[0][i] >= 1);
	    HexAssert(freq[1][i] >= 1);
	    numPaths[i] = freq[0][i] * freq[1][i];
	} else {
	    numPaths[i] = 0;
	}
    }
    
#if 1
    LogFine() << "Cells on Shortest VC Paths (w/o edges):" << '\n';
    for (BitsetIterator i(onShort); i; ++i) {
	LogFine() << *i << ": " << numPaths[*i] << '\n';
    }
    LogFine() << '\n';
#endif
    if (!inclEdges) return onShort;
    
    // We now have the frequency of the `keys', but not the frequencies of
    // cells on the VC `edges'. We compute those now. Note that a `key' for
    // one path may also be on an `edge' for another.
    bitset_t onShortEdge;
    for (BitsetIterator i(onShort); i; ++i) {
	bitset_t nbrOfI = group_nbs[*i];
	
        for (BitsetIterator j(onShort & nbrOfI); *j != *i; ++j) {
	    if (dist[0][*i] == dist[0][*j]) continue;
	    
	    // Edge (i,j) is on some shortest path. Compute its carrier cells.
	    const VCList& lst = brd.Cons(c).GetList(VC::FULL, *i, *j);
	    bitset_t onThisEdge = lst.hardIntersection();
	    onShortEdge |= onThisEdge;
	    
	    // Compute number of shortest paths using this edge.
	    int freqOfEdge;
	    if (dist[0][*i] < dist[0][*j]) {
		HexAssert(dist[0][*i] + 1 == dist[0][*j]);
		freqOfEdge = freq[0][*i] * freq[1][*j];
	    } else {
		HexAssert(dist[0][*i] - 1 == dist[0][*j]);
		freqOfEdge = freq[0][*j] * freq[1][*i];
	    }
	    
	    // Update number of paths for cells on this edge.
            for (BitsetIterator k(onThisEdge); k; ++k) {
		numPaths[*k] += freqOfEdge;
	    }
	}
    }
    
    // Because we don't check edge carriers when computing shortest paths,
    // a cell may appear on a shortest path numerous times. Thus, here we
    // try to curb the over-estimation of a cell.
    HexAssert(numPaths[HexPointUtil::colorEdge1(c)] ==
	      numPaths[HexPointUtil::colorEdge2(c)]);
    int totalNumPaths = numPaths[HexPointUtil::colorEdge1(c)];
    for (int i=0; i<BITSETSIZE; i++) {
	if (numPaths[i] > totalNumPaths)
	    numPaths[i] = totalNumPaths;
	numPaths[i] *= 100;
	numPaths[i] /= totalNumPaths;
	if (preferKeys && !onShort.test(i)) {
	    numPaths[i] *= 3;
	    numPaths[i] /= 4;
	}
    }
    
#if 1
    LogFine() << "Cells on Shortest VC Paths (with edges):" << '\n';
    for (BitsetIterator i(onShort | onShortEdge); i; ++i) {
	LogFine() << *i << ": " << numPaths[*i] << "\%" << '\n';
    }
    LogFine() << '\n';
#endif
    
    return onShort | onShortEdge;
}

bitset_t GraphUtils::CellsOnShortestWinningVCPaths(const HexBoard& brd, 
						   HexColor c,
						   bool inclEdges)
{
    int numPaths[BITSETSIZE];
    return FrequencyOnShortestWinningVCPaths(brd, c, inclEdges, numPaths);
    /*bitset_t all = brd.getEmpty();
    ComputeShortestVCPathMoveOrdering(brd, c, all);
    return all;
    */
}

void GraphUtils::ComputeShortestVCPathMoveOrdering(const HexBoard& brd,
						   HexColor c,
						   bitset_t cellsToOrder)
{
    // Compute number of shortest VC paths using each cell.
    bitset_t ordering;
    int numPaths[BLACK_AND_WHITE][BITSETSIZE];
    for (BWIterator col; col; ++col) {
	bool adjust = (*col == c);
	ordering |= FrequencyOnShortestWinningVCPaths(brd, *col, true,
						      numPaths[*col], adjust);
    }
    
    // Sort the cells to be considered by frequency.
    std::vector<HexMoveValue> cellEvals(ordering.count());
    for (BitsetIterator i(cellsToOrder); i; ++i) {

	int distFromCentre = 0;
	distFromCentre += brd.Const().Distance(*i, 
                          BoardUtils::CenterPointLeft(brd.Const()));
	distFromCentre += brd.Const().Distance(*i, 
                          BoardUtils::CenterPointRight(brd.Const()));
	
	double value = 0.5 - (0.01 * distFromCentre);
	if (ordering.test(*i)) {
	    value += numPaths[0][*i] + numPaths[1][*i];
	}
	HexMoveValue hmv(*i, value);
	cellEvals.push_back(hmv);
    }
    stable_sort(cellEvals.begin(), cellEvals.end(),
		std::greater<HexMoveValue>());
    
    // Print out resulting move ordering.
    LogInfo() << "MOVE ORDERING" << '\n';
    for (unsigned i=0; i<cellEvals.size(); i++) 
    {
	LogInfo() << cellEvals[i].point()
		 << ": " << cellEvals[i].value()
		 << '\n';
    }
}

//----------------------------------------------------------------------------
