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
#include "HandCodedPattern.hpp"
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

    /** Categorizes cells as dead, captured, etc. Board will be
        modified with the fill-in.
    */
    void ComputeInferiorCells(HexColor color, Groups& board,
                              PatternState& pastate, 
                              InferiorCells& out) const;
    
    /** Computes fill-in; dominated and vulnerable cells are not
        stored.
    */
    void ComputeFillin(HexColor color, Groups& board, 
                       PatternState& pastate, InferiorCells& out,
                       HexColorSet colors_to_capture=ALL_COLORS) const;

    /** Computes only the dead and captured cells; board will be
        modified to have the captured cells filled-in. Returns number of
        cells filled-in.
    */
    int ComputeDeadCaptured(Groups& board, PatternState& pastate,
                            InferiorCells& inf, 
                            HexColorSet colors_to_capture) const;
    
    // @} // @name

    //------------------------------------------------------------------------

    /** @name Methods to find various types of inferior cells */
    // @{ 

    /** Returns the dead cells among the consider set. */
    bitset_t FindDead(const PatternState& board, 
                      const bitset_t& consider) const;

    /** Finds vulnerable cells for color among the consider set. */
    void FindVulnerable(const PatternState& board, HexColor color,
			const bitset_t& consider,
			InferiorCells& inf) const;

    /** Finds reversible cells for color among the consider set. */
    void FindReversible(const PatternState& board, HexColor color,
			const bitset_t& consider,
			InferiorCells& inf) const;

    /** Finds dominated cells for color among the consider set using
        local patterns. Calls FindHandCodedDominated(). */
    void FindDominated(const PatternState& board, HexColor color, 
                       const bitset_t& consider, InferiorCells& inf) const;

    /** Finds all dominated cell patterns for color on this one cell. */
    void FindDominatedOnCell(const PatternState& pastate,
                             HexColor color, 
                             HexPoint cell,
                             PatternHits& hits) const;

    /** Finds cells dominated via hand-coded patterns. */
    void FindHandCodedDominated(const StoneBoard& board, HexColor color,
                                const bitset_t& consider, 
                                InferiorCells& inf) const;

    /** Finds captured cells for color among the consider set using
        local patterns. */
    bitset_t FindCaptured(const PatternState& board, HexColor color, 
                          const bitset_t& consider) const;

    /** Finds the permanently inferior cells for color among consider
        set using local patterns. */
    bitset_t FindPermanentlyInferior(const PatternState& board, 
                                     HexColor color, 
                                     const bitset_t& consider,
                                     bitset_t& carrier) const;

    // @} // @name
    
    //------------------------------------------------------------------------

    /** @name Parameters */
    // @{ 

    /** @todo Document what presimplicial pairs are! */
    bool FindPresimplicialPairs() const;

    /** @see FindPresimplicialPairs() */
    void SetFindPresimplicialPairs(bool enable);

    /** @todo Document permanently inferior cells. */
    bool FindPermanentlyInferior() const;

    /** @see FindPermanentlyInferior() */
    void SetFindPermanentlyInferior(bool enable);

    /** Find all killers for each cell if true, stop at first if false. */
    bool FindAllPatternKillers() const;

    /** @see FindAllPatternKillers() */
    void SetFindAllPatternKillers(bool enable);

    /** Find all reversers for each cell if true, stop at first if false. */
    bool FindAllPatternReversers() const;

    /** @see FindAllPatternReversers() */
    void SetFindAllPatternReversers(bool enable);

    /** Find all dominators for each cell if true, stop at first if false. */
    bool FindAllPatternDominators() const;

    /** @see FindAllPatternDominators() */
    void SetFindAllPatternDominators(bool enable);

    /** @todo Document hand coded patterns. */
    bool UseHandCodedPatterns() const;

    /** @see UseHandCodedPatterns() */
    void SetUseHandCodedPatterns(bool enable);

    /** Performs a 1-ply search for the opponent: any dead stones
        created in child states are backed-up as vulnerable cells in
        this state, with the killer set to all the created fillin.*/
    bool BackupOpponentDead() const;

    /** @see BackupOpponentDead() */
    void SetBackupOpponentDead(bool enable);

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

    // @}

private:

    /** Loads local patterns from "ice-pattern-file". */
    void LoadPatterns();

    /** Creates the set of hand-coded patterns. */
    void LoadHandCodedPatterns();

    //------------------------------------------------------------

    /** Calls FindPermanentlyInferior() and adds any found to the
        board and the set of inferior cells. 
    */
    int FillinPermanentlyInferior(Groups& groups, 
                                  PatternState& board, HexColor color,
                                  InferiorCells& out, 
                                  HexColorSet colors_to_capture) const; 

    /** Calls ComputeDeadRegions() and FindThreeSetCliques() and adds
        fill-in to board and set of inferior cells.
    */
    int CliqueCutsetDead(Groups& groups, PatternState& pastate, 
                         InferiorCells& out) const;

    /** For each empty cell on the board, the move is played with the
        opponent's stone (ie, !color) and the fill-in is computed.  Any
        dead cells in this state are backed-up as vulnerable cells in the
        original state, with the set of captured stones as the
        vulnerable-carrier.  This can be moderately expensive.
        
        @todo Link to the "ice-backup-opp-dead" option, or link it's
        documentation here.
    */
    int BackupOpponentDead(HexColor color, const StoneBoard& board, 
                           PatternState& pastate, InferiorCells& out) const;

    /** Finds vulnerable cells for color and finds presimplicial pairs
        and fills them in for the other color.  Simplicial stones will
        be added as dead and played to the board as DEAD_COLOR. */
    int FillInVulnerable(HexColor color, Groups& groups, 
                         PatternState& pastate, InferiorCells& inf, 
                         HexColorSet colors_to_capture) const;

    //------------------------------------------------------------

    /** Handles color flipping/rotations for this hand-coded pattern.
        If pattern matches, dominators are added to inf.
    */
    void CheckHandCodedDominates(const StoneBoard& brd, HexColor color,
                                 const HandCodedPattern& pattern, 
                                 const bitset_t& consider, 
                                 InferiorCells& inf) const;

    //------------------------------------------------------------

    /** @see FindPresimplicialPairs() */
    bool m_find_presimplicial_pairs;

    /** @see FindPermanentlyInferior() */
    bool m_find_permanently_inferior;

    /** @see FindAllPatternKillers() */
    bool m_find_all_pattern_killers;

    /** @see FindAllPatternReversers() */
    bool m_find_all_pattern_reversers;

    /** @see FindAllPatternDominators() */
    bool m_find_all_pattern_dominators;

    /** @see UseHandCodedPatterns() */
    bool m_use_handcoded_patterns;

    /** @see BackupOpponentDead() */
    bool m_backup_opponent_dead;

    /** @see FindThreeSidedDeadRegions() */
    bool m_find_three_sided_dead_regions;

    /** @see IterativeDeadRegions() */
    bool m_iterative_dead_regions;
    
    //------------------------------------------------------------

    std::vector<HandCodedPattern> m_hand_coded;

    IcePatternSet m_patterns;
};

inline bool ICEngine::FindPresimplicialPairs() const
{
    return m_find_presimplicial_pairs;
}

inline void ICEngine::SetFindPresimplicialPairs(bool enable)
{
    m_find_presimplicial_pairs = enable;
}

inline bool ICEngine::FindPermanentlyInferior() const
{
    return m_find_permanently_inferior;
}

inline void ICEngine::SetFindPermanentlyInferior(bool enable)
{
    m_find_permanently_inferior = enable;
}

inline bool ICEngine::FindAllPatternKillers() const
{
    return m_find_all_pattern_killers;
}

inline void ICEngine::SetFindAllPatternKillers(bool enable)
{
    m_find_all_pattern_killers = enable;
}

inline bool ICEngine::FindAllPatternReversers() const
{
    return m_find_all_pattern_reversers;
}

inline void ICEngine::SetFindAllPatternReversers(bool enable)
{
    m_find_all_pattern_reversers = enable;
}

inline bool ICEngine::FindAllPatternDominators() const
{
    return m_find_all_pattern_dominators;
}

inline void ICEngine::SetFindAllPatternDominators(bool enable)
{
    m_find_all_pattern_dominators = enable;
}

inline bool ICEngine::UseHandCodedPatterns() const
{
    return m_use_handcoded_patterns;
}

inline void ICEngine::SetUseHandCodedPatterns(bool enable)
{
    m_use_handcoded_patterns = enable;
}

inline bool ICEngine::BackupOpponentDead() const
{
    return m_backup_opponent_dead;
}

inline void ICEngine::SetBackupOpponentDead(bool enable)
{
    m_backup_opponent_dead = enable;
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

//----------------------------------------------------------------------------

/** Utilities needed by ICE. */
namespace IceUtil 
{
    /** Adds the inferior cell info from in to the inferior cell info
        of out.
    */
    void Update(InferiorCells& out, const InferiorCells& in);
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // ICENGINE_H
