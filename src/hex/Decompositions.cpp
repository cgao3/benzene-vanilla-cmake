//----------------------------------------------------------------------------
/** @file Decompositions.cpp */
//----------------------------------------------------------------------------

#include "BitsetIterator.hpp"
#include "Decompositions.hpp"
#include "GraphUtil.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

namespace {

bool g_decompsInitialized = false;
std::vector<Pattern> g_oppmiai[BLACK_AND_WHITE];
HashedPatternSet g_hash_oppmiai[BLACK_AND_WHITE];

/** Initialize the miai pattern. */
void InitializeOppMiai()
{
    if (g_decompsInitialized) 
        return;

    LogFine() << "--InitializeOppMiai\n";

    //
    // Miai between groups of opposite color.
    // W is marked; so if you use this pattern on the black members of 
    // group, it will tell you the white groups that are adjacent to it. 
    // Used in the decomposition stuff. 
    //
    //               . W  
    //              * .                        [oppmiai/0]
    // m:5,0,4,4,0;1,0,0,0,0;0,0,0,0,0;0,0,0,0,0;0,0,0,0,0;0,0,0,0,0;1
    std::string oppmiai 
        = "m:5,0,4,4,0;1,0,0,0,0;0,0,0,0,0;0,0,0,0,0;0,0,0,0,0;0,0,0,0,0;1";

    Pattern pattern;
    if (!pattern.Unserialize(oppmiai))
        throw BenzeneException("InitializeOppMiai: unable to parse pattern!");

    pattern.SetName("oppmiai");

    g_oppmiai[BLACK].push_back(pattern);
    pattern.FlipColors();
    g_oppmiai[WHITE].push_back(pattern);
        
    for (BWIterator c; c; ++c)
        g_hash_oppmiai[*c].Hash(g_oppmiai[*c]);
}

/** @todo Is it possible to speed this up? */
void ComputeAdjacentByMiai(const HexBoard& brd, PointToBitset& adjByMiai)
{
    BenzeneAssert(g_decompsInitialized);

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
                HexPoint cj = brd.GetGroups().CaptainOf(hits[j].Moves1()[0]);
                adjByMiai[cj].set(cp);
                adjByMiai[cp].set(cj);
            }
        }
    }
}

//----------------------------------------------------------------------------

} // anonymous namespace

//----------------------------------------------------------------------------

void Decompositions::Initialize()
{
    InitializeOppMiai();
    g_decompsInitialized = true;
}

bool Decompositions::Find(const HexBoard& brd, HexColor color,
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

        NOTE: Assumes that edges that touch are adjacent. See
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
    BenzeneAssert(adjTo.size() >= 2);
    if (adjTo.size() == 2) 
    {
	captured.reset();
	return false;
    }
    
    // Compute graph representing board from color's perspective.
    PointToBitset graphNbs;
    GraphUtil::ComputeDigraph(brd.GetGroups(), color, graphNbs);
    
    // Find (ordered) pairs of color groups that are VC-connected and have at
    // least two adjacent opponent groups in common.
    PointToBitset::iterator g1, g2;
    for (g1 = adjTo.begin(); g1 != adjTo.end(); ++g1) {
	for (g2 = adjTo.begin(); g2 != g1; ++g2) {
	    if ((g1->second & g2->second).count() < 2) continue;
	    if (!cons.Exists(g1->first, g2->first, VC::FULL)) continue;
	    
	    // This is such a pair, so at least one of the two is not an edge.
	    // Find which color edges are not equal to either of these groups.
	    BenzeneAssert(!HexPointUtil::isEdge(g1->first) ||
                          !HexPointUtil::isEdge(g2->first));
	    bool edge1Free = (g1->first != edge1 && g2->first != edge1);
	    bool edge2Free = (g1->first != edge2 && g2->first != edge2);
	    
	    // Find the set of empty cells bounded by these two groups.
	    bitset_t stopSet = graphNbs[g1->first] | graphNbs[g2->first];
	    bitset_t decompArea;
	    decompArea.reset();
	    if (edge1Free)
		decompArea |= GraphUtil::BFS(edge1, graphNbs, stopSet);
	    if (edge2Free)
		decompArea |= GraphUtil::BFS(edge2, graphNbs, stopSet);
	    decompArea.flip();
	    decompArea &= brd.GetPosition().GetEmpty();
	    
	    // If the pair has a VC confined to these cells, then we have
	    // a decomposition - return it.
	    const VCList& vl = cons.GetList(VC::FULL, g1->first, g2->first);
	    for (VCListConstIterator it(vl); it; ++it)
            {
		if (BitsetUtil::IsSubsetOf(it->Carrier(), decompArea)) 
                {
		    captured = it->Carrier();
		    return true;
		}
	    }
	}
    }
    
    // No combinatorial decomposition with a VC was found.
    captured.reset();
    return false;
}

bool Decompositions::FindSplitting(const HexBoard& brd, HexColor color,
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

    // NOTE: must & with getCells() because we want non-edge groups;
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
