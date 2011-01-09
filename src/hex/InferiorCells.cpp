//----------------------------------------------------------------------------
/** @file InferiorCells.cpp

    @todo Handle a sink in the dominated component graph being
    partially outside the mustplay -- in this case, no representative
    of the sink need be chosen (they are all losing).

    @note The set of dominated cells must be recomputed each time the
    domination graph or the vulnerable or reversible info is changed.
    Dominated() does this computation lazily when required.
*/
//----------------------------------------------------------------------------

#include "BitsetIterator.hpp"
#include "InferiorCells.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

InferiorCells::InferiorCells()
    : m_dominated_computed(false)
{
}

//----------------------------------------------------------------------------

bitset_t InferiorCells::Dominated() const
{
    if (!m_dominated_computed) {
        
        // remove vulnerable and reversible cells from graph
        Digraph<HexPoint> g(m_dom_graph);
        for (BitsetIterator p(Vulnerable()); p; ++p) {
            g.RemoveVertex(*p);
        }
        for (BitsetIterator p(Reversible()); p; ++p) {
            g.RemoveVertex(*p);
        }

        bitset_t captains = InferiorCellsUtil::FindDominationCaptains(g);
        bitset_t vertices = BitsetUtil::SetToBitset(g.Vertices());
        m_dominated = vertices - captains;
        m_dominated_computed = true;

        /// TODO: ensure m_dominated is disjoint from all others.
        BenzeneAssert((m_dominated & Vulnerable()).none());
        BenzeneAssert((m_dominated & Reversible()).none());
    }
    return m_dominated;
}


bitset_t InferiorCells::All() const
{
    return Dead()
        | Vulnerable() | Reversible() | Dominated()
        | Captured(BLACK) | Captured(WHITE) 
        | PermInf(BLACK) | PermInf(WHITE)
        | MutualFillin(BLACK) | MutualFillin(WHITE);
}

bitset_t InferiorCells::Fillin(HexColor color) const
{
    bitset_t ret = Captured(color) | PermInf(color) | MutualFillin(color);
    if (color == DEAD_COLOR) ret |= Dead();
    return ret;
}

//----------------------------------------------------------------------------

void InferiorCells::AddDead(HexPoint dead)
{
    bitset_t b;
    b.set(dead);
    AddDead(b);
}

void InferiorCells::AddDead(const bitset_t& dead)
{
    m_dead |= dead;
    
    RemoveVulnerable(dead);
    RemoveReversible(dead);
    RemoveDominated(dead);

    AssertPairwiseDisjoint();
}

void InferiorCells::AddCaptured(HexColor color, HexPoint captured)
{
    bitset_t b;
    b.set(captured);
    AddCaptured(color, b);
}

void InferiorCells::AddCaptured(HexColor color, const bitset_t& captured)
{
    m_captured[color] |= captured;

    RemoveVulnerable(captured);
    RemoveReversible(captured);
    RemoveDominated(captured);

    AssertPairwiseDisjoint();
}

void InferiorCells::AddPermInf(HexColor color, 
                               const bitset_t& cells, 
                               const bitset_t& carrier)
{
    m_perm_inf[color] |= cells;
    m_perm_inf_carrier[color] |= carrier;

    RemoveVulnerable(cells);
    RemoveReversible(cells);
    RemoveDominated(cells);

    AssertPairwiseDisjoint();
}

void InferiorCells::AddPermInf(HexColor color, HexPoint cell, 
                               const bitset_t& carrier)
{
    bitset_t b;
    b.set(cell);
    AddPermInf(color, b, carrier);
}

void InferiorCells::AddMutualFillin(HexColor color, 
                                    const bitset_t& cells, 
                                    const bitset_t& carrier)
{
    m_mutual_fillin[color] |= cells;
    m_mutual_fillin_carrier[color] |= carrier;

    RemoveVulnerable(cells);
    RemoveReversible(cells);
    RemoveDominated(cells);

    AssertPairwiseDisjoint();
}

void InferiorCells::AddMutualFillin(HexColor color, HexPoint cell, 
                                    const bitset_t& carrier)
{
    bitset_t b;
    b.set(cell);
    AddMutualFillin(color, b, carrier);
}

void InferiorCells::AddDominated(HexPoint cell, const std::set<HexPoint>& dom)
{
    m_dom_graph.AddEdges(cell, dom);
    m_dominated_computed = false;

    //AssertPairwiseDisjoint();
}

void InferiorCells::AddDominated(HexPoint cell, HexPoint dominator)
{
    m_dom_graph.AddEdge(cell, dominator);
    m_dominated_computed = false;

    //AssertPairwiseDisjoint();
}

void InferiorCells::AddVulnerable(HexPoint cell, 
                                  const std::set<HexPoint>& killers)
{
    m_vulnerable.set(cell);

    // add each killer with an empty carrier
    std::set<HexPoint>::iterator it = killers.begin();
    for (; it != killers.end(); ++it) {
        m_killers[cell].insert(VulnerableKiller(*it));
    }

    // update reversible and dominated
    RemoveReversible(cell);
    m_dominated_computed = false;

    AssertPairwiseDisjoint();
}

void InferiorCells::AddVulnerable(HexPoint cell, 
                                  const std::set<VulnerableKiller>& killers)
{
    m_vulnerable.set(cell);
    m_killers[cell].insert(killers.begin(), killers.end());
    RemoveReversible(cell);
    m_dominated_computed = false;

    AssertPairwiseDisjoint();
}

void InferiorCells::AddVulnerable(HexPoint cell, HexPoint killer)
{
    m_vulnerable.set(cell);
    m_killers[cell].insert(VulnerableKiller(killer));
    RemoveReversible(cell);
    m_dominated_computed = false;

    AssertPairwiseDisjoint();
}

void InferiorCells::AddVulnerable(HexPoint cell, const VulnerableKiller& killer)
{
    m_vulnerable.set(cell);
    m_killers[cell].insert(killer);
    RemoveReversible(cell);
    m_dominated_computed = false;

    AssertPairwiseDisjoint();
}

void InferiorCells::AddReversible(HexPoint cell, bitset_t carrier,
                                  HexPoint reverser)
{
    // Cell and carrier have equivalent roles, so just merge
    bitset_t reversibleCandidates = carrier;
    reversibleCandidates.set(cell);

    // Cannot add any if not independent of previous reversible cells
    if (m_allReversibleCarriers.test(reverser)) return;
    if ((m_allReversers & reversibleCandidates).any()) return;
    if (reversibleCandidates.test(reverser)) return;

    // Independent, so mark all (non-vulnerable) candidates as reversible
    bool noAdditions = true;
    for (BitsetIterator it(reversibleCandidates); it; ++it)
    {
        if (m_vulnerable.test(*it)) continue;
        noAdditions = false;
        m_reversible.set(*it);
        m_reversers[*it].insert(reverser);
    }
    if (noAdditions) return;
    m_allReversibleCarriers |= reversibleCandidates;
    m_allReversers.set(reverser);

    m_dominated_computed = false;
    AssertPairwiseDisjoint();
}

void InferiorCells::AddReversible(HexPoint cell, bitset_t carrier,
                                  const std::set<HexPoint>& reversers)
{
    // Cell and carrier have equivalent roles, so just merge
    bitset_t reversibleCandidates = carrier;
    reversibleCandidates.set(cell);
    // Merge all reversers into one big pot (all or nothing :)
    bitset_t reverserCandidates;
    for (std::set<HexPoint>::iterator it = reversers.begin();
         it != reversers.end(); ++it)
    {
        reverserCandidates.set(*it);
    }

    // Cannot add any if not independent of previous reversible cells
    if ((m_allReversibleCarriers & reverserCandidates).any()) return;
    if ((m_allReversers & reversibleCandidates).any()) return;
    if ((reverserCandidates & reversibleCandidates).any()) return;

    // Independent, so mark all (non-vulnerable) candidates as reversible
    bool noAdditions = true;
    for (BitsetIterator it(reversibleCandidates); it; ++it)
    {
        if (m_vulnerable.test(*it)) continue;
        noAdditions = false;
        m_reversible.set(*it);
        m_reversers[*it].insert(reversers.begin(), reversers.end());
    }
    if (noAdditions) return;
    m_allReversibleCarriers |= reversibleCandidates;
    m_allReversers |= reverserCandidates;

    m_dominated_computed = false;
    AssertPairwiseDisjoint();
}

//----------------------------------------------------------------------------

void InferiorCells::AddDominatedFrom(const InferiorCells& other)
{
    bitset_t vertices=BitsetUtil::SetToBitset(other.m_dom_graph.Vertices());
    for (BitsetIterator p(vertices); p; ++p) {
        AddDominated(*p, other.m_dom_graph.OutSet(*p));
    }
    //AssertPairwiseDisjoint();
}

void InferiorCells::AddVulnerableFrom(const InferiorCells& other)
{
    for (BitsetIterator p(other.Vulnerable()); p; ++p) {
        AddVulnerable(*p, other.m_killers[*p]);
    }
    AssertPairwiseDisjoint();
}

void InferiorCells::AddReversibleFrom(const InferiorCells& other)
{
    for (BitsetIterator p(other.Reversible()); p; ++p) {
        AddReversible(*p, EMPTY_BITSET, other.m_reversers[*p]);
    }
    m_allReversibleCarriers |= other.AllReversibleCarriers();
    m_allReversers |= other.AllReversers();
    AssertPairwiseDisjoint();
}

void InferiorCells::AddPermInfFrom(HexColor color, const InferiorCells& other)
{
    m_perm_inf[color] |= other.m_perm_inf[color];
    m_perm_inf_carrier[color] |= other.m_perm_inf_carrier[color];
    AssertPairwiseDisjoint();
}

void InferiorCells::AddMutualFillinFrom(HexColor color,
                                        const InferiorCells& other)
{
    m_mutual_fillin[color] |= other.m_mutual_fillin[color];
    m_mutual_fillin_carrier[color] |= other.m_mutual_fillin_carrier[color];
    AssertPairwiseDisjoint();
}

//----------------------------------------------------------------------------

void InferiorCells::Clear()
{
    ClearDead();
    ClearVulnerable();
    ClearReversible();
    for (BWIterator c; c; ++c) {
        ClearCaptured(*c);
        ClearPermInf(*c);
        ClearMutualFillin(*c);
    }
    ClearDominated();
}

void InferiorCells::ClearDead()
{
    m_dead.reset();
}

void InferiorCells::ClearCaptured(HexColor color)
{
    m_captured[color].reset();
}

void InferiorCells::ClearPermInf(HexColor color)
{
    m_perm_inf[color].reset();
    m_perm_inf_carrier[color].reset();
}

void InferiorCells::ClearMutualFillin(HexColor color)
{
    m_mutual_fillin[color].reset();
    m_mutual_fillin_carrier[color].reset();
}

void InferiorCells::ClearVulnerable()
{
    RemoveVulnerable(m_vulnerable);
    m_dominated_computed = false;
}

void InferiorCells::ClearReversible()
{
    RemoveReversible(m_reversible);
    m_allReversibleCarriers.reset();
    m_allReversers.reset();
    m_dominated_computed = false;
}

void InferiorCells::ClearDominated()
{
    m_dom_graph.Clear();
    m_dominated_computed = false;
}

//----------------------------------------------------------------------------

void InferiorCells::RemoveDominated(const bitset_t& dominated)
{
    bitset_t vertices = BitsetUtil::SetToBitset(m_dom_graph.Vertices());
    for (BitsetIterator p(vertices & dominated); p; ++p) {
        m_dom_graph.RemoveVertex(*p);
    }
    m_dominated_computed = false;
}

void InferiorCells::RemoveVulnerable(const bitset_t& vulnerable)
{
    for (BitsetIterator p(vulnerable & m_vulnerable); p; ++p) {
        m_killers[*p].clear();
    }
    m_vulnerable = m_vulnerable - vulnerable;
    m_dominated_computed = false;
}

void InferiorCells::RemoveReversible(const bitset_t& reversible)
{
    for (BitsetIterator p(reversible & m_reversible); p; ++p) {
        m_reversers[*p].clear();
    }
    m_reversible = m_reversible - reversible;
    m_dominated_computed = false;
}

void InferiorCells::RemoveReversible(HexPoint reversible)
{
    if (m_reversible.test(reversible)) {
        m_reversers[reversible].clear();
        m_reversible.reset(reversible);
        m_dominated_computed = false;
    }
}

//----------------------------------------------------------------------------

void InferiorCells::AssertPairwiseDisjoint() const
{
    BenzeneAssert((m_dead & m_vulnerable).none());
    BenzeneAssert((m_dead & m_reversible).none());
    BenzeneAssert((m_reversible & m_vulnerable).none());
    BenzeneAssert((m_allReversibleCarriers & m_allReversers).none());
    BenzeneAssert((m_reversible & m_allReversibleCarriers) == m_reversible);
    BenzeneAssert(!m_dominated_computed || (m_dead & m_dominated).none());
    BenzeneAssert(!m_dominated_computed || (m_vulnerable & m_dominated).none());
    BenzeneAssert(!m_dominated_computed || (m_reversible & m_dominated).none());

    for (BWIterator c; c; ++c) {
        BenzeneAssert((m_captured[*c] & m_dead).none());
        BenzeneAssert(!m_dominated_computed
                  || (m_captured[*c] & m_dominated).none());
        BenzeneAssert((m_captured[*c] & m_vulnerable).none());
        BenzeneAssert((m_captured[*c] & m_reversible).none());
        BenzeneAssert((m_captured[*c] & m_captured[!*c]).none());
        
        BenzeneAssert((m_perm_inf[*c] & m_dead).none());
        BenzeneAssert(!m_dominated_computed
                  || (m_perm_inf[*c] & m_dominated).none());
        BenzeneAssert((m_perm_inf[*c] & m_vulnerable).none());
        BenzeneAssert((m_perm_inf[*c] & m_reversible).none());

        BenzeneAssert((m_captured[*c] & m_perm_inf[*c]).none());
        BenzeneAssert((m_captured[*c] & m_perm_inf[!*c]).none());

        BenzeneAssert((m_mutual_fillin[*c] & m_dead).none());
        BenzeneAssert(!m_dominated_computed
                  || (m_mutual_fillin[*c] & m_dominated).none());
        BenzeneAssert((m_mutual_fillin[*c] & m_vulnerable).none());
        BenzeneAssert((m_mutual_fillin[*c] & m_reversible).none());

        BenzeneAssert((m_mutual_fillin[*c] & m_captured[*c]).none());
        BenzeneAssert((m_mutual_fillin[*c] & m_captured[!*c]).none());
        BenzeneAssert((m_mutual_fillin[*c] & m_perm_inf[*c]).none());
        BenzeneAssert((m_mutual_fillin[*c] & m_perm_inf[!*c]).none());
    }
}

//----------------------------------------------------------------------------

bitset_t InferiorCells::FindPresimplicialPairs() const
{
    bitset_t fillin;
    std::set<VulnerableKiller>::iterator k1, k2;

    /** @todo Handle vulnerable cycles larger than length 2? If they
        occur, they are extrememly rare, so it is probably not worth
        it. */

    for (BitsetIterator x(m_vulnerable); x; ++x) {
        if (fillin.test(*x)) continue;

        for (k1 = m_killers[*x].begin(); k1 != m_killers[*x].end(); ++k1) {
            HexPoint y = k1->killer();
            if (fillin.test(y)) continue;
            if ((k1->carrier() & fillin).any()) continue;

            bool success = false;
            for (k2 = m_killers[y].begin(); k2 != m_killers[y].end(); ++k2) {
                HexPoint z = k2->killer();
                if (z != *x) continue;
                if (fillin.test(z)) continue;
                if ((k2->carrier() & fillin).any()) continue;

                // if x kills y and y kills x and the carriers do not
                // intersect, fill them in along with their carriers. 
                if ((k1->carrier() & k2->carrier()).none()) {
                    bitset_t both = k1->carrier() | k2->carrier();
                    if ((both & fillin).none()) {
                        fillin |= both;
                        fillin.set(y);
                        fillin.set(*x);
                        success = true;
                        break;
                    }
                }
            }

            // if x and a killer of x have been filled in, there is no
            // point checking the remaining killers of x. 
            if (success) break;
        }
    }

    return fillin;
}

bitset_t InferiorCells::DeductionSet(HexColor color) const
{
    bitset_t deductionSet;
    deductionSet = Captured(color);
    deductionSet |= PermInf(color);
    deductionSet |= PermInfCarrier(color);
    deductionSet |= MutualFillin(color);
    deductionSet |= MutualFillinCarrier(color);
    return deductionSet;
}

//----------------------------------------------------------------------------

std::string InferiorCells::GuiOutput() const
{
    std::size_t c = 0;
    std::ostringstream out;
    for (int i = 0; i < FIRST_INVALID; ++i) 
    {
        HexPoint p = static_cast<HexPoint>(i);
        std::ostringstream os;
        
        os << " " << p << " ";
        if (Dead().test(i)) 
        {
            os << "fd";
            os << ((DEAD_COLOR == BLACK) ? "b" : "w");
        }
        else if (Captured(BLACK).test(i))
            os << "fcb";
        else if (Captured(WHITE).test(i))
            os << "fcw";
        else if (PermInf(BLACK).test(i))
            os << "fpb";
        else if (PermInf(WHITE).test(i))
            os << "fpw";
        else if (MutualFillin(BLACK).test(i))
            os << "fub";
        else if (MutualFillin(WHITE).test(i))
            os << "fuw";
        else if (Vulnerable().test(i)) 
        {
            os << "iv[";
            bool first=true;
            std::set<VulnerableKiller>::const_iterator i;
            for (i = m_killers[p].begin(); i != m_killers[p].end(); ++i) 
            {
                if (!first) os << "-";
                os << i->killer();
                first = false;
            }
            os << "]";
        }
        else if (Reversible().test(i)) 
        {
            os << "ir[";
            bool first=true;
            std::set<HexPoint>::const_iterator i;
            for (i = m_reversers[p].begin(); i != m_reversers[p].end(); ++i) 
            {
                if (!first) os << "-";
                os << *i;
                first = false;
            }
            os << "]";
        }
        else if (Dominated().test(i)) 
        {
            os << "id[";
            bool first=true;
            std::set<HexPoint>::iterator i;
            for (i=m_dom_graph.out_begin(p); i!=m_dom_graph.out_end(p); ++i) 
            {
                // PHIL IS CONFUSED: CAN THIS HAPPEN??
                if (Vulnerable().test(*i))
                    continue;
                if (!first) os << "-";
                os << *i;
                first = false;
            }
            os << "]";
        }
        else 
            continue;

        std::string str = os.str();
        std::size_t t = str.size();
        if (c + t > 40) 
        {
            out << '\n';
            c = t;
        } 
        else 
            c += t;

        out << str;
    }
    return out.str();
}

//----------------------------------------------------------------------------

bitset_t 
InferiorCellsUtil::FindDominationCaptains(const Digraph<HexPoint>& graph)
{
    bitset_t captains;

    // Find the strongly connected components of the domination graph.
    std::vector<HexPointSet> comp;
    graph.FindStronglyConnectedComponents(comp);

    // Find the sinks in the component graph
    for (std::size_t i=0; i < comp.size(); ++i) {

        std::set<HexPoint> out;
        graph.OutSet(comp[i], out);

        // remove members of the component from the outset since
        // we are only concerned with edges leaving the component
        std::set<HexPoint>::iterator it;
        for (it = comp[i].begin(); it != comp[i].end(); ++it) 
            out.erase(*it);
        
        // if a sink, pick a representative for the component
        if (out.empty()) {
            captains.set(*comp[i].begin());
        }
    }
    
    return captains;
}

//----------------------------------------------------------------------------
