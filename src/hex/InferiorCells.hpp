//----------------------------------------------------------------------------
/** @file
 */
//----------------------------------------------------------------------------

#ifndef INFERIOR_CELLS_HPP
#define INFERIOR_CELLS_HPP

#include "Hex.hpp"
#include "Digraph.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

class VulnerableKiller
{
public:

    /** Creates killer with empty carrier. */
    VulnerableKiller(HexPoint killer);

    /** Creates killer with given carrier. */
    VulnerableKiller(HexPoint killer, const bitset_t& carrier);

    HexPoint killer() const;
    bitset_t carrier() const;

    bool operator==(const VulnerableKiller& other) const;
    bool operator!=(const VulnerableKiller& other) const;
    bool operator<(const VulnerableKiller& other) const;

private:
    HexPoint m_killer;
    bitset_t m_carrier;
};

inline VulnerableKiller::VulnerableKiller(HexPoint killer)
    : m_killer(killer),
      m_carrier()
{
}

inline VulnerableKiller::VulnerableKiller(HexPoint killer, 
                                          const bitset_t& carrier)
    : m_killer(killer),
      m_carrier(carrier)
{
}

inline HexPoint VulnerableKiller::killer() const
{
    return m_killer;
}

inline bitset_t VulnerableKiller::carrier() const
{
    return m_carrier;
}

inline bool VulnerableKiller::operator==(const VulnerableKiller& other) const
{
    /** @todo This ignores the carrier. This means only the first (killer,
        carrier) pair is stored for each killer.  We may want to
        keep only the smallest carrier, or all of them.  */
    return (m_killer == other.m_killer);
}

inline bool VulnerableKiller::operator!=(const VulnerableKiller& other) const
{
    return !operator==(other);
}

inline bool VulnerableKiller::operator<(const VulnerableKiller& other) const
{
    if (m_killer != other.m_killer)
        return m_killer < other.m_killer;
    return false;
}

//----------------------------------------------------------------------------

/** Set of inferior cells. */
class InferiorCells
{
public:

    /** Constructs empty inferior cell set. */
    InferiorCells();
    
    //------------------------------------------------------------------------

    bitset_t Dead() const;
    bitset_t Captured(HexColor color) const;
    bitset_t PermInf(HexColor color) const;
    bitset_t PermInfCarrier(HexColor color) const;
    bitset_t MutualFillin(HexColor color) const;
    bitset_t MutualFillinCarrier(HexColor color) const;

    /** Returns the set of vulnerable cells. */
    bitset_t Vulnerable() const;

    /** Returns the set of reversible cells.

        @note An empty cell can be both reversible and vulnerable. In
        this case it will be vulnerable and not appear in the cells
        returned by Reversible(). */
    bitset_t Reversible() const;

    /** Returns the set of dominated cells.  This is not the same as
        the set of all cells dominated by some other cell. The
        returned is a maximal set of dominated cells that can be ignored
        during move

        @note A cell can be both dominated (have an outgoing arc in
        the domination graph), and be vulnerable (have an outgoing arc
        in the vulnerable graph) and/or reversible. In such a case, the
        cell will never appear in the cells returned by Dominated(). */

    bitset_t Dominated() const;

    bitset_t All() const;

    bitset_t Fillin(HexColor color) const;

    const std::set<VulnerableKiller>& Killers(HexPoint p) const;

    const std::set<HexPoint>& Reversers(HexPoint p) const;
    bitset_t AllReversers() const;
    bitset_t AllReversibleCarriers() const;
    
    //------------------------------------------------------------------------

    /** Returns a string representation of its internal state.
        The format is as follows:
        1)  First character is either f (for fill-in) or i (for ignorable)
        2a) If fill-in, 2nd character is c/d/p/u (captured/dead/perminf/mutual)
            and 3rd character is b/w (black/white fill-in colour)
        2b) If ignorable, 2nd character is v/r/d (vulnerable/reversible/domin)
            and 3rd entry is list of killers/reversers/dominators
     */
    std::string GuiOutput() const;

    /** Examines the vulnerable cells; returns the set of
        presimplicial cells on the cells they kill. */
    bitset_t FindPresimplicialPairs() const;

    /** Uses the inferior cell information to compute the deduction set.
        This is the part of the proof set used to derive the equivalent
        ICE-reduced board.
        The proof set is composed of this, the played stones, and (some
        subset of) the empty cells on the ICE-reduced board.
        Note: it is assumed the color passed in is the player for whom
        the pruning (vulnerable, reversible, dominated) was computed, so
        these are not used... just the fillin.
    */
    bitset_t DeductionSet(HexColor color) const;

    //------------------------------------------------------------------------

    void AddDead(const bitset_t& dead);
    void AddDead(HexPoint dead);
    
    void AddCaptured(HexColor color, const bitset_t& captured);
    void AddCaptured(HexColor color, HexPoint captured);

    void AddPermInf(HexColor color, const bitset_t& cells, 
                    const bitset_t& carrier);
    void AddPermInf(HexColor color, HexPoint cell, const bitset_t& carrier);
    
    void AddMutualFillin(HexColor color, const bitset_t& cells, 
                         const bitset_t& carrier);
    void AddMutualFillin(HexColor color, HexPoint cell,
                         const bitset_t& carrier);

    void AddDominated(HexPoint cell, HexPoint dominator);
    void AddDominated(HexPoint cell, const std::set<HexPoint>& dom);

    void AddVulnerable(HexPoint cell, HexPoint killer);
    void AddVulnerable(HexPoint cell, const std::set<HexPoint>& killers);
    void AddVulnerable(HexPoint cell, const VulnerableKiller& killer);
    void AddVulnerable(HexPoint cell, const std::set<VulnerableKiller>& dom);

    void AddReversible(HexPoint cell, bitset_t carrier, HexPoint reverser);
    void AddReversible(HexPoint cell, bitset_t carrier,
                       const std::set<HexPoint>& reversers);

    void AddDominatedFrom(const InferiorCells& other);
    void AddVulnerableFrom(const InferiorCells& other);
    void AddReversibleFrom(const InferiorCells& other);
    void AddPermInfFrom(HexColor color, const InferiorCells& other);
    void AddMutualFillinFrom(HexColor color, const InferiorCells& other);

    //------------------------------------------------------------------------

    void Clear();

    void ClearDead();
    void ClearVulnerable();
    void ClearReversible();
    void ClearDominated();
    void ClearCaptured(HexColor color);
    void ClearPermInf(HexColor color);
    void ClearMutualFillin(HexColor color);

    void RemoveDead(const bitset_t& dead);
    void RemoveCaptured(HexColor color, const bitset_t& captured);
    void RemoveDominated(const bitset_t& dominated);
    void RemoveVulnerable(const bitset_t& vulnerable);
    void RemoveReversible(const bitset_t& reversible);
    void RemoveReversible(HexPoint reversible);
    void RemovePermInf(HexColor color, const bitset_t& perminf);
    void RemoveMutualFillin(HexColor color, const bitset_t& mutual);
    
private:

    //------------------------------------------------------------------------

    void AssertPairwiseDisjoint() const;

    //------------------------------------------------------------------------

    bitset_t m_dead;

    bitset_t m_captured[BLACK_AND_WHITE];

    bitset_t m_perm_inf[BLACK_AND_WHITE];
    bitset_t m_perm_inf_carrier[BLACK_AND_WHITE];

    bitset_t m_mutual_fillin[BLACK_AND_WHITE];
    bitset_t m_mutual_fillin_carrier[BLACK_AND_WHITE];

    //------------------------------------------------------------------------

    /** Vulnerable cells; not those involved in presimplicial
        pairs, though. */
    bitset_t m_vulnerable;
    std::set<VulnerableKiller> m_killers[BITSETSIZE];
    
    //------------------------------------------------------------------------

    /** Reversible cells and their reversers. */
    bitset_t m_reversible;
    std::set<HexPoint> m_reversers[BITSETSIZE];
    /** Data to test for independent captured-reversible sets. */
    bitset_t m_allReversibleCarriers;
    bitset_t m_allReversers;

    //------------------------------------------------------------------------

    /** Graph of domination; dominated cells point to their
        dominators. */
    Digraph<HexPoint> m_dom_graph;

    /** True if the dominated set has been computed from the
        domination graph.  Set to false whenever the domination graph
        changes. */
    mutable bool m_dominated_computed;

    /** The sources of the domination graph minus a few
        representatives. */
    mutable bitset_t m_dominated;
    
};

inline bitset_t InferiorCells::Dead() const
{
    return m_dead;
}
    
inline bitset_t InferiorCells::Vulnerable() const
{
    return m_vulnerable;
}

inline bitset_t InferiorCells::Reversible() const
{
    return m_reversible;
}

inline bitset_t InferiorCells::Captured(HexColor color) const
{
    return m_captured[color];
}

inline bitset_t InferiorCells::PermInf(HexColor color) const
{
    return m_perm_inf[color];
}

inline bitset_t InferiorCells::PermInfCarrier(HexColor color) const
{
    return m_perm_inf_carrier[color];
}

inline bitset_t InferiorCells::MutualFillin(HexColor color) const
{
    return m_mutual_fillin[color];
}

inline bitset_t InferiorCells::MutualFillinCarrier(HexColor color) const
{
    return m_mutual_fillin_carrier[color];
}

inline 
const std::set<VulnerableKiller>& InferiorCells::Killers(HexPoint p) const
{
    return m_killers[p];
}

inline 
const std::set<HexPoint>& InferiorCells::Reversers(HexPoint p) const
{
    return m_reversers[p];
}

inline bitset_t InferiorCells::AllReversers() const
{
    return m_allReversers;
}

inline bitset_t InferiorCells::AllReversibleCarriers() const
{
    return m_allReversibleCarriers;
}

//------------------------------------------------------------------------

/** Utilities on InferiorCells. */
namespace InferiorCellsUtil
{

    bitset_t FindDominationCaptains(const Digraph<HexPoint>& graph);

}

//------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // INFERIOR_CELLS_HPP
