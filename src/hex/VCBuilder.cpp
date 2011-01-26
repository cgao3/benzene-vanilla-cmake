//----------------------------------------------------------------------------
/** @file VCBuilder.cpp */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "SgTimer.h"

#include "Hex.hpp"
#include "BitsetIterator.hpp"
#include "ChangeLog.hpp"
#include "Misc.hpp"
#include "VCBuilder.hpp"
#include "VCSet.hpp"
#include "VCPattern.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

VCBuilderParam::VCBuilderParam()
    : max_ors(4),
      and_over_edge(false),
      use_patterns(true),
      use_non_edge_patterns(true),
      use_greedy_union(true),
      abort_on_winning_connection(false)
{
}

//----------------------------------------------------------------------------

VCBuilder::VCBuilder(VCBuilderParam& param)
    : m_orRule(*this), 
      m_param(param)
{
    LoadCapturedSetPatterns();
}

VCBuilder::~VCBuilder()
{
}

void VCBuilder::LoadCapturedSetPatterns()
{
    std::ifstream inFile;
    try {
        std::string file = MiscUtil::OpenFile("vc-captured-set.txt", inFile);
        LogConfig() << "VCBuilder: reading captured set patterns from '" 
                    << file << "'.\n";
    }
    catch (BenzeneException& e) {
        throw BenzeneException() << "VCBuilder: " << e.what();
    }
    std::vector<Pattern> patterns;
    Pattern::LoadPatternsFromStream(inFile, patterns);
    LogConfig() << "VCBuilder:: parsed " << patterns.size() << " patterns.\n";
    for (std::size_t i = 0; i < patterns.size(); ++i)
    {
        m_capturedSetPatterns[WHITE].push_back(patterns[i]);
        patterns[i].FlipColors();
        m_capturedSetPatterns[BLACK].push_back(patterns[i]);
    }
    for (BWIterator c; c; ++c) 
        m_hash_capturedSetPatterns[*c].Hash(m_capturedSetPatterns[*c]);
}

//----------------------------------------------------------------------------

// Static VC construction

void VCBuilder::Build(VCSet& con, const Groups& groups, 
                      const PatternState& patterns)
{
    SgTimer timer;
    m_con = &con;
    m_color = con.Color();
    m_groups = &groups;
    m_brd = &m_groups->Board();
    m_log = 0;
    m_con->Clear();
    m_statistics = &m_statsForColor[m_color];
    m_queue.Clear();

    ComputeCapturedSets(patterns);
    AddBaseVCs();
    if (m_param.use_patterns)
        AddPatternVCs();
    DoSearch();

    LogFine() << "  " << timer.GetTime() << "s to build vcs.\n";
}

/** Computes the 0-connections defined by adjacency.*/
void VCBuilder::AddBaseVCs()
{
    HexColorSet not_other = HexColorSetUtil::ColorOrEmpty(m_color);
    for (GroupIterator x(*m_groups, not_other); x; ++x) 
    {
        for (BitsetIterator y(x->Nbs() & m_brd->GetEmpty()); y; ++y) 
        {
            VC vc(x->Captain(), *y);
            m_statistics->base_attempts++;
            if (m_con->Add(vc, m_log))
            {
                m_statistics->base_successes++;
                m_queue.Push(std::make_pair(vc.X(), vc.Y()));
            }
        }
    }
}

/** Adds vcs obtained by pre-computed patterns. */
void VCBuilder::AddPatternVCs()
{
    const VCPatternSet& patterns 
        = VCPattern::GetPatterns(m_brd->Width(), m_brd->Height(), m_color);
    for (std::size_t i=0; i<patterns.size(); ++i) 
    {
        const VCPattern& pat = patterns[i];
        if (!m_param.use_non_edge_patterns
            && !HexPointUtil::isEdge(pat.Endpoint(0))
            && !HexPointUtil::isEdge(pat.Endpoint(1)))
            continue;
        if (pat.Matches(m_color, *m_brd)) 
        {
            bitset_t carrier = pat.NotOpponent() - m_brd->GetColor(m_color);
            carrier.reset(pat.Endpoint(0));
            carrier.reset(pat.Endpoint(1));
            VC vc(pat.Endpoint(0), pat.Endpoint(1), carrier, VC_RULE_BASE);

            m_statistics->pattern_attempts++;
            if (m_con->Add(vc, m_log))
            {
                m_statistics->pattern_successes++;
                m_queue.Push(std::make_pair(vc.X(), vc.Y()));
            }
        }
    }
}

void VCBuilder::ComputeCapturedSets(const PatternState& patterns)
{
    SG_UNUSED(patterns);
    for (BoardIterator p(m_brd->Const().EdgesAndInterior()); p; ++p)
    {
        m_capturedSet[*p] = EMPTY_BITSET;
        if (m_brd->GetColor(*p) == EMPTY)
        {
            PatternHits hits;
            patterns.MatchOnCell(m_hash_capturedSetPatterns[m_color],
                                 *p, PatternState::STOP_AT_FIRST_HIT, hits);
            for (std::size_t i = 0; i < hits.size(); ++i)
            {
                const std::vector<HexPoint>& moves = hits[0].Moves2();
                bitset_t carrier;
                for (std::size_t j = 0; j < moves.size(); ++j)
                    m_capturedSet[*p].set(moves[j]);
            }
        }
    }
}

//----------------------------------------------------------------------------
// Incremental VC construction

void VCBuilder::Build(VCSet& con, const Groups& oldGroups,
                      const Groups& newGroups, const PatternState& patterns,
                      bitset_t added[BLACK_AND_WHITE], ChangeLog<VC>* log)
{
    BenzeneAssert((added[BLACK] & added[WHITE]).none());
    SgTimer timer;
    m_con = &con;
    m_color = con.Color();
    m_groups = &newGroups;
    m_brd = &m_groups->Board();
    m_log = log;
    m_statistics = &m_statsForColor[m_color];
    m_queue.Clear();

    ComputeCapturedSets(patterns);
    Merge(oldGroups, added);
    if (m_param.use_patterns)
        AddPatternVCs();
    DoSearch();

    LogFine() << "  " << timer.GetTime() << "s to build vcs incrementally.\n" ;
}

/** @page mergeshrink Incremental Update Algorithm
    
    The connection set is updated to the new state of the board in a
    single pass. In this pass connections touched by opponent stones
    are destroyed, connections touched by friendly stones are resized,
    and connections in groups that are merged into larger groups are
    merged into the proper connection lists. This entire process is
    called the "merge".
    
    The merge begins by noting the set of "affected" stones. These are
    the stones that were just played as well as those groups adjacent
    to the played stones.

    Any list with either endpoint in the affected set will need to
    either pass its connections to the list now responsible for that
    group, or recieve connections from other lists that it is now
    responsible for. Lists belonging to groups that are merged into
    other groups are not destroyed, they remain so that undoing this
    merge is more efficient.

    Every list needs to be checked for shrinking. This entails
    removing any cells from a connection's carrier that are now
    occupied by friendly stones. Semi-connections that have their keys
    played must be upgraded to full connections. */
void VCBuilder::Merge(const Groups& oldGroups, bitset_t added[BLACK_AND_WHITE])
{
    // Kill connections containing stones the opponent just played.
    // NOTE: This *must* be done in the original state, not in the
    // state with the newly added stones. If we are adding stones of
    // both colors there could be two groups of our stones that are
    // going to be merged, but we need to kill connections touching
    // the opponent stones before we do so. 
    RemoveAllContaining(oldGroups, added[!m_color]);
        
    // Find groups adjacent to any played stone of color; add them to
    // the affected set along with the played stones.
    bitset_t affected = added[m_color];
    for (BitsetIterator x(added[m_color]); x; ++x)
    {
        for (BoardIterator y(m_brd->Const().Nbs(*x)); y; ++y)
        {
            const Group& grp = oldGroups.GetGroup(*y);
            if (grp.Color() == m_color)
                affected.set(grp.Captain());
        }
    }
    MergeAndShrink(affected, added[m_color]);
}

void VCBuilder::MergeAndShrink(const bitset_t& affected,
                               const bitset_t& added)
{
    HexColorSet not_other = HexColorSetUtil::NotColor(!m_color);
    for (BoardIterator x(m_brd->Stones(not_other)); x; ++x) 
    {
        if (!m_groups->IsCaptain(*x) && !affected.test(*x)) 
            continue;
        for (BoardIterator y(m_brd->Stones(not_other)); *y != *x; ++y) 
        {
            if (!m_groups->IsCaptain(*y) && !affected.test(*y)) 
                continue;
            HexPoint cx = m_groups->CaptainOf(*x);
            HexPoint cy = m_groups->CaptainOf(*y);
            // Lists between (cx, cx) are never used, so only do work
            // if it's worthwhile. This can occur if y was recently
            // played next to group x, now they both have the same
            // captain, so no point merging old connections into
            // (captain, captain).
            if (cx != cy) 
            {
                m_queue.Push(std::make_pair(cx, cy));
                MergeAndShrink(added, *x, *y, cx, cy);
            }
        }
    }
}

/** Merges and shrinks connections between the given endpoints.
    @bug It is possible that we end up with semi connections that are
    supersets of full connections due to the shrinking.  These are
    rare and unimportant and the cost of checking for them exceeds any
    gain we get from removing them.
 */
void VCBuilder::MergeAndShrink(const bitset_t& added, 
                               HexPoint xin, HexPoint yin,
                               HexPoint xout, HexPoint yout)
{
    BenzeneAssert(xin != yin);
    BenzeneAssert(xout != yout);

    VCList* fullsIn = &m_con->GetList(VC::FULL, xin, yin);
    VCList* semisIn = &m_con->GetList(VC::SEMI, xin, yin);
    VCList* fullsOut= &m_con->GetList(VC::FULL, xout, yout);
    VCList* semisOut= &m_con->GetList(VC::SEMI, xout, yout);
    BenzeneAssert((fullsIn == fullsOut) == (semisIn == semisOut));
    bool doingMerge = (fullsIn != fullsOut);

    // Shrink all 0-connections.
    {
        std::list<VC> removed;
        fullsIn->RemoveAllContaining(added, removed, m_log);
        if (doingMerge) 
            fullsOut->Add(*fullsIn, m_log);
        for (std::list<VC>::iterator it = removed.begin(); 
             it != removed.end(); ++it) 
        {
            VC v = VC::ShrinkFull(*it, added, xout, yout);
            if (fullsOut->Add(v, m_log))
                m_statistics->shrunk0++;
        }
    }

    // Shrink all 1-connections.
    std::list<VC> removed;
    semisIn->RemoveAllContaining(added, removed, m_log);
    if (doingMerge)
    {   
        // BUG: These could be supersets of fullsOut.
        semisOut->Add(*semisIn, m_log);
    }
    // Shrink connections that touch played cells.
    // Do not upgrade during this step.
    std::list<VC>::iterator it;
    for (it = removed.begin(); it != removed.end(); ++it) 
    {
        if (!added.test(it->Key())) 
        {
            VC v = VC::ShrinkSemi(*it, added, xout, yout);
            // BUG: These could be supersets of fullsOut.
            if (semisOut->Add(v, m_log))
                m_statistics->shrunk1++;
        }
    }
    // Upgrade semis. Need to do this after shrinking to ensure
    // that we remove all sc supersets from semisOut.
    for (it = removed.begin(); it != removed.end(); ++it) 
    {
        if (added.test(it->Key())) 
        {
            VC v = VC::UpgradeSemi(*it, added, xout, yout);
            if (fullsOut->Add(v, m_log))
            {
                // Remove supersets from the semi-list; do not
                // invalidate list intersection since this semi was a
                // member of the list. Actually, this probably doesn't
                // matter since the call to RemoveAllContaining()
                // already clobbered the intersections.
                semisOut->RemoveSuperSetsOf(v.Carrier(), m_log, false);
                m_statistics->upgraded++;
            }
        }
    }
}

/** Removes all connections whose intersection with given set is
    non-empty. Any list that is modified is added to the queue, since
    some unprocessed connections could have been brought under the
    softlimit. */
void VCBuilder::RemoveAllContaining(const Groups& oldGroups,
                                    const bitset_t& bs)
{
    // Use old groupset, but skip old groups that are 
    // now the opponent's color--don't need to do anything for those.
    HexColorSet not_other = HexColorSetUtil::NotColor(!m_color);
    for (GroupIterator x(oldGroups, not_other); x; ++x) 
    { 
        HexPoint xc = x->Captain();
	if (m_groups->GetGroup(xc).Color() == !m_color)
	    continue;
        for (GroupIterator y(oldGroups, not_other); &*y != &*x; ++y) 
        {
            HexPoint yc = y->Captain();
	    if (m_groups->GetGroup(yc).Color() == !m_color)
	        continue;
            std::size_t cur0 = m_con->GetList(VC::FULL, xc, yc)
                .RemoveAllContaining(bs, m_log);
            m_statistics->killed0 += cur0; 
            std::size_t cur1 = m_con->GetList(VC::SEMI, xc, yc)
                .RemoveAllContaining(bs, m_log);
            m_statistics->killed1 += cur1;
            if (cur0 || cur1)
                m_queue.Push(std::make_pair(xc, yc));
        }
    }
}

//----------------------------------------------------------------------------
// VC Construction methods
//----------------------------------------------------------------------------

void VCBuilder::ProcessSemis(HexPoint xc, HexPoint yc)
{
    VCList& semis = m_con->GetList(VC::SEMI, xc, yc);
    VCList& fulls = m_con->GetList(VC::FULL, xc, yc);
    bitset_t capturedSet = m_capturedSet[xc] | m_capturedSet[yc];
    bitset_t uncapturedSet = capturedSet;
    uncapturedSet.flip();
    // Nothing to do, so abort. 
    if ((semis.HardIntersection() & uncapturedSet).any())
        return;
    std::list<VC> added;
    for (VCListIterator cur(semis, semis.Softlimit()); cur; ++cur) 
    {
        if (!cur->Processed()) 
        {
            m_statistics->doOrs++;
            if (m_orRule(*cur, &semis, &fulls, added, m_param.max_ors, 
                         m_log, *m_statistics))
            {
                m_statistics->goodOrs++;
            }
            cur->SetProcessed(true);
            if (m_log)
                m_log->Push(ChangeLog<VC>::PROCESSED, *cur);
        }
    }
    // If no full exists, create one by unioning the entire list
    if (fulls.Empty()) 
    {
        bitset_t carrier = m_param.use_greedy_union 
            ? semis.GetGreedyUnion() 
            : semis.GetUnion();
        fulls.Add(VC(xc, yc, carrier | capturedSet, VC_RULE_ALL), m_log);
        // NOTE: No need to remove supersets of v from the semi
        // list since there can be none!
    } 
}

void VCBuilder::ProcessFulls(HexPoint xc, HexPoint yc)
{
    VCList& fulls = m_con->GetList(VC::FULL, xc, yc);
    for (VCListIterator cur(fulls, fulls.Softlimit()); cur; ++cur) 
    {
        if (!cur->Processed()) 
        {
            AndClosure(*cur);
            cur->SetProcessed(true);
            if (m_log)
                m_log->Push(ChangeLog<VC>::PROCESSED, *cur);
        }
    }
}

void VCBuilder::DoSearch()
{
    bool winning_connection = false;
    while (!m_queue.Empty()) 
    {
        HexPointPair pair = m_queue.Front();
        m_queue.Pop();
        ProcessSemis(pair.first, pair.second);
        ProcessFulls(pair.first, pair.second);
        if (m_param.abort_on_winning_connection && 
            m_con->Exists(HexPointUtil::colorEdge1(m_color),
                          HexPointUtil::colorEdge2(m_color),
                          VC::FULL))
        {
            winning_connection = true;
            break;
        }
    }        
    BenzeneAssert(winning_connection || m_queue.Empty());
    if (winning_connection) 
        LogFine() << "Aborted on winning connection.\n";
    // Process the side-to-side semi list to ensure we have a full if
    // mustplay is empty.
    // TODO: IS THIS STILL NEEDED?
    ProcessSemis(m_groups->CaptainOf(HexPointUtil::colorEdge1(m_color)),
                 m_groups->CaptainOf(HexPointUtil::colorEdge2(m_color)));
}

//----------------------------------------------------------------------------

/** Computes the and closure for the vc. Let x and y be vc's
    endpoints. A single pass over the board is performed. For each z,
    we try to and the list of fulls between z and x and z and y with
    vc. This function is a major bottleneck. Every operation in it
    needs to be as efficient as possible. */
void VCBuilder::AndClosure(const VC& vc)
{
    HexColor other = !m_color;
    HexColorSet not_other = HexColorSetUtil::NotColor(other);
    HexPoint endp[2];
    endp[0] = m_groups->CaptainOf(vc.X());
    endp[1] = m_groups->CaptainOf(vc.Y());
    HexColor endc[2];
    endc[0] = m_brd->GetColor(endp[0]);
    endc[1] = m_brd->GetColor(endp[1]);
    BenzeneAssert(endc[0] != other);
    BenzeneAssert(endc[1] != other);
    bitset_t vcCapturedSet = m_capturedSet[endp[0]] | m_capturedSet[endp[1]];
    for (GroupIterator g(*m_groups, not_other); g; ++g) 
    {
        HexPoint z = g->Captain();
        if (z == endp[0] || z == endp[1])
            continue;
        if (vc.Carrier().test(z))
            continue;
        bitset_t capturedSet = vcCapturedSet | m_capturedSet[z];
        bitset_t uncapturedSet = capturedSet;
        uncapturedSet.flip();
        for (int i = 0; i < 2; i++)
        {
            int j = (i + 1) & 1;
            if (m_param.and_over_edge || !HexPointUtil::isEdge(endp[i])) 
            {
                VCList* fulls = &m_con->GetList(VC::FULL, z, endp[i]);
                if ((fulls->SoftIntersection() & vc.Carrier()
                     & uncapturedSet).any())
                    continue;
                AndRule rule = (endc[i] == EMPTY) ? CREATE_SEMI : CREATE_FULL;
                DoAnd(z, endp[i], endp[j], rule, vc, capturedSet, 
                      &m_con->GetList(VC::FULL, z, endp[i]));
            }
        }
    }
}

/** Compares vc to each connection in the softlimit of the given list.
    Creates a new connection if intersection is empty, or if the
    intersection is a subset of the captured set. Created connections
    are added with AddNewFull() or AddNewSemi(). */
void VCBuilder::DoAnd(HexPoint from, HexPoint over, HexPoint to,
                      AndRule rule, const VC& vc, const bitset_t& capturedSet, 
                      const VCList* old)
{
    if (old->Empty())
        return;
    for (VCListConstIterator i(*old, old->Softlimit()); i; ++i) 
    {
        if (!i->Processed())
            continue;
        if (i->Carrier().test(to))
            continue;
        bitset_t intersection = i->Carrier() & vc.Carrier();
        if (intersection.none())
        {
            if (rule == CREATE_FULL)
            {
                m_statistics->and_full_attempts++;
                if (AddNewFull(VC::AndVCs(from, to, *i, vc)))
                    m_statistics->and_full_successes++;
            }
            else if (rule == CREATE_SEMI)
            {
                m_statistics->and_semi_attempts++;
                if (AddNewSemi(VC::AndVCs(from, to, *i, vc, over)))
                    m_statistics->and_semi_successes++;
            }
        }
        else if (BitsetUtil::IsSubsetOf(intersection, capturedSet))
        {
            if (rule == CREATE_FULL)
            {
                m_statistics->and_full_attempts++;
                if (AddNewFull(VC::AndVCs(from, to, *i, vc, capturedSet)))
                    m_statistics->and_full_successes++;
            }
            else if (rule == CREATE_SEMI)
            {
                m_statistics->and_semi_attempts++;
                if (AddNewSemi(VC::AndVCs(from, to, *i, vc, capturedSet, over)))
                    m_statistics->and_semi_successes++;
            }
        }
    }
}

/** Runs over all subsets of size 2 to maxOrs of semis containing vc
    and adds the union to out if it has an empty intersection. This
    function is a major bottleneck and so needs to be as efficient as
    possible.

    Subsets are built-up incrementally. If a semi does not make the subsets
    intersection smaller, it is skipped. 

    TODO: Check if unrolling the recursion really does speed it up.

    @return number of connections successfully added.
*/
int VCBuilder::OrRule::operator()(const VC& vc, 
                                  const VCList* semi_list, 
                                  VCList* full_list, 
                                  std::list<VC>& added, 
                                  int maxOrs,
                                  ChangeLog<VC>* log, 
                                  VCBuilderStatistics& stats)
{
    if (semi_list->Empty())
        return 0;
    // Copy processed semis (unprocessed semis are not used here)
    m_semi.clear();
    for (VCListConstIterator it(*semi_list, semi_list->Softlimit()); it; ++it)
        if (it->Processed())
            m_semi.push_back(*it);
    if (m_semi.empty())
        return 0;
    // For each i in [0, N-1], compute intersection of semi[i, N-1]
    std::size_t N = m_semi.size();
    if (m_tail.size() < N)
        m_tail.resize(N);
    m_tail[N-1] = m_semi[N-1].Carrier();
    for (int i = static_cast<int>(N - 2); i >= 0; --i)
        m_tail[i] = m_semi[i].Carrier() & m_tail[i+1];
    maxOrs--;
    BenzeneAssert(maxOrs < 16);
    // Compute the captured-set union for the endpoints of this list
    bitset_t capturedSet = m_builder.m_capturedSet[semi_list->GetX()] 
                         | m_builder.m_capturedSet[semi_list->GetY()];
    bitset_t uncapturedSet = capturedSet;
    uncapturedSet.flip();
    std::size_t index[16];
    bitset_t ors[16];
    bitset_t ands[16];
    ors[0] = vc.Carrier();
    ands[0] = vc.Carrier();
    index[1] = 0;
    int d = 1;
    int count = 0;
    while (true) 
    {
        std::size_t i = index[d];
        // The current intersection (some subset from [0, i-1]) is not
        // disjoint with the intersection of [i, N), so stop. Note that
        // the captured set is not considered in the intersection.
        if ((i < N) && (ands[d-1] & m_tail[i] & uncapturedSet).any())
            i = N;
        if (i == N) 
        {
            if (d == 1) 
                break;
            ++index[--d];
            continue;
        }
        ands[d] = ands[d-1] & m_semi[i].Carrier();
        ors[d]  =  ors[d-1] | m_semi[i].Carrier();
        if (ands[d].none()) 
        {
            // Create a new full.
            // NOTE: We do not use AddNewFull() because if add is
            // successful, it checks for semi-supersets and adds the
            // list to the queue. Both of these operations are not
            // needed here.
            VC v(full_list->GetX(), full_list->GetY(), ors[d], VC_RULE_OR);
            stats.or_attempts++;
            if (full_list->Add(v, log) != VCList::ADD_FAILED) 
            {
                count++;
                stats.or_successes++;
                added.push_back(v);
            }
            ++index[d];
        } 
        else if (BitsetUtil::IsSubsetOf(ands[d], capturedSet))
        {
            // Create a new full.
            // This vc has one or both captured sets in its carrier.
            bitset_t carrier = ors[d];
            if ((ands[d] & m_builder.m_capturedSet[semi_list->GetX()]).any())
                carrier |= m_builder.m_capturedSet[semi_list->GetX()];
            if ((ands[d] & m_builder.m_capturedSet[semi_list->GetY()]).any())
                carrier |= m_builder.m_capturedSet[semi_list->GetY()];
            VC v(full_list->GetX(), full_list->GetY(), carrier, VC_RULE_OR);
            stats.or_attempts++;
            if (full_list->Add(v, log) != VCList::ADD_FAILED) 
            {
                ++count;
                stats.or_successes++;
                added.push_back(v);
            }
            ++index[d];
        }
        else if (ands[d] == ands[d-1]) 
        {
            // This connection does not shrink intersection so skip it
            ++index[d];
        }
        else 
        {
            // This connection reduces intersection, if not at max
            // depth see if more semis can reduce it to the empty set
            // (or at least a subset of the captured set).
            if (d < maxOrs) 
                index[++d] = ++i;
            else
                ++index[d];
        }
    }
    return count;
}

/** Tries to add a new full-connection.  
    If vc is successfully added, then: 1) semi-connections between
    (vc.X(), vc.Y()) that are supersets of vc of removed; and 2), the
    endpoints (vc.X(), vc.Y()) are added to queue if vc was added
    inside the softlimit, signalling that more work needs to be
    performed on this list. */
bool VCBuilder::AddNewFull(const VC& vc)
{
    BenzeneAssert(vc.GetType() == VC::FULL);
    VCList::AddResult result = m_con->Add(vc, m_log);
    if (result != VCList::ADD_FAILED) 
    {
        m_con->GetList(VC::SEMI, vc.X(), vc.Y())
            .RemoveSuperSetsOf(vc.Carrier(), m_log);
        if (result == VCList::ADDED_INSIDE_SOFT_LIMIT)
            m_queue.Push(std::make_pair(vc.X(), vc.Y()));
        return true;
    }
    return false;
}

/** Tries to add a new semi-connection.
        
    Does not add if semi is a superset of some full-connection between
    (vc.x(), and vc.y()).
    
    If add is successfull and intersection on semi-list is empty: if
    semi was added inside soft limit, (vc.x(), vc.y()) is added to
    work queue; otherwise, if no full exists between (vc.x(), vc.y()),
    the entire semi list is combined to form a new full connection.

    This ensures that there is always a full connection whenever the
    intersection the semi-list is empty. */
bool VCBuilder::AddNewSemi(const VC& vc)
{
    VCList* outFull = &m_con->GetList(VC::FULL, vc.X(), vc.Y());
    VCList* outSemi = &m_con->GetList(VC::SEMI, vc.X(), vc.Y());
    if (!outFull->IsSupersetOfAny(vc.Carrier())) 
    {
        VCList::AddResult result = outSemi->Add(vc, m_log);
        if (result != VCList::ADD_FAILED) 
        {
            if (outSemi->HardIntersection().none())
            {
                if (result == VCList::ADDED_INSIDE_SOFT_LIMIT) 
                    m_queue.Push(std::make_pair(vc.X(), vc.Y()));
                else if (outFull->Empty())
                {
                    bitset_t carrier = m_param.use_greedy_union 
                        ? outSemi->GetGreedyUnion() 
                        : outSemi->GetUnion();
                    VC v(outFull->GetX(), outFull->GetY(), 
                         carrier, VC_RULE_ALL);
                    outFull->Add(v, m_log);
                }
            }
            return true;
        }
    }
    return false;
}

//----------------------------------------------------------------------------

std::string VCBuilderStatistics::ToString() const
{
    std::ostringstream os;
    os << "["
       << "base=" << base_successes << "/" << base_attempts << '\n'
       << "pat=" << pattern_successes << "/" << pattern_attempts << '\n'
       << "and-f=" << and_full_successes << "/" << and_full_attempts << '\n'
       << "and-s=" << and_semi_successes << "/" << and_semi_attempts << '\n'
       << "or=" << or_successes << "/" << or_attempts << '\n'
       << "doOr()=" << goodOrs << "/" << doOrs << '\n'
       << "s0/s1/u1=" << shrunk0 << "/" << shrunk1 << "/"<< upgraded << '\n'
       << "killed0/1=" << killed0 << "/" << killed1 << '\n'
       << "]";
    return os.str();
}

//----------------------------------------------------------------------------

/** @page workqueue VCBuilder Work Queue

    WorkQueue stores the endpoints of any VCLists that need further
    processing. Endpoints are pushed onto the back of the queue and
    popped off the front, in FIFO order. It also ensures only unique
    elements are added; that is, a list is added only once until it is
    processed.

    The implementation here is a simple vector with an index
    simulating the front of the queue; that is, Push() uses
    push_back() to add elements to the back and Pop() increments the
    index of the front. This means the vector will need to be as large
    as the number of calls to Push(), not the maximum number of
    elements in the queue at any given time.
    
    On 11x11, the vector quickly grows to hold 2^14 elements if anding
    over the edge, and 2^13 if not. Since only unique elements are
    added, in the worst case this value will be the smallest n such
    that 2^n > xy, where x and y are the width and height of the
    board.

    This implementation was chosen for efficiency: a std::deque uses
    dynamic memory, and so every push()/pop() requires at least one
    call to malloc/free. The effect is small, but can be as
    significant as 1-3% of the total run-time, especially on smaller
    boards.
*/
VCBuilder::WorkQueue::WorkQueue()
    : m_head(0), 
      m_array(128)
{
}

bool VCBuilder::WorkQueue::Empty() const
{
    return m_head == m_array.size();
}

const HexPointPair& VCBuilder::WorkQueue::Front() const
{
    return m_array[m_head];
}

std::size_t VCBuilder::WorkQueue::Capacity() const
{
    return m_array.capacity();
}

void VCBuilder::WorkQueue::Clear()
{
    memset(m_seen, 0, sizeof(m_seen));
    m_array.clear();
    m_head = 0;
}

void VCBuilder::WorkQueue::Pop()
{
    m_seen[Front().first][Front().second] = false;
    m_head++;
}

void VCBuilder::WorkQueue::Push(const HexPointPair& p)
{
    HexPoint a = std::min(p.first, p.second);
    HexPoint b = std::max(p.first, p.second);
    if (!m_seen[a][b]) 
    {
        m_seen[a][a] = true;
        m_array.push_back(std::make_pair(a, b));
    }
}

//----------------------------------------------------------------------------
