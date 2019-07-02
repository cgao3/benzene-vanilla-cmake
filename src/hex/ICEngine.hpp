//----------------------------------------------------------------------------
/** @file
 */
//----------------------------------------------------------------------------

#ifndef ICENGINE_H
#define ICENGINE_H

#include "Digraph.hpp"
#include "Groups.hpp"
#include "InferiorCells.hpp"
#include "IcePatternSet.hpp"
#include "PatternState.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Inferior Cell Engine. 
    Finds inferior cells on a given boardstate.

    ICE is thread-safe. Multiple threads can use the same instance
    of ICEngine without any problems. 
*/
class ICEngine
{
public:

    //------------------------------------------------------------------------

    /** Constructor. */
    ICEngine();

    /** Destructor. */
    virtual ~ICEngine();

    //------------------------------------------------------------------------
    
    /** @name Board modifying functions of ICEngine */
    // @{

    /** Categorizes cells as fillined, inferior, etc. for color.
	If last_move is valid, then it return a reverser if there is one,
	else it returns INVALID_POINT. */
    HexPoint ComputeInferiorCells(HexColor color, Groups& groups,
				  PatternState& pastate, 
				  InferiorCells& inf,
				  HexPoint last_move=INVALID_POINT,
				  bool only_around_last_move=false) const;
  
    /** The different fillin modes that can be used for the next functions.
	In MONOCOLOR_USING_CAPTURED, captured cells are also computed for
	the othe color, but at the end they are cleared. This mode is stronger
	than MONOCOLOR but slower. This can be disabled, @see UseCapture.

	/!\ : MONOCOLOR fillin mode may result in more winning moves for
	player color, which after inferior pruning may cause incorrect results.
	Thus, you should use it only if:
	- you will not compute any inferior cells on it (for instance if you
	are just trying to find new inferior results);
	- color is not the next to move;
	- you don't care about rigour (for instance in the player) (indeed, in
	practice, this hardly ever fails). */
    typedef enum { MONOCOLOR, MONOCOLOR_USING_CAPTURED, BICOLOR } FillinMode;
  
    /** Computes fillin, and returns the number of cells fillined.
	If consider is given, then the fillin is computed from consider. 
	If last_move is given, then the fillin is computed around it. This
        is equivalent to calling it with consider equal to the set of empty
	cells close to it..*/
    std::size_t ComputeFillin(
			Groups& groups, PatternState& pastate,
			InferiorCells& inf, HexColor color,
			FillinMode mode=BICOLOR) const ;
    std::size_t ComputeFillin(
			Groups& groups, PatternState& pastate,
			InferiorCells& inf, HexColor color,
			FillinMode mode,
			HexPoint last_move) const ;
    std::size_t ComputeFillin(
			Groups& groups, PatternState& pastate,
			InferiorCells& inf, HexColor color,
			FillinMode mode,
		        const bitset_t& consider,
			bool clear_inf=true) const ;
  
    std::size_t ComputePatternFillin(
		        PatternState& pastate,
			InferiorCells& inf, HexColor color,
			FillinMode mode,
		        const bitset_t& consider) const;
  
    // @} // @name

    //------------------------------------------------------------------------

    /** @name Methods to find various types of inferior cells */
    // @{ 

    /** Finds strong-reversible (and vulnerable) cells for color
	among the consider set. */
    void FindSReversible(const PatternState& pastate, HexColor color,
			 const bitset_t& consider,
			 InferiorCells& inf) const;
    /** Finds threat-reversible cells for color
	among the consider set. */
    void FindTReversible(const PatternState& pastate, HexColor color,
			 const bitset_t& consider,
			 InferiorCells& inf) const;

    /** Finds vulnerable cells for color among the consider set. */
    void FindVulnerable(const PatternState& pastate, HexColor color,
			const bitset_t& consider,
			InferiorCells& inf) const;

    /** Finds inferior cells for color among the consider set using
	local patterns. */
    void FindInferior(const PatternState& board, HexColor color, 
                      const bitset_t& consider, InferiorCells& inf) const;

    /** The following two functions use MAX_EXTENSION_MOVES1, and thus may
	miss a few patterns. */
  
    /** If the cell is (non necessarily strongly) reversible, then
	returns a reverser, else returns INVALID_POINT.
	The cell can be empty or not.
	/!\ : if p is occupied and fillin has been computed using it,
	this function may (of course) return incorrect result. */
    HexPoint IsReversible(PatternState& pastate,
			  HexColor color, HexPoint p) const;
  
    /** Finds all inferior cell patterns for color on this one cell. */
    PatternHits FindInferiorOnCell(const PatternState& pastate,
				   HexColor color, 
				   HexPoint cell) const;

    // @} // @name
    
    //------------------------------------------------------------------------

    /** @name Parameters */
    // @{ 

    /** @todo Document what presimplicial pairs are! */
    bool FindPresimplicialPairs() const;

    /** @see FindPresimplicialPairs() */
    void SetFindPresimplicialPairs(bool enable);

    /** Find all killers for each cell if true, stop at first if false. */
    bool FindAllPatternKillers() const;

    /** @see FindAllPatternKillers() */
    void SetFindAllPatternKillers(bool enable);

    /** Find all superiors for each cell if true, stop at first if false. */
    bool FindAllPatternSuperiors() const;

    /** @see FindAllPatternSuperiors() */
    void SetFindAllPatternSuperiors(bool enable);

    /** @todo Document three sided dead regions. */
    bool FindThreeSidedDeadRegions() const;

    /** @see FindThreeSidedDeadRegions() */
    void SetFindThreeSidedDeadRegions(bool enable);

    /** Performs a dead region sweep on each iteration of
        the fillin loop if true, only at the end if false. 
        @todo Link to fillin algo documentation!
    */
    bool IterativeDeadRegions() const;

    /** @see IterativeDeadRegions() */
    void SetIterativeDeadRegions(bool enable);

    /** If disabled, MONOCOLOR_USING_CAPTURE is treated as
        MONOCOLOR in ComputeFillin(). */
    bool UseCapture() const;

    /** @see UseCapture() */
    void SetUseCapture(bool enable);
  
    /** If disabled, no reverser is computed by ComputeInferiorCells(). */
    bool FindReversible() const;

    /** @see FindReversible() */
    void SetFindReversible(bool enable);

    /** If disabled, strongly reversible patterns are not use in
	IsReversible(). */
    bool UseSReversibleAsReversible() const;

    /** @see UseSReversibleAsReversible() */
    void SetUseSReversibleAsReversible(bool enable);

    // @}

private:
    /** @see FindPresimplicialPairs() */
    bool m_find_presimplicial_pairs;

    /** @see FindAllPatternKillers() */
    bool m_find_all_pattern_killers;

    /** @see FindAllPatternSuperiors() */
    bool m_find_all_pattern_superiors;

    /** @see FindThreeSidedDeadRegions() */
    bool m_find_three_sided_dead_regions;

    /** @see IterativeDeadRegions() */
    bool m_iterative_dead_regions;

    /** @see UseCapture() */
    bool m_use_capture;

    /** @see FindReversible() */
    bool m_find_reversible;

    /** @see UseSReversibleAsReversible() */
    bool m_use_s_reversible_as_reversible;

    IcePatternSet m_patterns;

    void LoadPatterns();

    std::size_t CliqueCutsetDead(HexColor color, Groups& groups,
				 PatternState& pastate, 
                                 InferiorCells& inf) const;

    std::size_t FillInVulnerable(HexColor color, Groups& groups, 
                                 PatternState& pastate,
				 InferiorCells& inf) const;

    static FillinMode TurnOffCapture(FillinMode mode);
    static bool UsesCapture(FillinMode mode);
    static bool IsMonocolorUsingCapture(FillinMode mode);
    static bool Bicolor(FillinMode mode);

};

inline bool ICEngine::FindPresimplicialPairs() const
{
    return m_find_presimplicial_pairs;
}

inline void ICEngine::SetFindPresimplicialPairs(bool enable)
{
    m_find_presimplicial_pairs = enable;
}

inline bool ICEngine::FindAllPatternKillers() const
{
    return m_find_all_pattern_killers;
}

inline void ICEngine::SetFindAllPatternKillers(bool enable)
{
    m_find_all_pattern_killers = enable;
}

inline bool ICEngine::FindAllPatternSuperiors() const
{
    return m_find_all_pattern_superiors;
}

inline void ICEngine::SetFindAllPatternSuperiors(bool enable)
{
    m_find_all_pattern_superiors = enable;
}

inline bool ICEngine::FindThreeSidedDeadRegions() const
{
    return m_find_three_sided_dead_regions;
}

inline void ICEngine::SetFindThreeSidedDeadRegions(bool enable)
{
    m_find_three_sided_dead_regions = enable;
}

inline bool ICEngine::IterativeDeadRegions() const
{
    return m_iterative_dead_regions;
}

inline void ICEngine::SetIterativeDeadRegions(bool enable)
{
    m_iterative_dead_regions = enable;
}

inline bool ICEngine::UseCapture() const
{
    return m_use_capture;
}

inline void ICEngine::SetUseCapture(bool enable)
{
    m_use_capture = enable;
}

inline bool ICEngine::FindReversible() const
{
    return m_find_reversible;
}

inline void ICEngine::SetFindReversible(bool enable)
{
    m_find_reversible = enable;
}

inline bool ICEngine::UseSReversibleAsReversible() const
{
    return m_use_s_reversible_as_reversible;
}

inline void ICEngine::SetUseSReversibleAsReversible(bool enable)
{
    m_use_s_reversible_as_reversible = enable;
}

//----------------------------------------------------------------------------

/** Utilities needed by ICE. */
namespace IceUtil 
{
    /** Adds the inferior cell info from in to the inferior cell info
        of out. */
    void Update(InferiorCells& out, const InferiorCells& in);
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // ICENGINE_H
