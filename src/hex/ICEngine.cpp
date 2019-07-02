//----------------------------------------------------------------------------
/** @file ICEngine.cpp */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "SgTimer.h"

#include "BoardUtil.hpp"
#include "BitsetIterator.hpp"
#include "HexColor.hpp"
#include "ICEngine.hpp"

#include "boost/filesystem/path.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

namespace {

//----------------------------------------------------------------------------
   
/** Returns set of cells not reachable from either edge. These areas
    are dead, but may not be identified via patterns, etc. Thus we use
    a BFS-type algorithm, checking which areas we can reach from an
    edge without going through the opposite edge or stones of the
    opponent's colour. Note that if the game is already decided, all
    remaining empty cells are dead.
*/
bitset_t ComputeEdgeUnreachableRegions(const StoneBoard& brd, HexColor c,
                                       const bitset_t& stopSet,
                                       bool flowFrom1=true,
                                       bool flowFrom2=true)
{
    bitset_t reachable;
    bitset_t flowSet = (brd.GetEmpty() | brd.GetColor(c)) 
        & brd.Const().GetCells();
    if (flowFrom1) 
    {
        bitset_t flowSet1 = flowSet;
        flowSet1.set(HexPointUtil::colorEdge1(c));
        reachable
	  |= BoardUtil::ReachableOnBitset(brd.Const(), flowSet1, stopSet,
					  HexPointUtil::colorEdge1(c));
    }
    if (flowFrom2) 
    {
        bitset_t flowSet2 = flowSet;
        flowSet2.set(HexPointUtil::colorEdge2(c));
        reachable
            |= BoardUtil::ReachableOnBitset(brd.Const(), flowSet2, stopSet,
					    HexPointUtil::colorEdge2(c));
    }
    return brd.GetEmpty() - reachable;
}

/** Computes dead regions on the board created by a single group's
    neighbour set. This finds dead regions that cannot be identified
    using only local patterns/properties.
*/
bitset_t ComputeDeadRegions(const Groups& groups)
{
    const StoneBoard& brd = groups.Board();
    if (groups.IsGameOver())
	return brd.GetEmpty();
    
    bitset_t dead;
    for (GroupIterator i(groups, NOT_EMPTY); i; ++i) 
    {
        /** @note We believe single stone groups cannot isolate regions by
	    themselves (i.e. they need to be combined with a non-singleton
	    group to create a dead region). This should be proven [Phil]. */
	if (i->Size() == 1)
            continue;
	
	HexColor c = i->Color();
	BenzeneAssert(HexColorUtil::isBlackWhite(c));
	
	/** Compute which empty cells are reachable from the edges when we
	    cannot go through this group's empty neighbours (which form a
	    clique). If the clique covers one edge, we only compute
	    reachability from the opposite edge. */
	bitset_t cliqueCutset = i->Nbs() & brd.GetEmpty();
	dead |= ComputeEdgeUnreachableRegions(brd, c, cliqueCutset,
                                 i->Captain() != HexPointUtil::colorEdge1(c),
                                 i->Captain() != HexPointUtil::colorEdge2(c));
    }
    
    // Areas not reachable due to one or more clique cutsets are dead.
    BenzeneAssert(BitsetUtil::IsSubsetOf(dead, brd.GetEmpty()));
    return dead;
}

/** Finds dead regions formed by one group as well as a single cell adjacent
    to two of the group's neighbours (but not the group itself).
*/
bitset_t FindType1Cliques(const Groups& groups)
{
    bitset_t dead;
    const StoneBoard& brd = groups.Board();
    bitset_t empty = brd.GetEmpty();
    
    // Find two cells that are adjacent through some group, but not directly.
    for (BitsetIterator x(empty); x; ++x) {
	for (BitsetIterator y(empty); *y != *x; ++y) {
	    if (brd.Const().Adjacent(*x, *y)) continue;
	    bitset_t xyNbs 
                = groups.Nbs(*x, NOT_EMPTY) & groups.Nbs(*y, NOT_EMPTY);
	    if (xyNbs.none()) continue;
	    
	    // Find a 3rd cell directly adjacent to the first two, but not
	    // adjacent to some group that connects them.
	    for (BitsetIterator z(empty); z; ++z) {
		if (!brd.Const().Adjacent(*x, *z)) continue;
		if (!brd.Const().Adjacent(*y, *z)) continue;
		BenzeneAssert(*x != *z);
		BenzeneAssert(*y != *z);
		bitset_t xyExclusiveNbs = xyNbs - groups.Nbs(*z, NOT_EMPTY);
		if (xyExclusiveNbs.none()) continue;
		
		// The 3 cells x,y,z form a clique.
		bitset_t clique;
		clique.set(*x);
		clique.set(*y);
		clique.set(*z);
		
		// The specific group(s) common to x and y do not affect the
		// stop set, so we check reachability at most once per color.
		if ((xyExclusiveNbs & brd.GetBlack()).any()) {
		    dead |= ComputeEdgeUnreachableRegions(brd, BLACK, clique);
		    BenzeneAssert(BitsetUtil::IsSubsetOf(dead, empty));
		}
		if ((xyExclusiveNbs & brd.GetWhite()).any()) {
		    dead |= ComputeEdgeUnreachableRegions(brd, WHITE, clique);
		    BenzeneAssert(BitsetUtil::IsSubsetOf(dead, empty));
		}
	    }
	}
    }
    BenzeneAssert(BitsetUtil::IsSubsetOf(dead, empty));
    return dead;
}

/** Finds dead regions formed by two groups of the same color, using
    common empty neighbours and a direct adjacency between two of
    their exclusive neighbours.
*/
bitset_t FindType2Cliques(const Groups& groups)
{
    const StoneBoard& brd = groups.Board();
    bitset_t dead;
    bitset_t empty = brd.GetEmpty();
    
    // Find two non-edge groups of the same color with both common
    // empty neighbours in common and also exclusive empty neighbours.
    for (BWIterator c; c; ++c) {
	for (GroupIterator g1(groups, *c); g1; ++g1) {
	    if (HexPointUtil::isEdge(g1->Captain())) continue;
	    bitset_t g1_nbs = groups.Nbs(*g1, EMPTY);
	    
	    for (GroupIterator g2(groups, *c); &*g2 != &*g1; ++g2) {
                if (HexPointUtil::isEdge(g2->Captain())) continue;
		bitset_t g2_nbs = groups.Nbs(*g2, EMPTY);
		if ((g1_nbs & g2_nbs).none()) continue;
		
		bitset_t g1Exclusive = g1_nbs - g2_nbs;
		if (g1Exclusive.none()) continue;
		bitset_t g2Exclusive = g2_nbs - g1_nbs;
		if (g2Exclusive.none()) continue;
		
		// Now find two cells exclusive neighbours of these two
		// groups that are directly adjacent to one another.
		for (BitsetIterator x(g1Exclusive); x; ++x) {
		    for (BitsetIterator y(g2Exclusive); y; ++y) {
			if (!brd.Const().Adjacent(*x, *y)) continue;
			
			// Cells x, y and the common neighbours of
			// groups g1, g2 form a clique.
			bitset_t clique = g1_nbs & g2_nbs;
			clique.set(*x);
			clique.set(*y);
			dead |= ComputeEdgeUnreachableRegions(brd, *c, clique);
			BenzeneAssert(BitsetUtil::IsSubsetOf(dead, empty));
		    }
		}
	    }
	}
    }
    BenzeneAssert(BitsetUtil::IsSubsetOf(dead, empty));
    return dead;
}

/** Finds dead regions cutoff by cliques created by 3 groups of the
    same color.
*/
bitset_t FindType3Cliques(const Groups& groups)
{
    const StoneBoard& brd = groups.Board();
    bitset_t dead;
    bitset_t empty = brd.GetEmpty();
    
    // Find 3 non-edge groups of the same color such that each pair has
    // a non-empty intersection of their empty neighbours.
    for (BWIterator c; c; ++c) {
	for (GroupIterator g1(groups, *c); g1; ++g1) {
	    if (HexPointUtil::isEdge(g1->Captain())) continue;
	    bitset_t g1_nbs = groups.Nbs(*g1, EMPTY);
	    
	    for (GroupIterator g2(groups, *c); &*g2 != &*g1; ++g2) {
		if (HexPointUtil::isEdge(g2->Captain())) continue;
		bitset_t g2_nbs = groups.Nbs(*g2, EMPTY);
		if ((g1_nbs & g2_nbs).none()) continue;
		
		for (GroupIterator g3(groups, *c); &*g3 != &*g2; ++g3) {
		    if (HexPointUtil::isEdge(g3->Captain())) continue;
		    bitset_t g3_nbs = groups.Nbs(*g3, EMPTY);
		    if ((g1_nbs & g3_nbs).none()) continue;
		    if ((g2_nbs & g3_nbs).none()) continue;
		    
		    // The union of the pairwise neighbour intersections
		    // of groups g1, g2, g3 form a clique.
		    bitset_t clique;
		    clique = (g1_nbs & g2_nbs) |
			     (g1_nbs & g3_nbs) |
			     (g2_nbs & g3_nbs);
		    dead |= ComputeEdgeUnreachableRegions(brd, *c, clique);
		    BenzeneAssert(BitsetUtil::IsSubsetOf(dead, empty));
		}
	    }
	}
    }
    BenzeneAssert(BitsetUtil::IsSubsetOf(dead, empty));
    return dead;
}

/** Computes dead regions on the board separated via a clique cutset
    composed of the intersection of three known maximal
    cliques. Returns the union of calls to FindType1Cliques(),
    FindType2Cliques(), and FindType3Cliques().
    
    This finds additional regions not identified via local patterns.
*/
bitset_t FindThreeSetCliques(const Groups& groups)
{
    if (groups.IsGameOver())
	return groups.Board().GetEmpty();

    bitset_t dead1 = FindType1Cliques(groups);
    bitset_t dead2 = FindType2Cliques(groups);
    bitset_t dead3 = FindType3Cliques(groups);

    return dead1 | dead2 | dead3;
}

//----------------------------------------------------------------------------

/** Returns true if the vector of points forms a clique on brd, while
    excluding any checks on exclude (to find pre-simplicial cells
    exclude should be in vn). */
bool IsClique(const ConstBoard& brd, const std::vector<HexPoint>& vn, 
              HexPoint exclude=INVALID_POINT)
{
    for (unsigned a=0; a<vn.size(); ++a) {
        if (vn[a] == exclude) continue;
        for (unsigned b=a+1; b<vn.size(); ++b) {
            if (vn[b] == exclude) continue;
            if (!brd.Adjacent(static_cast<HexPoint>(vn[a]),
                              static_cast<HexPoint>(vn[b])))
                return false;
        }
    }
    return true;
}

/** Finds dead and vulnerable cells using graph theory; ie, not using
    local patterns. The board will have any found dead cells filled
    in color. */
void UseGraphTheoryToFindDeadVulnerable(HexColor color, Groups& groups,
                                        PatternState& pastate,
                                        InferiorCells& inf)
{
    StoneBoard& brd = groups.Board();
    bitset_t simplicial;
    bitset_t adj_to_both_edges = 
        groups.Nbs(HexPointUtil::colorEdge1(color), EMPTY) &
        groups.Nbs(HexPointUtil::colorEdge2(color), EMPTY);
    bitset_t consider = brd.GetEmpty();
    consider = consider - adj_to_both_edges;
    
    // find presimplicial cells and their dominators
    for (BitsetIterator p(consider); p; ++p) 
    {
        std::set<HexPoint> enbs, cnbs;
        bitset_t empty_adj_to_group;
        bool adj_to_edge = false;
	HexPoint edgeNbr = INVALID_POINT;
	
        // Categorize neighbours as either 'empty' or 'color'. 
        for (BoardIterator nb(brd.Const().Nbs(*p)); nb; ++nb) 
        {
            HexColor ncolor = brd.GetColor(*nb);
            if (ncolor == EMPTY) 
            {
                enbs.insert(*nb);
            } 
            else if (ncolor == color) 
            {
                HexPoint cap = groups.CaptainOf(*nb);
                bitset_t adj = groups.Nbs(cap, EMPTY);
                adj.reset(*p);
		
                // Ignore color groups with no empty neighbours (after
                // removing *p).  If color group has one non-*p
                // neighbour, store it as an empty neighbour.
                // Otherwise, add as a color group (helps us to
                // identify cliques later).  Lastly, edges are a
                // special case - always added as a group.
                if (HexPointUtil::isColorEdge(cap, color)) 
                {
		    BenzeneAssert(!adj_to_edge || edgeNbr == cap);
                    adj_to_edge = true;
		    edgeNbr = cap;
                    cnbs.insert(cap);
                    empty_adj_to_group |= adj;
                } 
                else if (adj.count() == 1) 
                {
                    enbs.insert(static_cast<HexPoint>
                                (BitsetUtil::FindSetBit(adj)));
                } 
                else if (adj.count() >= 2) 
                {
                    cnbs.insert(cap);
                    empty_adj_to_group |= adj;
                }
            }
        }
        
        // Remove empty neighbours that are adjacent to a color neighbour.
        std::set<HexPoint>::iterator it;
        for (it = enbs.begin(); it != enbs.end(); ) 
        {
            if (empty_adj_to_group.test(*it)) 
            {
                enbs.erase(it);
                it = enbs.begin();
            } 
            else 
                ++it;
        }
	
        ////////////////////////////////////////////////////////////
        // if adjacent to one empty cell, or a single group of
        // your color, then neighbours are a clique, so *p is dead.
        if (enbs.size() + cnbs.size() <= 1)
            simplicial.set(*p);
        // Handle cells adjacent to the edge and those adjacent to
        // multiple groups of color (2 or 3). Need to test whether
        // the edge/a group's neighbours include all other groups'
        // neighbours, possibly omitting one. This, along with at most
        // one empty neighbour, makes the cell dead or vulnerable.
        else if (adj_to_edge || cnbs.size() >= 2) 
        {
	    if (enbs.size() >= 2) 
                continue;
	    
	    if (cnbs.size() == 1) 
            {
		BenzeneAssert(adj_to_edge && enbs.size() == 1);
                inf.AddVulnerable(*p, *enbs.begin());
	    } 
            else 
            {
		BenzeneAssert(!adj_to_edge || 
                          HexPointUtil::isColorEdge(edgeNbr, color));

		bitset_t killers_bs;
		bool isPreSimp = false;
		
		// Determine if *p is dead (flag if vulnerable)
		for (std::set<HexPoint>::iterator i = cnbs.begin();
                     i != cnbs.end(); ++i) 
                {
 		    // When adjacent to the edge, only the edge can
		    // trump other groups' adjacencies.
		    if (adj_to_edge && *i != edgeNbr) continue;

		    bitset_t remainingNbs = 
                        empty_adj_to_group - groups.Nbs(*i, EMPTY);
		    
		    if (remainingNbs.count() == 0) 
                    {
			if (enbs.size() == 0)
			    simplicial.set(*p);
                        else 
                        {
			    BenzeneAssert(enbs.size() == 1);
			    isPreSimp = true;
			    killers_bs.set(*enbs.begin());
			}
		    } 
                    else if (remainingNbs.count() == 1 && enbs.size() == 0) 
                    {
			isPreSimp = true;
			killers_bs.set(BitsetUtil::FindSetBit(remainingNbs));
		    }
		}
		
                if (!simplicial.test(*p) && isPreSimp) 
                {
		    BenzeneAssert(killers_bs.any());
                    for (BitsetIterator k(killers_bs); k; ++k)
                        inf.AddVulnerable(*p, *k);
		}
	    }

        }
	// If many neighbours and previous cases didn't apply, 
        // then most likely *p is not dead or vulnerable.
	else if (enbs.size() + cnbs.size() >= 4) 
        {
            // do nothing
	}
        // If adjacent to one group and some empty cells, then *p
	// cannot be dead, but might be vulnerable.
	else if (cnbs.size() == 1) 
        {
	    if (enbs.size() > 1) 
                continue;

	    BenzeneAssert(enbs.size() == 1);
	    BenzeneAssert(empty_adj_to_group.count() >= 2);
	    
	    // The single empty neighbour always kills *p
            inf.AddVulnerable(*p, *enbs.begin());
	    
	    if (empty_adj_to_group.count() == 2) 
            {
		// If the single group has only two neighbours, it is
		// possible that one or both of its neighbours are
		// adjacent to the single direct neighbour, causing us
		// to have more killers of *p
		HexPoint omit = *enbs.begin();
                for (BitsetIterator i(empty_adj_to_group); i; ++i)
                    enbs.insert(*i);
		
		// determine the additional killers of this vulnerable
		// cell
		std::vector<HexPoint> vn(enbs.begin(), enbs.end());
		for (unsigned ex=0; ex<vn.size(); ++ex) 
                {
		    if (vn[ex] == omit) 
                        continue;
		    if (IsClique(brd.Const(), vn, vn[ex]))
                        inf.AddVulnerable(*p, vn[ex]);
		}
	    }
	}
        else 
        {
            // If all empty neighbours form a clique, is dead. Otherwise
            // check if eliminating one makes the rest a clique.
            BenzeneAssert(cnbs.size() == 0);
            std::vector<HexPoint> vn(enbs.begin(), enbs.end());

            if (IsClique(brd.Const(), vn))
                simplicial.set(*p);
            else 
            {
                for (unsigned ex=0; ex<vn.size(); ++ex)
                    if (IsClique(brd.Const(), vn, vn[ex]))
                        inf.AddVulnerable(*p, vn[ex]);
            }
        }
    }
    // Add the simplicial stones to the board
    if (simplicial.any())
    {
	inf.AddFillin(color, simplicial);
        brd.AddColor(color, simplicial);
        pastate.Update(simplicial);
    }
}

//----------------------------------------------------------------------------

}  // anonymous namespace

//----------------------------------------------------------------------------

// By default, strongly reversible are not used as reversible because they
// are most of the time pruned.
ICEngine::ICEngine()
    : m_find_presimplicial_pairs(false),
      m_find_all_pattern_killers(false),
      m_find_all_pattern_superiors(true),
      m_find_three_sided_dead_regions(false),
      m_iterative_dead_regions(false),
      m_use_capture(true),
      m_find_reversible(true),
      m_use_s_reversible_as_reversible(false)
{
    LoadPatterns();
}

ICEngine::~ICEngine()
{
}

//----------------------------------------------------------------------------

/** Loads local patterns from "ice-pattern-file". */
void ICEngine::LoadPatterns()
{
    m_patterns.LoadPatterns("ice-patterns.txt");
}    

//----------------------------------------------------------------------------

HexPoint ICEngine::ComputeInferiorCells(HexColor color, Groups& groups,
					PatternState& pastate,
					InferiorCells& inf,
					HexPoint last_move,
					bool only_around_last_move) const
{
#ifndef NDEBUG
    BenzeneAssert(groups.Board() == pastate.Board());
    StoneBoard oldBoard(groups.Board());
#endif
    SgTimer timer;

    StoneBoard& brd = pastate.Board();
    HexPoint reverser = INVALID_POINT;
    bool find_reversible = m_find_reversible && (last_move != INVALID_POINT);

    // Warning : we cannot fillin to find reversible, as there is a risk that
    // the fillin is different from the one used for pruning at last step.
    // Thus, unless the fillin is incremental, we are forced to just use the
    // stones placed on the board.
    
    if (find_reversible)
    {
	BenzeneAssert(brd.IsColor(last_move, !color));
	reverser = IsReversible(pastate, !color, last_move);
    }
    if (only_around_last_move)
        ComputeFillin(groups, pastate, inf, color, BICOLOR, last_move);
    else
        ComputeFillin(groups, pastate, inf, color, BICOLOR);

    bitset_t consider = groups.Board().GetEmpty();
    FindSReversible(pastate, color, consider, inf);
    FindTReversible(pastate, color, consider, inf);
    FindInferior(pastate, color, consider, inf);
    
    LogFine() << "  " << timer.GetTime() << "s to find inferior cells.\n";

#ifndef NDEBUG
    BenzeneAssert(groups.Board().Hash() == oldBoard.Hash());
#endif
    
    return reverser;
}

std::size_t ICEngine::ComputeFillin(Groups& groups, PatternState& pastate,
				    InferiorCells& inf, HexColor color,
                                    FillinMode mode) const
{
    return ComputeFillin(groups, pastate, inf, color, mode,
			 groups.Board().GetEmpty());
}

std::size_t ICEngine::ComputeFillin(Groups& groups, PatternState& pastate,
				    InferiorCells& inf, HexColor color,
                                    FillinMode mode,
				    HexPoint last_move) const
{
    if (last_move == INVALID_POINT)
        return ComputeFillin(groups, pastate, inf, color, mode);
    bitset_t consider;
    for (BoardIterator p = groups.Board().Const()
	   .Nbs(last_move, Pattern::MAX_EXTENSION);
	 p; ++p)
        consider.set(*p);
    consider &= groups.Board().GetEmpty();
    return ComputeFillin(groups, pastate, inf, color, mode, consider);
}

std::size_t ICEngine::ComputeFillin(Groups& groups, PatternState& pastate,
				    InferiorCells& inf, HexColor color,
                                    FillinMode mod,
				    const bitset_t& consider,
				    bool clear_inf) const
{
    FillinMode mode = (m_use_capture ? mod : TurnOffCapture(mod));
    StoneBoard& board = groups.Board(); // physically == pastate.Board()
    std::size_t count = 0 ;
    if (clear_inf) inf.Clear();
    
    bool first = true;
    while(true)
    {
        if (first)
	    count += ComputePatternFillin(pastate, inf, color,
					  mode, consider);
	else
	    count += ComputePatternFillin(pastate, inf, color,
					  mode, board.GetEmpty());
	if (m_find_presimplicial_pairs)
	{
	    std::size_t loc_count = FillInVulnerable(color, groups,
						     pastate, inf);
	    if (UsesCapture(mode))
	        loc_count += FillInVulnerable(!color, groups,
					      pastate, inf);
	    if (loc_count != 0)
	    {
		count += loc_count;
		continue;
	    }
	}
	
        if (m_iterative_dead_regions)
	{
	    std::size_t loc_count = CliqueCutsetDead(color, groups,
						     pastate, inf);
	    if (loc_count != 0)
	    {
	        count += loc_count;
		continue;
	    }
	}
	
	break;
    }
    
    if (!m_iterative_dead_regions)
        count += CliqueCutsetDead(color, groups, pastate, inf);

    bitset_t captured = inf.Fillin(!color);
    if (IsMonocolorUsingCapture(mode) && captured.any())
    // we un-fillin the captured cell for the other color, and then we
    // try to monocolor-fillin.
    {
        inf.ClearFillin(!color);
	board.AddColor(EMPTY, captured);
	pastate.Update(captured);
	count -= captured.count();
	bitset_t consider = groups.Board().Const()
	  .Nbs(captured, Pattern::MAX_EXTENSION)
	  & groups.Board().GetEmpty();
	GroupBuilder::Build(board, groups);
	std::size_t loc_count = ComputeFillin(groups, pastate, inf, color,
					      MONOCOLOR, consider, false);
	if (loc_count)
	{
	    count += loc_count;
	    GroupBuilder::Build(board, groups);
	}
    }
    else if (count)
        GroupBuilder::Build(board, groups);
    return count;
}

std::size_t ICEngine::ComputePatternFillin(
				    PatternState& pastate,
				    InferiorCells& inf, HexColor color,
				    FillinMode mode,
				    const bitset_t& cons) const
{
    StoneBoard& board = pastate.Board();
    bitset_t consider = cons;
    std::size_t count = 0;
    while (consider.any())
    {
	// consider changes inside the loop, but the starting tests ensure
	// correctness whatever the behaviour is.
	for (BitsetIterator p(consider); p; ++p) 
	{
	    if (!consider.test(*p)) continue;
	    consider.reset(*p);
	    if (inf.Fillin(BLACK).test(*p)
		|| inf.Fillin(WHITE).test(*p))
	        continue;
	    PatternHits hits ;
	    pastate.MatchOnCell(m_patterns.HashedEFillin(), *p,
				PatternState::STOP_AT_FIRST_HIT, hits);
	    if (!hits.empty())
	    {
	        HexColor c = ( Bicolor(mode) ?
			       board.PickColor(color, *p) : color );
		inf.AddFillin(c, *p);
		board.AddColor(c, *p);
		pastate.Update(*p);
		for (BoardIterator n = board.Const()
		       .Nbs(*p,Pattern::MAX_EXTENSION); n; ++n)
		    if (board.IsEmpty(*n))
		        consider.set(*n);
		++count;
		continue;
	    }
	    
	    std::vector<HexColor> to_fillin;
	    to_fillin.push_back(color);
	    if (UsesCapture(mode))
	        to_fillin.push_back(!color);

	    for (std::vector<HexColor>::const_iterator c = to_fillin.begin();
		 c != to_fillin.end(); ++c)
	    {
	      if (*c == color || Bicolor(mode))
	          pastate.MatchOnCell(m_patterns.HashedFillin(*c), *p,
				      PatternState::STOP_AT_FIRST_HIT, hits);
	      else
		  pastate.MatchOnCell(m_patterns.HashedCaptured(*c), *p,
				      PatternState::STOP_AT_FIRST_HIT, hits);
	      if (!hits.empty())
	      {
		    inf.AddFillin(*c, *p);
		    board.AddColor(*c, *p);
		    pastate.Update(*p);
		    for (BoardIterator n = board.Const()
			   .Nbs(*p,Pattern::MAX_EXTENSION)
			   ; n; ++n)
		        if (board.IsEmpty(*n))
			    consider.set(*n);
		    ++count;
		    const std::vector<HexPoint>& others = hits[0].Moves1();
		    const std::vector<HexPoint>& opps = hits[0].Moves2();
		    for (std::vector<HexPoint>::const_iterator
			   it = others.begin(); it != others.end(); ++it)
		    {
			consider.reset(*it);
			inf.AddFillin(*c, *it);
			board.AddColor(*c, *it);
			pastate.Update(*it);
			for (BoardIterator n = board.Const()
			       .Nbs(*it,Pattern::MAX_EXTENSION)
			       ; n; ++n)
			    if (board.IsEmpty(*n))
			        consider.set(*n);
			++count;
		    }
		    
		    if (UsesCapture(mode))
		    // This optimisation (making it work for M_U_C too)
		    // works only because the cells in opps are captured.
		    {
		        for (std::vector<HexPoint>::const_iterator
			       it = opps.begin(); it != opps.end(); ++it)
			{
			    consider.reset(*it);
			    inf.AddFillin(!*c, *it);
			    board.AddColor(!*c, *it);
			    pastate.Update(*it);
			    for (BoardIterator n = board.Const()
				   .Nbs(*it,Pattern::MAX_EXTENSION)
				   ; n; ++n)
			        if (board.IsEmpty(*n))
				    consider.set(*n);
			    ++count;
			}
		    }
		    break; // do not fillin for the other color
		}
	    }
	}
    }
    return count;
}

/** Finds vulnerable cells for !color and finds presimplicial pairs and
    fills them for color.  Simplicial stones will be filled as color. */
std::size_t ICEngine::FillInVulnerable(HexColor color, Groups& groups, 
				       PatternState& pastate,
				       InferiorCells& inf) const
{ 
    inf.ClearVulnerable();
    inf.ClearSReversible();

    UseGraphTheoryToFindDeadVulnerable(!color, groups, pastate, inf);

    bitset_t consider = groups.Board().GetEmpty();
    FindVulnerable(pastate, !color, consider, inf);
    
    bitset_t fillin = inf.FindPresimplicialPairs();
    if (fillin.any()) 
    {
        inf.AddFillin(color, fillin);
	groups.Board().AddColor(color, fillin);
	pastate.Update(fillin);
    }
    return fillin.count();
}

/** Calls ComputeDeadRegions() and FindThreeSetCliques() and adds
    fillin to board and set of inferior cells. */
std::size_t ICEngine::CliqueCutsetDead(HexColor color, Groups& groups,
				       PatternState& pastate,
                                       InferiorCells& inf) const
{
    StoneBoard& brd = groups.Board();
    bitset_t notReachable = ComputeDeadRegions(groups);
    if (m_find_three_sided_dead_regions)
        notReachable |= FindThreeSetCliques(groups);
    if (notReachable.any()) 
    {
        inf.AddFillin(color, notReachable);
        brd.AddColor(color, notReachable);
        pastate.Update(notReachable);
    }
    return notReachable.count();
}

//----------------------------------------------------------------------------

void ICEngine::FindSReversible(const PatternState& pastate, HexColor color,
			       const bitset_t& consider,
			       InferiorCells& inf) const
{
    std::vector<PatternHits> hits(FIRST_INVALID);
    bitset_t rev = pastate.MatchOnBoard(consider, 
					m_patterns.HashedSReversible(color),
				        PatternState::MATCH_ALL, hits);
    // We have to use MATCH_ALL not to miss a vulnerable because of
    // a reversible, because some reversible may not be usable, and
    // because some reversible patterns have several reversible cells.

    // If it is vulnerable, we try not to add+remove as reversible.
    for (BitsetIterator p(rev); p; ++p) 
    {
        for (unsigned i=0; i<hits[*p].size(); ++i) 
        {
	    const std::vector<HexPoint>& empty = hits[*p][i].Empty();
	    if (empty.size() != 1)
	        continue;
	    // the only empty cell is the reverser, so vulnerable
	    const std::vector<HexPoint>& killer = hits[*p][i].Moves2();
	    BenzeneAssert(killer.size() == 1);
	    inf.AddVulnerable(*p, killer[0]);
	    if (!m_find_all_pattern_killers)
	      break;
        }
    }
    
    for (BitsetIterator p(rev); p; ++p) 
    {
	for (unsigned i=0; i<hits[*p].size(); ++i) 
	{
	    const std::vector<HexPoint>& empty = hits[*p][i].Empty();
	    if (empty.size() == 1)
	        continue;
	    const std::vector<HexPoint>& others = hits[*p][i].Moves1();
	    const std::vector<HexPoint>& reverser = hits[*p][i].Moves2();
	    BenzeneAssert(reverser.size() == 1);
	    bitset_t carrier;
	    for (unsigned j=0; j<empty.size(); ++j)
	        if (empty[j] != reverser[0])  
		    carrier.set(empty[j]);
	    inf.AddSReversible(*p, carrier, reverser[0], false);
	    carrier.set(*p);
	    for (std::vector<HexPoint>::const_iterator it = others.begin();
		 it != others.end(); ++it)
	    {
	        carrier.reset(*it);
		inf.AddSReversible(*it, carrier, reverser[0], false);
		carrier.set(*it);
	    }
	}
    }
}

void ICEngine::FindTReversible(const PatternState& pastate, HexColor color,
			       const bitset_t& consider,
			       InferiorCells& inf) const
{
    std::vector<PatternHits> hits(FIRST_INVALID);
    bitset_t rev = pastate.MatchOnBoard(consider, 
					m_patterns.HashedTReversible(color),
				        PatternState::MATCH_ALL, hits);
    
    for (BitsetIterator p(rev); p; ++p) 
    {
	for (unsigned i=0; i<hits[*p].size(); ++i) 
	{
	    const std::vector<HexPoint>& empty = hits[*p][i].Empty();
	    const std::vector<HexPoint>& reverser = hits[*p][i].Moves2();
	    BenzeneAssert(reverser.size() == 1);
	    bitset_t carrier;
	    for (unsigned j=0; j<empty.size(); ++j)
	        if (empty[j] != reverser[0])  
		    carrier.set(empty[j]);
	    inf.AddSReversible(*p, carrier, reverser[0], true);
	}
    }
}

void ICEngine::FindVulnerable(const PatternState& pastate, HexColor color,
			      const bitset_t& consider,
			      InferiorCells& inf) const
{
    PatternState::MatchMode matchmode = PatternState::STOP_AT_FIRST_HIT;
    if (m_find_all_pattern_killers)
        matchmode = PatternState::MATCH_ALL;
  
    std::vector<PatternHits> hits(FIRST_INVALID);
    bitset_t vul = pastate.MatchOnBoard(consider, 
					m_patterns.HashedVulnerable(color),
				        matchmode, hits);
    
    for (BitsetIterator p(vul); p; ++p) 
    {
        for (unsigned i=0; i<hits[*p].size(); ++i) 
        {
	    const std::vector<HexPoint>& reverser = hits[*p][i].Moves2();
	    BenzeneAssert(reverser.size() == 1);
	    inf.AddVulnerable(*p, reverser[0]);
	    if (!m_find_all_pattern_killers)
	        break;
        }
    }
}

void ICEngine::FindInferior(const PatternState& pastate, HexColor color, 
			    const bitset_t& consider,
			    InferiorCells& inf) const
{
    PatternState::MatchMode matchmode =
      (m_find_all_pattern_superiors ?
       PatternState::MATCH_ALL :
       PatternState::STOP_AT_FIRST_HIT);

    std::vector<PatternHits> hits(FIRST_INVALID);
    bitset_t infe = pastate.MatchOnBoard(consider,
					 m_patterns.HashedInferior(color),
					 matchmode, hits);

    for (BitsetIterator p(infe); p; ++p) 
    {
        for (unsigned i=0; i<hits[*p].size(); ++i) 
        {
            const std::vector<HexPoint>& others = hits[*p][i].Moves1();
	    const std::vector<HexPoint>& superior = hits[*p][i].Moves2();
            BenzeneAssert(superior.size() == 1);
            inf.AddInferior(*p, superior[0]);
	    for (unsigned j=0; j<others.size(); ++j)
	    {
		inf.AddInferior(others[j], superior[0]);
	    }
        }
    }
}

HexPoint ICEngine::IsReversible(PatternState& pastate,
				HexColor color, HexPoint p) const
{
    if (!p)
        return INVALID_POINT;

    pastate.Update();
    PatternHits hits;
    
    // First, the patterns centered on p.
    pastate.MatchOnCell(m_patterns.HashedReversible(color), p,
    		        PatternState::STOP_AT_FIRST_HIT, hits);
    if (!hits.empty())
        return hits[0].Moves2()[0];
    
    if (!m_use_s_reversible_as_reversible)
        return INVALID_POINT;
    
    pastate.MatchOnCell(m_patterns.HashedSReversible(color), p,
		        PatternState::STOP_AT_FIRST_HIT, hits);
    if (!hits.empty())
	return hits[0].Moves2()[0];
    
    // Then, the patterns centered on a nearby cell.
    // There is no not-strongly-reversible pattern of this kind.
    StoneBoard& brd = pastate.Board();
    bool occupied = brd.IsOccupied(p);
 
    if (occupied)
    {
        BenzeneAssert(brd.IsColor(p, color));
	brd.AddColor(EMPTY,p);
	pastate.Update(p);
    }
    
    for (BoardIterator q = brd.Const()
	   .Nbs(p, Pattern::MAX_EXTENSION_MOVES1);
	 q; ++q)
    {
        if (brd.IsOccupied(*q))
	    continue;
	PatternHits loc_hits;
	pastate.MatchOnCell(m_patterns.HashedSReversible(color), *q,
			    PatternState::MATCH_ALL, loc_hits);
        for (unsigned i=0; i<loc_hits.size(); ++i)
	{
	    const std::vector<HexPoint>& others = loc_hits[i].Moves1();
	    if (find(others.begin(), others.end(), p) != others.end())
	    {
	        if (occupied) {brd.AddColor(color,p); pastate.Update(p);}
		return loc_hits[i].Moves2()[0];
	    }
	}
    }
    if (occupied) {brd.AddColor(color,p); pastate.Update(p);}
    return INVALID_POINT;
}

PatternHits ICEngine::FindInferiorOnCell(const PatternState& pastate,
					 HexColor color, 
					 HexPoint cell) const
{
    PatternHits hits;
    StoneBoard brd = pastate.Board();
    pastate.MatchOnCell(m_patterns.HashedInferior(color), cell,
		        PatternState::MATCH_ALL, hits);
    for (BoardIterator p = brd.Const()
	   .Nbs(cell, Pattern::MAX_EXTENSION_MOVES1);
	 p; ++p)
    {
        if (brd.IsOccupied(*p))
	    continue;
	PatternHits loc_hits;
	pastate.MatchOnCell(m_patterns.HashedInferior(color), *p,
			    PatternState::MATCH_ALL, loc_hits);
        for (unsigned i=0; i<loc_hits.size(); ++i)
	{
	    const std::vector<HexPoint>& others = loc_hits[i].Moves1();
	    if (find(others.begin(), others.end(), cell) != others.end())
	        hits.push_back(loc_hits[i]);
	}
    }
    return hits;
}


ICEngine::FillinMode ICEngine::TurnOffCapture(FillinMode mode)
{
    switch (mode)
    {
	case MONOCOLOR : return MONOCOLOR;
	case MONOCOLOR_USING_CAPTURED : return MONOCOLOR;
	case BICOLOR : return BICOLOR;
    }
}

bool ICEngine::UsesCapture(FillinMode mode)
{
    switch (mode)
    {
	case MONOCOLOR : return false;
	case MONOCOLOR_USING_CAPTURED : return true;
	case BICOLOR : return true;
    }
}

bool ICEngine::IsMonocolorUsingCapture(FillinMode mode)
{
    switch (mode)
    {
	case MONOCOLOR : return false;
	case MONOCOLOR_USING_CAPTURED : return true;
	case BICOLOR : return false;
    }
}

bool ICEngine::Bicolor(FillinMode mode)
{
    switch (mode)
    {
	case MONOCOLOR : return false;
	case MONOCOLOR_USING_CAPTURED : return false;
	case BICOLOR : return true;
    }
}

//----------------------------------------------------------------------------

void IceUtil::Update(InferiorCells& out, const InferiorCells& in)
{
    // Overwrite old prunable with new prunable
    out.ClearVulnerable();
    out.ClearSReversible();
    out.ClearInferior();
    out.AddVulnerableFrom(in);
    out.AddSReversibleFrom(in);
    out.AddInferiorFrom(in);

    // Add the new fillin to the old fillin
    for (BWIterator c; c; ++c) 
    {
        out.AddFillin(*c, in.Fillin(*c));
    }
}

//----------------------------------------------------------------------------
