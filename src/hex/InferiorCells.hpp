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

/** Set of inferior cells. */
class InferiorCells
{
public:

    /** Constructs empty inferior cell set. */
    InferiorCells();
    
    //------------------------------------------------------------------------

    bitset_t Fillin(HexColor color) const;

    /** Vulnerables could be included in SReversible, but given they block no
	pattern separating them allows to detect more pruning. */
    bitset_t Vulnerable() const;

    /** Also contains the threat-reversible as they are handled together.
	Actually, most following "SRev" functions also include them,
	unless there is a corresponding "TRev" one. */
    bitset_t SReversible() const;

    /** Returns the set of inferior cells.  This is not the same as
        the set of all cells inferior to some other cell. The
        returned is a maximal set of inferior cells that can be pruned
        during move

        @note A cell can be both inferior (have an outgoing arc in the
        inferiority graph) and strong-reversible. In such a case, the
        cell will never appear in the cells returned by Inferior(). */
    bitset_t Inferior() const;

    bitset_t All() const;

    const std::set<HexPoint>& Killers(HexPoint p) const;
    const std::set<HexPoint>& SReversers(HexPoint p) const;

    /** The blocker is:
	- for a strong-reverse pattern, the reverser
	- for a threat-reverse pattern, the reverser and the reversible
    */
    const bitset_t Blockers() const;
    /** The carrier is the set of all cells except the reverser and
	the reversible. */
    const bitset_t SReversibleCarriers() const;
    
    //------------------------------------------------------------------------

    /** Returns a string representation of its internal state.
        The format is as follows:
        1)  First character is either f (for fill-in) or i (for ignorable)
        2a) If fill-in, 2nd character is b/w (black/white)
        2b) If ignorable, 2nd character is v/s/i (vulnerable/strong-reversible
	  /inferior) and 3rd entry is list of killers/reversers/superiors
    */
    std::string GuiOutput() const;

    /** Examines the reversible cells; returns the set of
        presimplicial cells on the cells they kill. */
    bitset_t FindPresimplicialPairs() const;

    //------------------------------------------------------------------------

    void AddFillin(HexColor color, HexPoint fillin);
    void AddFillin(HexColor color, const bitset_t& fillin);

    /** A cell is vulnerable when it is strong-reversible with no carrier. */
    void AddVulnerable(HexPoint vulnerable,
		       HexPoint killer);
    void AddVulnerable(HexPoint vulnerable,
		       const std::set<HexPoint>& killers);

    /** Theis function test if the cell is already known to be strong-
	reversible (or vulnerable). In this case, it does nothing. */
    void AddSReversible(HexPoint reversible,
			const bitset_t& carrier,
			HexPoint reverser,
			bool is_threat);

    void AddInferior(HexPoint inferior,
		     HexPoint superior);
    void AddInferior(HexPoint inferior,
		     const std::set<HexPoint>& superiors);

    /** Make sure to have cleared before calling these. */
    void AddVulnerableFrom(const InferiorCells& other);
    void AddSReversibleFrom(const InferiorCells& other);
    void AddInferiorFrom(const InferiorCells& other);
  
    //------------------------------------------------------------------------

    void Clear();

    void ClearFillin(HexColor color);
    void ClearVulnerable();
    void ClearSReversible();
    void ClearInferior();

    void RemoveFillin(HexColor color, HexPoint fillin);
    void RemoveFillin(HexColor color, const bitset_t& fillin);

    void RemoveVulnerable(HexPoint vunerable);
    void RemoveVulnerable(const bitset_t& vulnerable);
    void RemoveSReversible(HexPoint reversible);
    void RemoveSReversible(const bitset_t& reversible);

    void RemoveInferior(HexPoint inferior);
    void RemoveInferior(const bitset_t& inferior);
    
private:

    //------------------------------------------------------------------------

    void AssertPairwiseDisjoint() const;

    //------------------------------------------------------------------------

    bitset_t m_fillin[BLACK_AND_WHITE];

    bitset_t m_vulnerable;
    std::set<HexPoint> m_killers[BITSETSIZE];
    bitset_t m_s_reversible;
    std::set<HexPoint> m_s_reversers[BITSETSIZE];

    bitset_t m_blockers;
    bitset_t m_s_reversible_carriers;

    //------------------------------------------------------------------------

    /** Graph of domination; dominated cells point to their
        dominators. */
    Digraph<HexPoint> m_inf_graph;

    /** True if the inferior set has been computed from the
        inferiority graph.  Set to false whenever the inferiority
        graph changes. */
    mutable bool m_inferior_computed;

    /** The sources of the inferiority graph minus a few
        representatives. */
    mutable bitset_t m_inferior;
    
};
    
inline bitset_t InferiorCells::Fillin(HexColor color) const
{
    return m_fillin[color];
}

inline bitset_t InferiorCells::Vulnerable() const
{
    return m_vulnerable;
}

inline bitset_t InferiorCells::SReversible() const
{
    return m_s_reversible;
}

inline 
const std::set<HexPoint>& InferiorCells::Killers(HexPoint p) const
{
    return m_killers[p];
}

inline 
const std::set<HexPoint>& InferiorCells::SReversers(HexPoint p) const
{
    return m_s_reversers[p];
}

inline 
const bitset_t InferiorCells::Blockers() const
{
    return m_blockers;
}

inline 
const bitset_t InferiorCells::SReversibleCarriers() const
{
    return m_s_reversible_carriers;
}

//------------------------------------------------------------------------

/** Utilities on InferiorCells. */
namespace InferiorCellsUtil
{

    bitset_t PrunableFromInferiorityGraph(const Digraph<HexPoint>& graph);

}

//------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // INFERIOR_CELLS_HPP
