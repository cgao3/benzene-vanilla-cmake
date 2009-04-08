//----------------------------------------------------------------------------
// $Id: InferiorCells.cpp 1657 2008-09-15 23:32:09Z broderic $
//----------------------------------------------------------------------------

#include "BitsetIterator.hpp"
#include "InferiorCells.hpp"

/** @file

    @todo Handle a sink in the dominated component graph being
    partially outside the mustplay -- in this case, no representative
    of the sink need be chosen (they are all losing).

    @note The set of dominated cells must be recomputed each time the
    domination graph or the vulnerable info is changed. Dominated()
    does this computation lazily when required.

*/

//----------------------------------------------------------------------------

InferiorCells::InferiorCells()
    : m_dominated_computed(false)
{
}

//----------------------------------------------------------------------------

bitset_t InferiorCells::Dominated() const
{
    if (!m_dominated_computed) {
        
        // remove vulnerable cells from graph
        Digraph<HexPoint> g(m_dom_graph);
        for (BitsetIterator p(Vulnerable()); p; ++p) {
            g.RemoveVertex(*p);
        }

        bitset_t captains = InferiorCellsUtil::FindDominationCaptains(g);
        bitset_t vertices = BitsetUtil::SetToBitset(g.Vertices());
        m_dominated = vertices - captains;
        m_dominated_computed = true;

        
        /// @todo ensure m_dominated is disjoint from all others.
        HexAssert((m_dominated & Vulnerable()).none());

    }
    return m_dominated;
}


bitset_t InferiorCells::All() const
{
    return Dead()
        | Vulnerable() | Dominated()
        | Captured(BLACK) | Captured(WHITE) 
        | PermInf(BLACK) | PermInf(WHITE);
}

bitset_t InferiorCells::Fillin(HexColor color) const
{
    bitset_t ret = Captured(color) | PermInf(color);
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
    m_dominated_computed = false;

    AssertPairwiseDisjoint();
}

void InferiorCells::AddVulnerable(HexPoint cell, 
                                  const std::set<VulnerableKiller>& killers)
{
    m_vulnerable.set(cell);
    m_killers[cell].insert(killers.begin(), killers.end());
    m_dominated_computed = false;

    AssertPairwiseDisjoint();
}

void InferiorCells::AddVulnerable(HexPoint cell, HexPoint killer)
{
    m_vulnerable.set(cell);
    m_killers[cell].insert(VulnerableKiller(killer));
    m_dominated_computed = false;

    AssertPairwiseDisjoint();
}

void InferiorCells::AddVulnerable(HexPoint cell, const VulnerableKiller& killer)
{
    m_vulnerable.set(cell);
    m_killers[cell].insert(killer);
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

void InferiorCells::AddPermInfFrom(HexColor color, const InferiorCells& other)
{
    m_perm_inf[color] |= other.m_perm_inf[color];
    m_perm_inf_carrier[color] |= other.m_perm_inf_carrier[color];
    AssertPairwiseDisjoint();
}

//----------------------------------------------------------------------------

void InferiorCells::Clear()
{
    m_dead.reset();
    m_vulnerable.reset();
    for (int i=0; i<BITSETSIZE; i++) {
        m_killers[i].clear();
    }
    for (BWIterator c; c; ++c) {
        m_captured[*c].reset();
        m_perm_inf[*c].reset();
        m_perm_inf_carrier[*c].reset();
    }
    m_dom_graph.Clear();
    m_dominated_computed = false;
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

void InferiorCells::ClearVulnerable()
{
    RemoveVulnerable(m_vulnerable);
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

//----------------------------------------------------------------------------

void InferiorCells::AssertPairwiseDisjoint() const
{
    HexAssert((m_dead & m_vulnerable).none());
    //    HexAssert((m_dead & m_dominated).none());
    //    HexAssert((m_vulnerable & m_dominated).none());

    for (BWIterator c; c; ++c) {
        HexAssert((m_captured[*c] & m_dead).none());
        //        HexAssert((m_captured[*c] & m_dominated).none());
        HexAssert((m_captured[*c] & m_vulnerable).none());
        HexAssert((m_captured[*c] & m_captured[!*c]).none());
        
        HexAssert((m_perm_inf[*c] & m_dead).none());
        //        HexAssert((m_perm_inf[*c] & m_dominated).none());
        HexAssert((m_perm_inf[*c] & m_vulnerable).none());

        HexAssert((m_captured[*c] & m_perm_inf[*c]).none());
        HexAssert((m_captured[*c] & m_perm_inf[!*c]).none());
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

//----------------------------------------------------------------------------

std::string InferiorCells::GuiOutput() const
{
    uint t, c=0;
    std::ostringstream out;
    for (int i=0; i<FIRST_INVALID; i++) {
        HexPoint p = static_cast<HexPoint>(i);
        std::ostringstream os;
        
        os << " " << p << " ";
        if (Dead().test(i)) {
            os << "db";
            //os << ((brd.getColor(p) == BLACK) ? "b" : "w");
        }
        else if (Captured(BLACK).test(i)) 
            os << "b";
        else if (Captured(WHITE).test(i))
            os << "w";
        else if (PermInf(BLACK).test(i)) {
            os << "pb[";
            os << "]";
        }
        else if (PermInf(WHITE).test(i)) {
            os << "pw[";
            os << "]";
        }
        else if (Vulnerable().test(i)) {
            os << "#[";
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
        else if (Dominated().test(i)) {

            os << "![";
            bool first=true;
            std::set<HexPoint>::iterator i;
            for (i=m_dom_graph.out_begin(p); i!=m_dom_graph.out_end(p); ++i) {
                if (Vulnerable().test(*i))
                    continue;
                if (!first) os << "-";
                os << *i;
                first = false;
            }
            os << "]";
        }
        else {

            continue;

        }

        std::string str = os.str();
        t = str.size();

        if (c + t > 40) {
            out << "\n";
            c = t;
        } else {
            c += t;
        }

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
