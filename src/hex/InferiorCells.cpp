//----------------------------------------------------------------------------
/** @file InferiorCells.cpp

    @todo Handle a sink in the inferior component graph being
    partially outside the mustplay -- in this case, no representative
    of the sink need be chosen (they are all losing).

    @note The set of inferior cells must be recomputed each time the
    inferioriry graph or the reversible info is changed.
    Inferior() does this computation lazily when required.
*/
//----------------------------------------------------------------------------

#include "BitsetIterator.hpp"
#include "InferiorCells.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

InferiorCells::InferiorCells()
    : m_inferior_computed(false)
{
}

//----------------------------------------------------------------------------

bitset_t InferiorCells::Inferior() const
{
    if (!m_inferior_computed) {
        
        // Remove strong-reversible cells from graph
        Digraph<HexPoint> g(m_inf_graph);
        for (BitsetIterator p(Vulnerable()); p; ++p)
            g.RemoveVertex(*p);
	for (BitsetIterator p(SReversible()); p; ++p)
            g.RemoveVertex(*p);

        m_inferior = InferiorCellsUtil::PrunableFromInferiorityGraph(g);
        m_inferior_computed = true;

        /// TODO: ensure m_inferior is disjoint from all others.
        BenzeneAssert((m_inferior & Vulnerable()).none());
	BenzeneAssert((m_inferior & SReversible()).none());
    }
    return m_inferior;
}


bitset_t InferiorCells::All() const
{
    return  Fillin(BLACK) | Fillin(WHITE)
      | Vulnerable() | SReversible()
      | Inferior();
}

//----------------------------------------------------------------------------

void InferiorCells::AddFillin(HexColor color, HexPoint fillin)
{
    m_fillin[color].set(fillin);
    RemoveVulnerable(fillin);
    RemoveSReversible(fillin);
    RemoveInferior(fillin);
}

void InferiorCells::AddFillin(HexColor color, const bitset_t& fillin)
{
    m_fillin[color] |= fillin;

    RemoveVulnerable(fillin);
    RemoveSReversible(fillin);
    RemoveInferior(fillin);

    AssertPairwiseDisjoint();
}

void InferiorCells::AddVulnerable(HexPoint vulnerable, HexPoint killer)
{
    m_vulnerable.set(vulnerable);
    m_killers[vulnerable].insert(killer);
    RemoveSReversible(vulnerable);
    m_inferior_computed = false;

    AssertPairwiseDisjoint();
}

void InferiorCells::AddVulnerable(HexPoint vulnerable,
                                  const std::set<HexPoint>& killers)
{
    m_vulnerable.set(vulnerable);
    std::set<HexPoint>::iterator it = killers.begin();
    for (; it != killers.end(); ++it) {
      m_killers[vulnerable].insert(*it);
    }
    RemoveSReversible(vulnerable);
    m_inferior_computed = false;

    AssertPairwiseDisjoint();
}

/** The theorem used is that, if we can sort some (s_n,r_n,C_n) with
    s_n strong-(resp. threat-)reversed by r_n with carrier C_n in such
    a way that for all k<n, r_k (resp. r_k and s_k) not in C_n, then
    all the s_n can be pruned.
    (the carrier does not include s_n nor r_n)
    In practice, we keep a set of all the patterns that gave a pruning,
    and we make greedily sure that every new pattern blocks no other one
    or is blocked by no other one.
*/
/** blocker is INVALID_POINT for a strong-reverse pattern, and the
    reversed for a threat-reverse one. */
void InferiorCells::AddSReversible(HexPoint reversible,
				   const bitset_t& carrier,
				   HexPoint reverser,
				   bool is_threat)
{
    // First, we check that adding this would give more pruning, in order
    // not to load m_blockers and m_s_reversible_carriers for nothing.
    if (m_vulnerable.test(reversible) || m_s_reversible.test(reversible))
        return;

    if ((!m_s_reversible_carriers.test(reverser)
	 && (!is_threat ||
	     !m_s_reversible_carriers.test(reversible)))
	||
	(carrier & m_blockers).none())
    {
        m_blockers.set(reverser);
	if (is_threat) m_blockers.set(reversible);
	m_s_reversible_carriers |= carrier;
	m_s_reversible.set(reversible);
	m_s_reversers[reversible].insert(reverser);
    
	m_inferior_computed = false;
	AssertPairwiseDisjoint();
    }
} 

void InferiorCells::AddInferior(HexPoint inferior,
				HexPoint superior)
{
    m_inf_graph.AddEdge(inferior, superior);
    m_inferior_computed = false;

    //AssertPairwiseDisjoint();
}

void InferiorCells::AddInferior(HexPoint inferior,
				const std::set<HexPoint>& superiors)
{
    m_inf_graph.AddEdges(inferior, superiors);
    m_inferior_computed = false;

    //AssertPairwiseDisjoint();
}


//----------------------------------------------------------------------------

void InferiorCells::AddVulnerableFrom(const InferiorCells& other)
{
    for (BitsetIterator p(other.Vulnerable()); p; ++p)
        AddVulnerable(*p, other.m_killers[*p]);
    AssertPairwiseDisjoint();
}

void InferiorCells::AddSReversibleFrom(const InferiorCells& other)
{
    for (BitsetIterator p(other.SReversible()); p; ++p)
    {
        m_s_reversible.set(*p);
	m_s_reversers[*p].insert(other.m_s_reversers[*p].begin(),
				 other.m_s_reversers[*p].end());
    }
    m_blockers |= other.m_blockers ;
    m_s_reversible_carriers |= other.m_s_reversible_carriers ;
    m_inferior_computed = false;
    AssertPairwiseDisjoint();
}

void InferiorCells::AddInferiorFrom(const InferiorCells& other)
{
    bitset_t vertices=BitsetUtil::SetToBitset(other.m_inf_graph.Vertices());
    for (BitsetIterator p(vertices); p; ++p)
        AddInferior(*p, other.m_inf_graph.OutSet(*p));
    //AssertPairwiseDisjoint();
}

//----------------------------------------------------------------------------

void InferiorCells::Clear()
{
    for (BWIterator c; c; ++c) {
        ClearFillin(*c);
    }
    ClearVulnerable();
    ClearSReversible();
    ClearInferior();
}

void InferiorCells::ClearFillin(HexColor color)
{
    m_fillin[color].reset();
}

void InferiorCells::ClearVulnerable()
{
    RemoveVulnerable(m_vulnerable);
    m_inferior_computed = false;
}

void InferiorCells::ClearSReversible()
{
    RemoveSReversible(m_s_reversible);
    m_blockers = EMPTY;
    m_s_reversible_carriers = EMPTY;
    m_inferior_computed = false;
}

void InferiorCells::ClearInferior()
{
    m_inf_graph.Clear();
    m_inferior_computed = false;
}

//----------------------------------------------------------------------------

void InferiorCells::RemoveVulnerable(HexPoint vulnerable)
{
  if (m_vulnerable.test(vulnerable)) {
      m_killers[vulnerable].clear();
      m_vulnerable.reset(vulnerable);
      m_inferior_computed = false;
  }
}

void InferiorCells::RemoveVulnerable(const bitset_t& vulnerable)
{
    for (BitsetIterator p(vulnerable & m_vulnerable); p; ++p) {
        m_killers[*p].clear();
    }
    m_vulnerable = m_vulnerable - vulnerable;
    m_inferior_computed = false;
}

/** m_blockers and m_s_reversible_carriers are not updated, as we
    can't remove anything because maybe another strong-reversible
    move has the same carrier or blocker.
    Thus, calling this function should be avoided. */
void InferiorCells::RemoveSReversible(HexPoint reversible)
{
  if (m_s_reversible.test(reversible)) {
      m_s_reversers[reversible].clear();
      m_s_reversible.reset(reversible);
      m_inferior_computed = false;
  }
}

void InferiorCells::RemoveSReversible(const bitset_t& reversible)
{
    for (BitsetIterator p(reversible & m_s_reversible); p; ++p) {
        m_s_reversers[*p].clear();
    }
    m_s_reversible = m_s_reversible - reversible;
    m_inferior_computed = false;
}

void InferiorCells::RemoveInferior(HexPoint inferior)
{
  if (m_inferior.test(inferior)) {
      m_inf_graph.RemoveVertex(inferior);
      m_inferior_computed = false;
  }
}

void InferiorCells::RemoveInferior(const bitset_t& inferior)
{
    bitset_t vertices = BitsetUtil::SetToBitset(m_inf_graph.Vertices());
    for (BitsetIterator p(vertices & inferior); p; ++p) {
        m_inf_graph.RemoveVertex(*p);
    }
    m_inferior_computed = false;
}

//----------------------------------------------------------------------------

void InferiorCells::AssertPairwiseDisjoint() const
{
    BenzeneAssert((m_vulnerable & m_s_reversible).none());
    BenzeneAssert(!m_inferior_computed || (m_vulnerable & m_inferior).none());
    BenzeneAssert(!m_inferior_computed || (m_s_reversible & m_inferior).none());

    for (BWIterator c; c; ++c) {
        BenzeneAssert(!m_inferior_computed
                  || (m_fillin[*c] & m_inferior).none());
        BenzeneAssert((m_fillin[*c] & m_vulnerable).none());
	BenzeneAssert((m_fillin[*c] & m_s_reversible).none());
        BenzeneAssert((m_fillin[*c] & m_fillin[!*c]).none());
    }
}

//----------------------------------------------------------------------------

bitset_t InferiorCells::FindPresimplicialPairs() const
{
    bitset_t fillin;
  
    /** @todo Handle reversible cycles larger than length 2? If they
        occur, they are extrememly rare, so it is probably not worth
        it. */
    
    for (BitsetIterator x(m_vulnerable); x; ++x)
    {

        for (std::set<HexPoint>::iterator y = m_killers[*x].begin();
	     y != m_killers[*x].end(); ++y)
	{
	    if (m_killers[*y].find(*x) != m_killers[*y].end())
	    {
	        fillin.set(*x);
		fillin.set(*y);
		break;
	    }
	}
    }

    return fillin;
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
        if (Fillin(BLACK).test(i))
            os << "fb";
        else if (Fillin(WHITE).test(i))
            os << "fw";
        else if (Vulnerable().test(i)) 
        {
            os << "iv[";
            bool first=true;
            std::set<HexPoint>::const_iterator i;
            for (i = m_killers[p].begin(); i != m_killers[p].end(); ++i) 
            {
                if (!first) os << "-";
                os << *i;
                first = false;
            }
            os << "]";
        }
	else if (SReversible().test(i)) 
        {
            os << "is[";
            bool first=true;
            std::set<HexPoint>::const_iterator i;
            for (i = m_s_reversers[p].begin(); i != m_s_reversers[p].end(); ++i) 
            {
                if (!first) os << "-";
                os << *i;
                first = false;
            }
            os << "]";
        }
        else if (Inferior().test(i)) 
        {
            os << "ii[";
            bool first=true;
            std::set<HexPoint>::iterator i;
            for (i=m_inf_graph.out_begin(p); i!=m_inf_graph.out_end(p); ++i) 
            {
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

// Based on Kosaraju's algorithm
bitset_t 
InferiorCellsUtil::PrunableFromInferiorityGraph(const Digraph<HexPoint>& graph)
{
    Digraph<HexPoint> trans_graph;
    graph.Transpose(trans_graph);
    
    std::set<HexPoint> visited;
    std::set<HexPoint> killing;
    std::vector<HexPoint> stack;
    // First, a simple DFS on the transposed graph.
    for (std::set<HexPoint>::iterator it = trans_graph.Vertices().begin();
	 it != trans_graph.Vertices().end(); ++it)
    {
        if (visited.find(*it) != visited.end())
	    continue;
	trans_graph.DFS(*it, visited, killing, stack);
    }
    
    bitset_t prunable;
    visited.clear();
    std::vector<HexPoint> useless_stack;
    // Then, a DFS starting from the top of the stack.
    for (std::vector<HexPoint>::reverse_iterator it = stack.rbegin();
	 it != stack.rend(); ++it)
    {
        if (visited.find(*it) != visited.end())
	{
	    prunable.set(*it);
	    continue;
	}
        if (graph.DFS(*it, visited, killing, useless_stack))
	    prunable.set(*it);
	killing.insert(visited.begin(), visited.end());
	visited.clear();
    }
    return prunable;
}

//----------------------------------------------------------------------------
