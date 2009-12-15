//----------------------------------------------------------------------------
/** @file VCBuilder.cpp
 */
//----------------------------------------------------------------------------

#include "Hex.hpp"
#include "Time.hpp"
#include "BitsetIterator.hpp"
#include "GraphUtils.hpp"
#include "ChangeLog.hpp"
#include "VCBuilder.hpp"
#include "VCSet.hpp"
#include "VCPattern.hpp"
#include "VCUtils.hpp"

#include <boost/filesystem/path.hpp>

using namespace benzene;

//----------------------------------------------------------------------------

VCBuilderParam::VCBuilderParam()
    : max_ors(4),
      and_over_edge(false),
      use_patterns(true),
      use_non_edge_patterns(true),
      use_crossing_rule(false),
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
    using namespace boost::filesystem;
    path filename = path(ABS_TOP_SRCDIR) / "share" / "vc-captured-set.txt";
    filename.normalize();

    std::vector<Pattern> patterns;
    Pattern::LoadPatternsFromFile(filename.native_file_string().c_str(), 
                                  patterns);

    LogFine() << "--LoadCapturedSetPatterns()\n";    
    LogFine() << "Read " << patterns.size() << " patterns.\n";
    for (std::size_t i = 0; i < patterns.size(); ++i)
    {
        m_capturedSetPatterns[WHITE].push_back(patterns[i]);
        patterns[i].flipColors();
        m_capturedSetPatterns[BLACK].push_back(patterns[i]);
    }
    for (BWIterator c; c; ++c) 
        m_hash_capturedSetPatterns[*c].hash(m_capturedSetPatterns[*c]);
}

//----------------------------------------------------------------------------

// Static VC construction

void VCBuilder::Build(VCSet& con, const Groups& groups, 
                      const PatternState& patterns)
{
    m_con = &con;
    m_color = con.Color();
    m_groups = &groups;
    m_brd = &m_groups->Board();
    m_log = 0;

    double s = Time::Get();
    m_con->Clear();
    m_statistics = &m_statsForColor[m_color];
    m_queue.clear();

    ComputeCapturedSets(patterns);
    AddBaseVCs();
    if (m_param.use_patterns)
        AddPatternVCs();
    DoSearch();

    double e = Time::Get();
    LogFine() << "  " << (e-s) << "s to build vcs.\n";
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
                m_queue.push(std::make_pair(vc.x(), vc.y()));
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
                m_queue.push(std::make_pair(vc.x(), vc.y()));
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
                const std::vector<HexPoint>& moves = hits[0].moves2();
                bitset_t carrier;
                for (std::size_t j = 0; j < moves.size(); ++j)
                    m_capturedSet[*p].set(moves[j]);
                //LogInfo() << "Captured " << *p
                //          << m_brd->Write(m_capturedSet[*p]) << '\n';
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
    HexAssert((added[BLACK] & added[WHITE]).none());

    m_con = &con;
    m_color = con.Color();
    m_groups = &newGroups;
    m_brd = &m_groups->Board();
    m_log = log;

    double s = Time::Get();
    m_statistics = &m_statsForColor[m_color];
    m_queue.clear();

    ComputeCapturedSets(patterns);
    Merge(oldGroups, added);
    if (m_param.use_patterns)
        AddPatternVCs();
    DoSearch();

    double e = Time::Get();
    LogFine() << "  " << (e-s) << "s to build vcs incrementally.\n" ;
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
    either pass on its connections to the list now responsible for
    that group or recieve connections from other lists that it is now
    responsible for. Lists belonging to groups that are merge into
    other groups are not destroyed, they remain so that undoing this
    merge is more efficient.

    Every list needs to be checked for shrinking.
    
    TODO Finish this documentation!
            
*/
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
        for (BoardIterator y(m_brd->Const().Nbs(*x)); y; ++y)
        {
            const Group& grp = oldGroups.GetGroup(*y);
            if (grp.Color() == m_color)
                affected.set(grp.Captain());
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
                m_queue.push(std::make_pair(cx, cy));
                MergeAndShrink(added, *x, *y, cx, cy);
            }
        }
    }
}

void VCBuilder::MergeAndShrink(const bitset_t& added, 
                               HexPoint xin, HexPoint yin,
                               HexPoint xout, HexPoint yout)
{
    HexAssert(xin != yin);
    HexAssert(xout != yout);

    VCList* fulls_in = &m_con->GetList(VC::FULL, xin, yin);
    VCList* semis_in = &m_con->GetList(VC::SEMI, xin, yin);
    VCList* fulls_out= &m_con->GetList(VC::FULL, xout, yout);
    VCList* semis_out= &m_con->GetList(VC::SEMI, xout, yout);

    HexAssert((fulls_in == fulls_out) == (semis_in == semis_out));
    bool doing_merge = (fulls_in != fulls_out);

    std::list<VC> removed;
    std::list<VC>::iterator it;

    // 
    // Shrink all 0-connections.
    //
    // If (doing_merge) transfer remaining connections over as well. 
    //
    fulls_in->removeAllContaining(added, removed, m_log);
    if (doing_merge) 
    { 
        // Copied vc's will be set to unprocessed explicitly.
        /** @bug There could be supersets of these fulls in semis_out! */
        fulls_out->add(*fulls_in, m_log);
    }

    for (it = removed.begin(); it != removed.end(); ++it) 
    {
        VC v = VC::ShrinkFull(*it, added, xout, yout);
        /** @bug There could be supersets of these fulls in semis_out! */
        if (fulls_out->add(v, m_log))
            m_statistics->shrunk0++;
    }

    //
    // Shrink all 1-connections.
    // if (doing_merge) transfer remaining connections
    // over as well. 
    //
    removed.clear();
    semis_in->removeAllContaining(added, removed, m_log);
    if (doing_merge) 
    {
        // Copied vc's will be set to unprocessed explicitly.
        /** @bug These could be supersets of fulls_out. */
        semis_out->add(*semis_in, m_log);
    }

    // Shrink connections that touch played cells.
    // Do not upgrade during this step. 
    for (it = removed.begin(); it != removed.end(); ++it) 
    {
        if (!added.test(it->key())) 
        {
            VC v = VC::ShrinkSemi(*it, added, xout, yout);
            /** @bug These could be supersets of fulls_out. */
            if (semis_out->add(v, m_log))
                m_statistics->shrunk1++;
        }
    }

    // Upgrade semis. Need to do this after shrinking to ensure
    // that we remove all sc supersets from semis_out.
    for (it = removed.begin(); it != removed.end(); ++it) 
    {
        if (added.test(it->key())) 
        {
            VC v = VC::UpgradeSemi(*it, added, xout, yout);
            if (fulls_out->add(v, m_log))
            {
                // Remove supersets from the semi-list; do not
                // invalidate list intersection since this semi was a
                // member of the list. Actually, this probably doesn't
                // matter since the call to removeAllContaining()
                // already clobbered the intersections.
                semis_out->removeSuperSetsOf(v.carrier(), m_log, false);
                m_statistics->upgraded++;
            }
        }
    }
}

/** Removes all connections whose intersection with given set is
    non-empty. Any list that is modified is added to the queue, since
    some unprocessed connections could have been brought under the
    softlimit.
*/
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
            int cur0 = m_con->GetList(VC::FULL, xc, yc)
                .removeAllContaining(bs, m_log);
            m_statistics->killed0 += cur0; 
            int cur1 = m_con->GetList(VC::SEMI, xc, yc)
                .removeAllContaining(bs, m_log);
            m_statistics->killed1 += cur1;
            if (cur0 || cur1)
                m_queue.push(std::make_pair(xc, yc));
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
    if ((semis.hardIntersection() & uncapturedSet).any())
        return;

    int soft = semis.softlimit();
    VCList::iterator cur = semis.begin();
    VCList::iterator end = semis.end();
    std::list<VC> added;

    for (int count=0; count<soft && cur!=end; ++cur, ++count) 
    {
        if (!cur->processed()) 
        {
            if (m_param.use_crossing_rule)
                doCrossingRule(*cur, &semis);

            m_statistics->doOrs++;
            if (m_orRule(*cur, &semis, &fulls, added, m_param.max_ors, 
                         m_log, *m_statistics))
            {
                m_statistics->goodOrs++;
            }

            cur->setProcessed(true);
            
            if (m_log)
                m_log->push(ChangeLog<VC>::PROCESSED, *cur);
        }
    }

    // If no full exists, create one by unioning the entire list
    if (fulls.empty()) 
    {
        bitset_t carrier = m_param.use_greedy_union 
            ? semis.getGreedyUnion() 
            : semis.getUnion();

        fulls.add(VC(xc, yc, carrier | capturedSet, EMPTY_BITSET, VC_RULE_ALL),
                  m_log);
        // @note No need to remove supersets of v from the semi
        // list since there can be none!
    } 
}

void VCBuilder::ProcessFulls(HexPoint xc, HexPoint yc)
{
    VCList& fulls = m_con->GetList(VC::FULL, xc, yc);
    int soft = fulls.softlimit();
    VCList::iterator cur = fulls.begin();
    VCList::iterator end = fulls.end();
    for (int count=0; count<soft && cur!=end; ++cur, ++count) 
    {
        if (!cur->processed()) 
        {
            andClosure(*cur);
            cur->setProcessed(true);
            if (m_log)
                m_log->push(ChangeLog<VC>::PROCESSED, *cur);
        }
    }
}

void VCBuilder::DoSearch()
{
    bool winning_connection = false;
    while (!m_queue.empty()) 
    {
        HexPointPair pair = m_queue.front();
        m_queue.pop();

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
    HexAssert(winning_connection || m_queue.empty());

    if (winning_connection) 
        LogFine() << "Aborted on winning connection." << '\n';
            
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
    needs to be as efficient as possible.
*/
void VCBuilder::andClosure(const VC& vc)
{
    HexColor other = !m_color;
    HexColorSet not_other = HexColorSetUtil::NotColor(other);

    HexPoint endp[2];
    endp[0] = m_groups->CaptainOf(vc.x());
    endp[1] = m_groups->CaptainOf(vc.y());
    HexColor endc[2];
    endc[0] = m_brd->GetColor(endp[0]);
    endc[1] = m_brd->GetColor(endp[1]);

    if (endc[0] == other || endc[1] == other) {
        LogInfo() << *m_brd << '\n';
        LogInfo() << vc << '\n';
    }

    bitset_t vcCapturedSet = m_capturedSet[endp[0]] | m_capturedSet[endp[1]];
   
    HexAssert(endc[0] != other);
    HexAssert(endc[1] != other);
    for (GroupIterator g(*m_groups, not_other); g; ++g) 
    {
        HexPoint z = g->Captain();
        if (z == endp[0] || z == endp[1]) continue;
        if (vc.carrier().test(z)) continue;
        bitset_t capturedSet = vcCapturedSet | m_capturedSet[z];
        bitset_t uncapturedSet = capturedSet;
        uncapturedSet.flip();
        for (int i=0; i<2; i++)
        {
            int j = (i + 1) & 1;
            if (m_param.and_over_edge || !HexPointUtil::isEdge(endp[i])) 
            {
                VCList* fulls = &m_con->GetList(VC::FULL, z, endp[i]);
                if ((fulls->softIntersection() & vc.carrier()
                     & uncapturedSet).any())
                    continue;
                
                AndRule rule = (endc[i] == EMPTY) ? CREATE_SEMI : CREATE_FULL;
                doAnd(z, endp[i], endp[j], rule, vc, capturedSet, 
                      &m_con->GetList(VC::FULL, z, endp[i]));
            }
        }
    }
}

/** Compares vc to each connection in the softlimit of the given list.
    Creates a new connection if intersection is empty, or if the
    intersection is a subset of the captured set. Created connections
    are added with AddNewFull() or AddNewSemi().
*/
void VCBuilder::doAnd(HexPoint from, HexPoint over, HexPoint to,
                      AndRule rule, const VC& vc, const bitset_t& capturedSet, 
                      const VCList* old)
{
    if (old->empty())
        return;

    bitset_t stones;
    stones.set(m_groups->CaptainOf(over));

    int soft = old->softlimit();
    VCList::const_iterator i = old->begin();
    VCList::const_iterator end = old->end();
    for (int count=0; count<soft && i!=end; ++i, ++count) 
    {
        if (!i->processed())
            continue;
        if (i->carrier().test(to))
            continue;
        bitset_t intersection = i->carrier() & vc.carrier();
        if (intersection.none())
        {
            if (rule == CREATE_FULL)
            {
                m_statistics->and_full_attempts++;
                if (AddNewFull(VC::AndVCs(from, to, *i, vc, stones)))
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
                if (AddNewFull(VC::AndVCs(from, to, *i, vc, 
                                          capturedSet, stones)))
                    m_statistics->and_full_successes++;
            }
            else if (rule == CREATE_SEMI)
            {
                m_statistics->and_semi_attempts++;
                if (AddNewSemi(VC::AndVCs(from, to, *i, vc,
                                          capturedSet, over)))
                    m_statistics->and_semi_successes++;
            }
        }
    }
}

/** Runs over all subsets of size 2 to max_ors of semis containing vc
    and adds the union to out if it has an empty intersection. This
    function is a major bottleneck and so needs to be as efficient as
    possible.

    TODO: Document this more!

    TODO: Check if unrolling the recursion really does speed it up.

    @return number of connections successfully added.
*/
int VCBuilder::OrRule::operator()(const VC& vc, 
                                  const VCList* semi_list, 
                                  VCList* full_list, 
                                  std::list<VC>& added, 
                                  int max_ors,
                                  ChangeLog<VC>* log, 
                                  VCBuilderStatistics& stats)
{
    if (semi_list->empty())
        return 0;
    
    // copy processed semis (unprocessed semis are not used here)
    m_semi.clear();
    int soft = semi_list->softlimit();
    VCList::const_iterator it = semi_list->begin();
    VCList::const_iterator end = semi_list->end();
    for (int count=0; count<soft && it!=end; ++count, ++it)
        if (it->processed())
            m_semi.push_back(*it);

    if (m_semi.empty())
        return 0;

    std::size_t N = m_semi.size();

    // for each i in [0, N-1], compute intersection of semi[i, N-1]
    if (m_tail.size() < N)
        m_tail.resize(N);
    m_tail[N-1] = m_semi[N-1].carrier();
    for (int i=N-2; i>=0; --i)
        m_tail[i] = m_semi[i].carrier() & m_tail[i+1];

    max_ors--;
    HexAssert(max_ors < 16);

    // compute the captured-set union for the endpoints of this list
    bitset_t capturedSet = m_builder.m_capturedSet[semi_list->getX()] 
                         | m_builder.m_capturedSet[semi_list->getY()];
    bitset_t uncapturedSet = capturedSet;
    uncapturedSet.flip();

    std::size_t index[16];
    bitset_t ors[16];
    bitset_t ands[16];

    ors[0] = vc.carrier();
    ands[0] = vc.carrier();
    index[1] = 0;
    
    int d = 1;
    int count = 0;
    while (true) 
    {
        std::size_t i = index[d];

        // the current intersection (some subset from [0, i-1]) is not
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
        
        ands[d] = ands[d-1] & m_semi[i].carrier();
        ors[d]  =  ors[d-1] | m_semi[i].carrier();

        if (ands[d].none()) 
        {
            /** Create a new full.
                
                @note We do no use AddNewFull() because if add is
                successful, it checks for semi-supersets and adds the
                list to the queue. Both of these operations are not
                needed here.
            */
            VC v(full_list->getX(), full_list->getY(), ors[d], 
                 EMPTY_BITSET, VC_RULE_OR);

            stats.or_attempts++;
            if (full_list->add(v, log) != VCList::ADD_FAILED) 
            {
                count++;
                stats.or_successes++;
                added.push_back(v);
            }
        
            ++index[d];
        } 
        else if (BitsetUtil::IsSubsetOf(ands[d], capturedSet))
        {
            /** Create a new full.
                This vc has one or both captured sets in its carrier.
            */
            bitset_t carrier = ors[d];
            if ((ands[d] & m_builder.m_capturedSet[semi_list->getX()]).any())
                carrier |= m_builder.m_capturedSet[semi_list->getX()];
            if ((ands[d] & m_builder.m_capturedSet[semi_list->getY()]).any())
                carrier |= m_builder.m_capturedSet[semi_list->getY()];

            VC v(full_list->getX(), full_list->getY(), carrier,
                 EMPTY_BITSET, VC_RULE_OR);

            stats.or_attempts++;
            if (full_list->add(v, log) != VCList::ADD_FAILED) 
            {
                count++;
                stats.or_successes++;
                added.push_back(v);
            }
        
            ++index[d];
        }
        else if (ands[d] == ands[d-1]) 
        {
            // this connection does not shrink intersection so skip it
            ++index[d];
        }
        else 
        {
            // this connection reduces intersection, if not at max depth
            // see if more semis can reduce it to the empty set (or at least
            // a subset of the captured set).
            if (d < max_ors) 
                index[++d] = ++i;
            else
                ++index[d];
        }
    }
    return count;
}

/** Performs Crossing-rule.

    The crossing rule requires exactly 3 pairwise disjoint SCs between
    two empty cells x and y such that at least two of the SCs have
    mustuse (also called stepping stones). The crossing rule then
    concludes that there exists an SC between one mustuse in an SC and
    one mustuse in the other SC. This conclusion holds for any pair of
    mustuse, so long as they are in different SCs.
    
    The key of this SC could be either x or y. As long as we discard
    superset SCs produced by the crossing rule (i.e. SCs joining the
    same two points but with a carrier that is a superset of a known
    SC) and the AND-rule of H-search is in effect, then for all
    crossing rule SCs that are kept, either x or y can be set as the
    key (i.e. both will work).
    
    Lastly, there is a special case when either x or y forms a bridge
    with an edge (i.e. where the carrier will be captured). WLOG say x
    forms a bridge. Then if we set x to be the key of the SCs we are
    trying to produce, then it is allowable for the 3 SCs to not be
    pairwise disjoint; they can overlap on this bridge carrier since
    it will be captured. This helps find more important ladder-type
    connections near the edge. In this case, the bridge carrier must
    be included in the carrier of the output SC.
    
    A couple more notes:
    1) Since the crossing rule requires two SCs with mustuse, it
    almost always finds connections near an edge. Thus, do not use
    this rule unless you are also using the option and_over_edge.
    2) Because of mustuse this rule is as efficient (or more so) than
    the AND/OR-rules of H-search. However, it does not find many
    connections, and produces minimal strength gains in our
    AIs. Hopefully it can be extended or inspire a more productive
    rule in the future.
    3) For more details, see our publication (submitted to ACG 2009;
    the usual trio of Phil, Broderick, and Ryan).

    TODO: Reuse std::vectors to reduce dynamic allocations?
*/
void VCBuilder::doCrossingRule(const VC& vc, const VCList* semi_list)
{
    if (m_brd->GetColor(vc.x()) != EMPTY || m_brd->GetColor(vc.y()) != EMPTY)
        return;
    
    // copy processed semis
    std::vector<VC> semi;
    int soft = semi_list->softlimit();
    VCList::const_iterator it = semi_list->begin();
    VCList::const_iterator end = semi_list->end();
    for (int count=0; count<soft && it!=end; ++count, ++it) {
        if (it->processed())
            semi.push_back(*it);
    }
    if (semi.empty()) 
        return;

    // the endpoints will be the keys to any semi-connections we create
    std::vector<HexPoint> keys;
    keys.push_back(vc.x());
    keys.push_back(vc.y());
    
    // track if an SC has empty mustuse and get captains for vc's mustuse
    bitset_t mu[3];
    bool has_empty_mustuse0 = vc.stones().none();
    mu[0] = m_groups->CaptainizeBitset(vc.stones());

    for (std::size_t i=0; i<semi.size(); ++i) {
        const VC& vi = semi[i];

        bool has_empty_mustuse1 = has_empty_mustuse0;
        
        bool has_miai1 = false;
        HexPoint miai_endpoint = INVALID_POINT;
        HexPoint miai_edge = INVALID_POINT;

        {
            bitset_t I = vi.carrier() & vc.carrier();
            if (I.none()) 
            {
                // good!
            } 
            else if (I.count() == 2)
            {
                HexPoint k,e;
                if (VCUtils::ValidEdgeBridge(*m_brd, I, k, e) 
                    && (k==keys[0] || k==keys[1]))
                {
                    has_miai1 = true;
                    miai_endpoint = k;
                    miai_edge = e;
                }
                else
                {
                    continue;
                }
            }
            else 
            {
                continue;
            }
        }        
        
        if (vi.stones().none()) {
            if (has_empty_mustuse1) continue;
            has_empty_mustuse1 = true;
        }
                
        // get captains for vi's mustuse and check for intersection
        // with mu[0].
	mu[1] = m_groups->CaptainizeBitset(vi.stones());
        if ((mu[0] & mu[1]).any()) continue;
	

        //////////////////////////////////////////////////////////////
        for (std::size_t j=i+1; j<semi.size(); ++j) {
            const VC& vj = semi[j];

            bool has_empty_mustuse2 = has_empty_mustuse1;

            bool has_miai = has_miai1;

            {
                bitset_t I = vj.carrier() & vc.carrier();
                if (I.none()) 
                {
                    // good!
                } 
                else if (I.count() == 2 && !has_miai)
                {
                    HexPoint k,e;
                    if (VCUtils::ValidEdgeBridge(*m_brd, I, k, e) 
                        && (k==keys[0] || k==keys[1]))
                    {
                        has_miai = true;
                        miai_endpoint = k;
                        miai_edge = e;
                    }
                    else
                    {
                        continue;
                    }
                }
                else 
                {
                    continue;
                }
            }

            {
                bitset_t I = vj.carrier() & vi.carrier();
                if (I.none()) 
                {
                    // good!
                } 
                else if (I.count() == 2 && !has_miai)
                {
                    HexPoint k,e;
                    if (VCUtils::ValidEdgeBridge(*m_brd, I, k, e) 
                        && (k==keys[0] || k==keys[1]))
                    {
                        has_miai = true;
                        miai_endpoint = k;
                        miai_edge = e;
                    }
                    else 
                    {
                        continue;
                    }
                }    
                else
                {
                    continue;
                }
            }

            if (vj.stones().none()) {
                if (has_empty_mustuse2) continue;
                has_empty_mustuse2 = true;
            }
	    
            // get captains for vj's mustuse and check for intersection
            // with mu[0] and mu[1].
	    mu[2] = m_groups->CaptainizeBitset(vj.stones());
	    if ((mu[2] & (mu[0] | mu[1])).any()) continue;
            
            // (vc, vi, vj) are pairwise disjoint and only one of the
            // three potentially has an empty mustuse set, and their
            // mustuses are all disjoint.
            bitset_t carrier = vi.carrier() | vj.carrier() | vc.carrier();
	    HexAssert(!carrier.test(vc.x()));
	    HexAssert(!carrier.test(vc.y()));
            carrier.set(vc.x());
            carrier.set(vc.y());

            // add a new full-connection between endpoints and
            // used stones
            for (BitsetIterator p(mu[0] | mu[1] | mu[2]); p; ++p)
            {
                for (size_t k=0; k<keys.size(); ++k) 
                {
                    bitset_t our_carrier = carrier;
                    our_carrier.reset(keys[k]);

                    AddNewFull(VC(keys[k], *p, our_carrier, 
                                  EMPTY_BITSET, VC_RULE_CROSSING));
                }
            }

            // find all valid endpoints for the new semi-connections
            std::set<HexPointPair> ends;
            for (int a=0; a<2; ++a) {
                for (int b=a+1; b<3; ++b) {
                    for (BitsetIterator p1(mu[a]); p1; ++p1) {
                        for (BitsetIterator p2(mu[b]); p2; ++p2) {
			    HexAssert(*p1 != *p2);

                            // if using miai, must use the miai edge
                            if (has_miai 
                                && *p1 != miai_edge
                                && *p2 != miai_edge)
                                continue;

                            ends.insert(std::make_pair(std::min(*p1, *p2),
                                                       std::max(*p1, *p2)));

                        }                        
                    }                    
                }
            }

            for (std::size_t k=0; k<keys.size(); ++k) 
            {
                HexPoint key = keys[k];

                // if we have a miai, the only valid key is the miai endpoint
                if (has_miai && key != miai_endpoint)
                    continue;

                // add semi to all unique pairs found
                std::set<HexPointPair>::iterator it;
                for (it = ends.begin(); it != ends.end(); ++it) 
                {
                    HexPoint p1 = it->first;
                    HexPoint p2 = it->second;

                    bitset_t empty;
		    // no mustuse when generated from crossing rule
                    VC new_semi(p1, p2, key, carrier, empty, VC_RULE_CROSSING);

                    m_statistics->crossing_attempts++;
                    if (AddNewSemi(new_semi))
                    {
                        m_statistics->crossing_successes++;
#if 0
                        LogInfo() << "Crossing Rule: "
                                 << new_semi << "\n"
                                 << vc << "\n"
                                 << vi << "\n"
                                 << vj << '\n';
#endif
                    }
                }
            }
        }
    }
}

/** Tries to add a new full-connection to list between (vc.x(), vc.y()).

    If vc is successfully added, then:

    1) Removes any semi-connections between (vc.x(), vc.y()) that are
    supersets of vc.
    
    2) Adds (vc.x(), vc.y()) to the queue if vc was added inside the
    softlimit.
*/
bool VCBuilder::AddNewFull(const VC& vc)
{
    HexAssert(vc.type() == VC::FULL);
    VCList::AddResult result = m_con->Add(vc, m_log);
    if (result != VCList::ADD_FAILED) 
    {
        // a semi that is a superset of a full is useless, so remove
        // any that exist.
        m_con->GetList(VC::SEMI, vc.x(), vc.y())
            .removeSuperSetsOf(vc.carrier(), m_log);
                
        // add this list to the queue if inside the soft-limit
        if (result == VCList::ADDED_INSIDE_SOFT_LIMIT)
            m_queue.push(std::make_pair(vc.x(), vc.y()));

        return true;
    }
    return false;
}

/** Tries to add a new semi-connection to list between (vc.x(), vc.y()). 
        
    Does not add if vc is a superset of some full-connection between
    (vc.x(), and vc.y()).
    
    If vc is successfully added and intersection on semi-list is
    empty, then:

    1) if vc added inside soft limit, adds (vc.x(), vc.y()) to queue.
    
    2) otherwise, if no full exists between (vc.x(), vc.y()), adds the
    or over the entire semi list.
*/
bool VCBuilder::AddNewSemi(const VC& vc)
{
    VCList* out_full = &m_con->GetList(VC::FULL, vc.x(), vc.y());
    VCList* out_semi = &m_con->GetList(VC::SEMI, vc.x(), vc.y());
    
    if (!out_full->isSupersetOfAny(vc.carrier())) 
    {
        VCList::AddResult result = out_semi->add(vc, m_log);
        if (result != VCList::ADD_FAILED) 
        {
            if (out_semi->hardIntersection().none())
            {
                if (result == VCList::ADDED_INSIDE_SOFT_LIMIT) 
                {
                    m_queue.push(std::make_pair(vc.x(), vc.y()));
                } 
                else if (out_full->empty())
                {
                    bitset_t carrier = m_param.use_greedy_union 
                        ? out_semi->getGreedyUnion() 
                        : out_semi->getUnion();
                    
                    VC v(out_full->getX(), out_full->getY(), 
                         carrier, EMPTY_BITSET, VC_RULE_ALL);

                    out_full->add(v, m_log);
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
       << "crossing-s=" << crossing_successes << "/"
                        << crossing_attempts << '\n'
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
    simulating the front of the queue; that is, push() uses
    push_back() to add elements to the back and pop() increments the
    index of the front. This means the vector will need to be as large
    as the number of calls to push(), not the maximum number of
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

bool VCBuilder::WorkQueue::empty() const
{
    return m_head == m_array.size();
}

const HexPointPair& VCBuilder::WorkQueue::front() const
{
    return m_array[m_head];
}

std::size_t VCBuilder::WorkQueue::capacity() const
{
    return m_array.capacity();
}

void VCBuilder::WorkQueue::clear()
{
    memset(m_seen, 0, sizeof(m_seen));
    m_array.clear();
    m_head = 0;
}

void VCBuilder::WorkQueue::pop()
{
    m_seen[front().first][front().second] = false;
    m_head++;
}

void VCBuilder::WorkQueue::push(const HexPointPair& p)
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
