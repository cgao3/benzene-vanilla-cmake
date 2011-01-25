//----------------------------------------------------------------------------
/** @file ICEngine.cpp */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "SgTimer.h"

#include "BitsetIterator.hpp"
#include "ICEngine.hpp"
#include "BoardUtil.hpp"

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
    bitset_t reachable1, reachable2;
    bitset_t flowSet = (brd.GetEmpty() | brd.GetColor(c)) 
        & brd.Const().GetCells();
    if (flowFrom1) 
    {
        bitset_t flowSet1 = flowSet;
        flowSet1.set(HexPointUtil::colorEdge1(c));
        reachable1 
            = BoardUtil::ReachableOnBitset(brd.Const(), flowSet1, stopSet,
                                           HexPointUtil::colorEdge1(c));
    }
    if (flowFrom2) 
    {
        bitset_t flowSet2 = flowSet;
        flowSet2.set(HexPointUtil::colorEdge2(c));
        reachable2 
            = BoardUtil::ReachableOnBitset(brd.Const(), flowSet2, stopSet,
                                           HexPointUtil::colorEdge2(c));
    }
    return brd.GetEmpty() - (reachable1 | reachable2);
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
	    group to create a dead region. This should be proven [Phil]. */
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
    local patterns. The board will have any found dead cells
    filled-in. */
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
        inf.AddDead(simplicial);
        brd.AddColor(DEAD_COLOR, simplicial);
        pastate.Update(simplicial);
        GroupBuilder::Build(brd, groups);
    }
}

//----------------------------------------------------------------------------

}  // anonymous namespace

//----------------------------------------------------------------------------

ICEngine::ICEngine()
    : m_find_presimplicial_pairs(true),
      m_find_permanently_inferior(true),
      m_find_mutual_fillin(false),
      m_find_all_pattern_killers(true),
      m_find_all_pattern_reversers(false),
      m_find_all_pattern_dominators(false),
      m_use_handcoded_patterns(true),
      m_backup_opponent_dead(false),
      m_find_three_sided_dead_regions(false),
      m_iterative_dead_regions(false)
{
    LoadHandCodedPatterns();
    LoadPatterns();
}

ICEngine::~ICEngine()
{
}

//----------------------------------------------------------------------------

/** Creates the set of hand-coded patterns. */
void ICEngine::LoadHandCodedPatterns()
{
    HandCodedPattern::CreatePatterns(m_hand_coded);
    LogFine() << "ICEngine: " << m_hand_coded.size()
              << " hand coded patterns.\n";
}

/** Loads local patterns from "ice-pattern-file". */
void ICEngine::LoadPatterns()
{
    m_patterns.LoadPatterns("ice-patterns.txt");
}    

//----------------------------------------------------------------------------

std::size_t ICEngine::ComputeDeadCaptured(Groups& groups, PatternState& pastate,
                                          InferiorCells& inf, 
                                          HexColorSet colors_to_capture) const
{
    StoneBoard& brd = groups.Board();
    // find dead and captured cells and fill them in. 
    std::size_t count = 0;
    while (true) 
    {
        // search for dead; if some are found, fill them in
        // and iterate again.
        while (true) 
        {
            /** @todo This can be optimized quite a bit. */
            bitset_t dead = FindDead(pastate, brd.GetEmpty());
            if (dead.none()) 
                break;
            count += dead.count();
            inf.AddDead(dead);
            brd.AddColor(DEAD_COLOR, dead);
            pastate.Update(dead);
        }

        // search for black captured cells; if some are found,
        // fill them in and go back to look for more dead. 
        {
            bitset_t black;
            if (HexColorSetUtil::InSet(BLACK, colors_to_capture))
                black = FindCaptured(pastate, BLACK, brd.GetEmpty());
            if (black.any()) 
            {
                count += black.count();
                inf.AddCaptured(BLACK, black);
                brd.AddColor(BLACK, black);
                pastate.Update(black);
                continue;
            }
        }

        // search for white captured cells; if some are found, fill
        // them in and go back to look for more dead/black captured.
        {
            bitset_t white;
            if (HexColorSetUtil::InSet(WHITE, colors_to_capture))
                white = FindCaptured(pastate, WHITE, brd.GetEmpty());
            if (white.any()) 
            {
                count += white.count();
                inf.AddCaptured(WHITE, white);
                brd.AddColor(WHITE, white);
                pastate.Update(white);
                continue;
            }
        }
        // did not find any fillin, so abort.
        break;
    }
    if (count)
        GroupBuilder::Build(brd, groups);
    return count;
}

/** Calls FindPermanentlyInferior() and adds any found to the board
    and the set of inferior cells.x */
std::size_t ICEngine::FillinPermanentlyInferior(Groups& groups, 
                                         PatternState& pastate,
                                         HexColor color, InferiorCells& out, 
                                         HexColorSet colors_to_capture) const
{
    if (!m_find_permanently_inferior) 
        return 0;
    if (!HexColorSetUtil::InSet(color, colors_to_capture)) 
        return 0;
    StoneBoard& brd = groups.Board();
    bitset_t carrier;
    bitset_t perm = FindPermanentlyInferior(pastate, color, brd.GetEmpty(), 
                                            carrier);
    if (perm.any())
    {
        out.AddPermInf(color, perm, carrier);
        brd.AddColor(color, perm);
        pastate.Update(perm);
        GroupBuilder::Build(brd, groups);
    }
    return perm.count();
}

/** Calls FindMutualFillin() and adds any found to the board and the
    set of inferior cells.x */
std::size_t ICEngine::FillInMutualFillin(Groups& groups, PatternState& pastate,
                                         HexColor color, InferiorCells& out, 
                                         HexColorSet colors_to_capture) const
{
    if (!m_find_mutual_fillin)
        return 0;
    /** Can only use mutual fillin when both colours can be captured. */
    if (!HexColorSetUtil::InSet(BLACK, colors_to_capture) ||
        !HexColorSetUtil::InSet(WHITE, colors_to_capture))
        return 0;
    StoneBoard& brd = groups.Board();
    bitset_t carrier;
    bitset_t mut[BLACK_AND_WHITE];
    FindMutualFillin(pastate, color, brd.GetEmpty(), carrier, mut);
    if (mut[BLACK].any())
    {
        BenzeneAssert(mut[WHITE].any());
        BenzeneAssert((mut[BLACK] & mut[WHITE]).none());
        /** Note: mutual fillin carrier is same for both colors (* cells). */
        out.AddMutualFillin(BLACK, mut[BLACK], carrier);
        brd.AddColor(BLACK, mut[BLACK]);
        pastate.Update(mut[BLACK]);
        out.AddMutualFillin(WHITE, mut[WHITE], carrier);
        brd.AddColor(WHITE, mut[WHITE]);
        pastate.Update(mut[WHITE]);
        GroupBuilder::Build(brd, groups);
    }
    else
        BenzeneAssert(mut[WHITE].none());
    return (mut[BLACK] | mut[WHITE]).count();
}

/** Finds vulnerable cells for color and finds presimplicial pairs
    and fills them in for the other color.  Simplicial stones will
    be added as dead and played to the board as DEAD_COLOR. */
std::size_t ICEngine::FillInVulnerable(HexColor color, Groups& groups, 
                               PatternState& pastate, InferiorCells& inf, 
                               HexColorSet colors_to_capture) const
{
    std::size_t count = 0;
    inf.ClearVulnerable();

    UseGraphTheoryToFindDeadVulnerable(color, groups, pastate, inf);

    // Find vulnerable cells with local patterns--do not ignore the
    // presimplicial cells previously found because a pattern
    // may encode another dominator.
    bitset_t consider = groups.Board().GetEmpty() - inf.Dead();
    FindVulnerable(pastate, color, consider, inf);
    
    // Fill in presimplicial pairs only if we are doing fillin for the
    // other player.
    if (HexColorSetUtil::InSet(!color, colors_to_capture)) 
    {
        bitset_t captured = inf.FindPresimplicialPairs();
        if (captured.any()) 
        {
            inf.AddCaptured(!color, captured);
            groups.Board().AddColor(!color, captured);
            pastate.Update(captured);
            GroupBuilder::Build(groups.Board(), groups);
        }
        count += captured.count();
    }
    return count;
}

/** Calls ComputeDeadRegions() and FindThreeSetCliques() and adds
    fill-in to board and set of inferior cells. */
std::size_t ICEngine::CliqueCutsetDead(Groups& groups, PatternState& pastate,
                                       InferiorCells& out) const
{
    bitset_t notReachable = ComputeDeadRegions(groups);
    if (m_find_three_sided_dead_regions)
        notReachable |= FindThreeSetCliques(groups);
    if (notReachable.any()) 
    {
        out.AddDead(notReachable);
        groups.Board().AddColor(DEAD_COLOR, notReachable);
        pastate.Update(notReachable);
        GroupBuilder::Build(groups.Board(), groups);
    }
    return notReachable.count();
}

void ICEngine::ComputeFillin(HexColor color, Groups& groups, 
                             PatternState& pastate, InferiorCells& out,
                             HexColorSet colors_to_capture) const
{
    out.Clear();
    bool considerCliqueCutset = true;
    while(true)
    {
        std::size_t count;
        int iterations = 0;
        for (;; ++iterations)
        {
            count = 0;
            count += ComputeDeadCaptured(groups, pastate, out,
                                         colors_to_capture);
            count += FillinPermanentlyInferior(groups, pastate, color, out, 
                                               colors_to_capture);
            count += FillinPermanentlyInferior(groups, pastate, !color, out, 
                                               colors_to_capture);
            count += FillInMutualFillin(groups, pastate, color, out, 
                                        colors_to_capture);
            count += FillInMutualFillin(groups, pastate, !color, out, 
                                        colors_to_capture);
            count += FillInVulnerable(!color, groups, pastate, out, 
                                      colors_to_capture);
            count += FillInVulnerable(color, groups, pastate, out, 
                                      colors_to_capture);
            if (0 == count)
                break;
            considerCliqueCutset = true;
        }
        if (m_iterative_dead_regions && considerCliqueCutset)
            count = CliqueCutsetDead(groups, pastate, out);
        if (0 == count)
            break;
        considerCliqueCutset = false;
    }
    if (!m_iterative_dead_regions)
        CliqueCutsetDead(groups, pastate, out);
}

void ICEngine::ComputeInferiorCells(HexColor color, Groups& groups,
                                    PatternState& pastate,
                                    InferiorCells& out) const
{
#ifndef NDEBUG
    BenzeneAssert(groups.Board() == pastate.Board());
    StoneBoard oldBoard(groups.Board());
#endif
    SgTimer timer;

    ComputeFillin(color, groups, pastate, out);

    {
        // Note: We consider vulnerable cells when matching reversible patterns
        //       since the captured pattern applies to the entire carrier, not
        //       just the centre cell of the pattern.
        bitset_t consider = groups.Board().GetEmpty();
        FindReversible(pastate, color, consider, out);
    }

    {
        bitset_t consider = groups.Board().GetEmpty()
                            - out.Vulnerable()
                            - out.Reversible();
        FindDominated(pastate, color, consider, out);
    }

    if (m_backup_opponent_dead) 
    {
        // Play opponent in all empty cells, any dead they created
        // are actually vulnerable to the move played. 
        int found = BackupOpponentDead(color, groups.Board(), pastate, out);
        if (found) {
            LogFine() << "Found " << found 
                      << " cells vulnerable to opponent moves.\n";
        }
    }
    
    LogFine() << "  " << timer.GetTime() << "s to find inferior cells.\n";

#ifndef NDEBUG
    BenzeneAssert(groups.Board().Hash() == oldBoard.Hash());
#endif
}

/** For each empty cell on the board, the move is played with the
    opponent's stone (ie, !color) and the fill-in is computed.  Any
    dead cells in this state are backed-up as vulnerable cells in the
    original state, with the set of captured stones as the
    vulnerable-carrier.  This can be moderately expensive.
        
    @todo Link to the "ice-backup-opp-dead" option, or link it's
    documentation here.  
*/
std::size_t ICEngine::BackupOpponentDead(HexColor color, 
                                         const StoneBoard& board,
                                         PatternState& pastate,
                                         InferiorCells& out) const
{
    StoneBoard brd(board);
    PatternState ps(brd);
    ps.CopyState(pastate);

    bitset_t reversible = out.Reversible();
    bitset_t dominated = out.Dominated();

    std::size_t found = 0;
    for (BitsetIterator p(board.GetEmpty()); p; ++p) 
    {
        brd.StartNewGame();
        brd.SetColor(BLACK, board.GetBlack());
        brd.SetColor(WHITE, board.GetWhite());
        brd.PlayMove(!color, *p);
        ps.Update();
        Groups groups;
        GroupBuilder::Build(brd, groups);

        InferiorCells inf;
        ComputeFillin(color, groups, ps, inf);
        bitset_t filled = inf.Fillin(BLACK) | inf.Fillin(WHITE);

        for (BitsetIterator d(inf.Dead()); d; ++d) 
        {
            /** @todo Add if already vulnerable? */
            if (!out.Vulnerable().test(*d)
                && !reversible.test(*d)
                && !dominated.test(*d)) 
            {
                bitset_t carrier = filled;
                carrier.reset(*d);
                carrier.reset(*p);
                out.AddVulnerable(*d, VulnerableKiller(*p, carrier));
                found++;
            }
        }
    }
    return found;
}

//----------------------------------------------------------------------------

bitset_t ICEngine::FindDead(const PatternState& pastate,
                            const bitset_t& consider) const
{
    return pastate.MatchOnBoard(consider, m_patterns.HashedDead());
}

bitset_t ICEngine::FindCaptured(const PatternState& pastate, HexColor color, 
                                const bitset_t& consider) const
{
    bitset_t captured;
    for (BitsetIterator p(consider); p; ++p) 
    {
        if (captured.test(*p)) 
            continue;
        PatternHits hits;
        pastate.MatchOnCell(m_patterns.HashedCaptured(color), *p,
                            PatternState::STOP_AT_FIRST_HIT, hits);
        // Mark carrier as captured if carrier does not intersect the
        // set of captured cells found in this pass. 
        if (!hits.empty()) 
        {
            BenzeneAssert(hits.size() == 1);
            const std::vector<HexPoint>& moves = hits[0].Moves2();
            bitset_t carrier;
            for (unsigned i = 0; i < moves.size(); ++i)
                carrier.set(moves[i]);
            carrier.set(*p);
            if ((carrier & captured).none())
                captured |= carrier;
        }
    }
    return captured;
}

bitset_t ICEngine::FindPermanentlyInferior(const PatternState& pastate, 
                                           HexColor color, 
                                           const bitset_t& consider,
                                           bitset_t& carrier) const
{
    std::vector<PatternHits> hits(FIRST_INVALID);
    bitset_t ret = pastate.MatchOnBoard(consider, 
                                 m_patterns.HashedPermInf(color), 
                                 PatternState::STOP_AT_FIRST_HIT, hits);
    for (BitsetIterator p(ret); p; ++p) 
    {
        BenzeneAssert(hits[*p].size() == 1);
        const std::vector<HexPoint>& moves = hits[*p][0].Moves2();
        for (unsigned i=0; i<moves.size(); ++i)
            carrier.set(moves[i]);
    }
    return ret;
}

void ICEngine::FindMutualFillin(const PatternState& pastate, 
                                HexColor color, 
                                const bitset_t& consider,
                                bitset_t& carrier,
                                bitset_t *mut) const
{
    bitset_t altered;
    for (BitsetIterator p(consider); p; ++p) 
    {
        PatternHits hits;
        pastate.MatchOnCell(m_patterns.HashedMutualFillin(color), *p,
                            PatternState::STOP_AT_FIRST_HIT, hits);
        if (hits.empty())
            continue;

        /** Ensure this mutual fillin pattern does not interfere with
            any other already-added mutual fillin.
         */
        BenzeneAssert(hits.size() == 1);
        bitset_t willAlter;
        willAlter.set(*p);
        const std::vector<HexPoint>& moves1 = hits[0].Moves1();
        for (unsigned i = 0; i < moves1.size(); ++i)
                willAlter.set(moves1[i]);
        const std::vector<HexPoint>& moves2 = hits[0].Moves2();
        for (unsigned i = 0; i < moves2.size(); ++i)
                willAlter.set(moves2[i]);
        if ((willAlter & altered).any())
            continue;

        /** The mutual fillin can be added. */
        altered |= willAlter;
        carrier.set(*p);
        for (unsigned i = 0; i < moves1.size(); ++i)
            mut[color].set(moves1[i]);
        for (unsigned i = 0; i < moves2.size(); ++i)
            mut[!color].set(moves2[i]);
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

    // Add the new vulnerable cells with their killers
    for (BitsetIterator p(vul); p; ++p) 
    {
        for (unsigned j=0; j<hits[*p].size(); ++j) 
        {
            const std::vector<HexPoint>& moves1 = hits[*p][j].Moves1();
            BenzeneAssert(moves1.size() == 1);
            HexPoint killer = moves1[0];
            bitset_t carrier;
            const std::vector<HexPoint>& moves2 = hits[*p][j].Moves2();
            for (unsigned i=0; i<moves2.size(); ++i) {
                carrier.set(moves2[i]);
            }
            inf.AddVulnerable(*p, VulnerableKiller(killer, carrier));
        }
    }
}

void ICEngine::FindReversible(const PatternState& pastate, HexColor color, 
                              const bitset_t& consider,
                              InferiorCells& inf) const
{
    PatternState::MatchMode matchmode = PatternState::STOP_AT_FIRST_HIT;
    if (m_find_all_pattern_reversers)
        matchmode = PatternState::MATCH_ALL;

    // Find reversers using patterns
    std::vector<PatternHits> hits(FIRST_INVALID);
    bitset_t rev = pastate.MatchOnBoard(consider,
                                        m_patterns.HashedReversible(color),
                                        matchmode, hits);
    // Add the new reversible cells with their reversers
    for (BitsetIterator p(rev); p; ++p) 
    {
        for (unsigned j=0; j<hits[*p].size(); ++j) 
        {
            const std::vector<HexPoint>& moves1 = hits[*p][j].Moves1();
            BenzeneAssert(moves1.size() == 1);
            HexPoint reverser = moves1[0];
            // Carriers are mandatory for reversible patterns;
            // otherwise cannot check for independence
            BenzeneAssert(hits[*p][j].Moves2().size() != 0);
            bitset_t carrier;
            const std::vector<HexPoint>& moves2 = hits[*p][j].Moves2();
            for (unsigned i=0; i<moves2.size(); ++i) {
                carrier.set(moves2[i]);
            }
            inf.AddReversible(*p, carrier, reverser);
        }
    }
}

void ICEngine::FindDominated(const PatternState& pastate, HexColor color, 
                             const bitset_t& consider,
                             InferiorCells& inf) const
{
    PatternState::MatchMode matchmode = PatternState::STOP_AT_FIRST_HIT;
    if (m_find_all_pattern_dominators)
        matchmode = PatternState::MATCH_ALL;

    // Find dominators using patterns
    std::vector<PatternHits> hits(FIRST_INVALID);
    bitset_t dom = pastate.MatchOnBoard(consider,
                                        m_patterns.HashedDominated(color),
                                        matchmode, hits);
    // Add the new dominated cells with their dominators
    for (BitsetIterator p(dom); p; ++p) 
    {
        for (unsigned j = 0; j < hits[*p].size(); ++j) 
        {
            const std::vector<HexPoint>& moves1 = hits[*p][j].Moves1();
            BenzeneAssert(moves1.size() == 1);
            inf.AddDominated(*p, moves1[0]);
            // For now, no dominated patterns have carriers
            // Note: this can change in the future if more complex ICE
            // patterns are found
            BenzeneAssert(hits[*p][j].Moves2().size() == 0);
        }
    }
    // Add dominators found via hand coded patterns
    if (m_use_handcoded_patterns)
        FindHandCodedDominated(pastate.Board(), color, consider, inf);
}

void ICEngine::FindDominatedOnCell(const PatternState& pastate,
                                   HexColor color, 
                                   HexPoint cell,
                                   PatternHits& hits) const
{
    PatternState::MatchMode matchmode = PatternState::MATCH_ALL;
    pastate.MatchOnCell(m_patterns.HashedDominated(color), cell,
                        matchmode, hits);
}

void ICEngine::FindHandCodedDominated(const StoneBoard& board, 
                                      HexColor color,
                                      const bitset_t& consider, 
                                      InferiorCells& inf) const
{
    // If board is rectangular, these hand-coded patterns should not
    // be used because they need to be mirrored (which is not a valid
    // operation on non-square boards).
    if (board.Width() != board.Height()) 
        return;
    for (unsigned i=0; i<m_hand_coded.size(); ++i)
        CheckHandCodedDominates(board, color, m_hand_coded[i], 
                                consider, inf);
}

/** Handles color flipping/rotations for this hand-coded pattern.  If
    pattern matches, dominators are added to inf. */
void ICEngine::CheckHandCodedDominates(const StoneBoard& brd,
                                       HexColor color,
                                       const HandCodedPattern& pattern, 
                                       const bitset_t& consider, 
                                       InferiorCells& inf) const
{
    if (brd.Width() < 4 || brd.Height() < 3)
        return;
    HandCodedPattern pat(pattern);
    // Mirror and flip colors if checking for white
    if (color == WHITE) 
    {
        pat.mirror(brd.Const());
        pat.flipColors();
    }
    // Top corner
    if (consider.test(pat.dominatee()) && pat.check(brd))
        inf.AddDominated(pat.dominatee(), pat.dominator());
    // Bottom corner
    pat.rotate(brd.Const());
    if (consider.test(pat.dominatee()) && pat.check(brd))
        inf.AddDominated(pat.dominatee(), pat.dominator());
}

//----------------------------------------------------------------------------

void IceUtil::Update(InferiorCells& out, const InferiorCells& in)
{
    // Overwrite old vulnerable/dominated with new vulnerable/dominated 
    out.ClearVulnerable();
    out.ClearReversible();
    out.ClearDominated();
    out.AddVulnerableFrom(in);
    out.AddReversibleFrom(in);
    out.AddDominatedFrom(in);

    // Add the new fillin to the old fillin
    for (BWIterator c; c; ++c) 
    {
        out.AddCaptured(*c, in.Captured(*c));
        out.AddPermInfFrom(*c, in);
        out.AddMutualFillinFrom(*c, in);
    }

    // Add the new dead cells.
    out.AddDead(in.Dead());
}

//----------------------------------------------------------------------------
