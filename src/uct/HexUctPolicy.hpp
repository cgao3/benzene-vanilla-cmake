//----------------------------------------------------------------------------
/** @file HexUctPolicy.hpp
 */
//----------------------------------------------------------------------------

#ifndef HEXUCTPOLICY_H
#define HEXUCTPOLICY_H

#include "SgSystem.h"
#include "SgRandom.h"

#include "HexUctSearch.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Whether statistics on patterns should be collected or not.  This
    information is pretty much useless and slows down the search. */
#define COLLECT_PATTERN_STATISTICS 0

//----------------------------------------------------------------------------

/** Configuration options for policies. */
struct HexUctPolicyConfig
{
    bool patternHeuristic;

    int pattern_update_radius;

    int pattern_check_percent;

    HexUctPolicyConfig();
};

/** Statistics over all threads. */
struct HexUctPolicyStatistics
{
    int total_moves;

    int random_moves;

    int pattern_moves;

    std::map<const Pattern*, int> pattern_counts[BLACK_AND_WHITE];

    std::map<const Pattern*, int> pattern_picked[BLACK_AND_WHITE];

    HexUctPolicyStatistics()
        : total_moves(0),
          random_moves(0),
          pattern_moves(0)
    { }
};

/** Policy information shared amoung all threads. */
class HexUctSharedPolicy
{
public:

    /** Constructor. */
    HexUctSharedPolicy();

    /** Destructor. */
    ~HexUctSharedPolicy();

    //----------------------------------------------------------------------

    /** Loads patterns from shared directory. */
    void LoadPatterns();

    /** Returns set of patterns used to guide playouts. */
    const HashedPatternSet& PlayPatterns(HexColor color) const;

#if COLLECT_PATTERN_STATISTICS    
    /** Returns a string containing formatted statistics
        information. */
    std::string DumpStatistics();
#endif

    //----------------------------------------------------------------------

    /** Returns constant reference to configuration settings
        controlling all policies. */
    const HexUctPolicyConfig& Config() const;

#if COLLECT_PATTERN_STATISTICS    
    /** Returns reference to current statistics so that threads can
        update this information. */        
    HexUctPolicyStatistics& Statistics();
#endif
   
private:

    HexUctPolicyConfig m_config;

#if COLLECT_PATTERN_STATISTICS
    HexUctPolicyStatistics m_statistics;
#endif

    std::vector<Pattern> m_patterns[BLACK_AND_WHITE];

    HashedPatternSet m_hash_patterns[BLACK_AND_WHITE];

    //----------------------------------------------------------------------

    void LoadPlayPatterns(const std::string& filename);
};

inline const HexUctPolicyConfig& HexUctSharedPolicy::Config() const
{
    return m_config;
}

#if COLLECT_PATTERN_STATISTICS
inline HexUctPolicyStatistics& HexUctSharedPolicy::Statistics()
{
    return m_statistics;
}
#endif

inline const HashedPatternSet& 
HexUctSharedPolicy::PlayPatterns(HexColor color) const
{
    return m_hash_patterns[color];
}

//----------------------------------------------------------------------------

/** Generates moves during the random playout phase of UCT search.
    Uses local configuration and pattern data in HexUctSharedPolicy.
    Everything in this class must be thread-safe. 
*/
class HexUctPolicy : public HexUctSearchPolicy
{
public:

    /** Constructor. Creates a policy. */
    HexUctPolicy(HexUctSharedPolicy* shared);

    /* Destructor. */
    ~HexUctPolicy();

    /** Implementation of SgUctSearch::GenerateRandomMove().
        - Pattern move (if enabled)
        - Purely random
    */
    HexPoint GenerateMove(PatternState& pastate, HexColor color, 
                          HexPoint lastMove);

    /** Initializes the moves to generate from the empty cells on the
        given board. Should be called with the boardstate before any
        calls to GenerateMove(). */
    void InitializeForRollout(const StoneBoard& brd);

private:

    static const int MAX_VOTES = 1024;

    HexUctSharedPolicy* m_shared;

    std::vector<HexPoint> m_moves;

    /** Generator for this policy. */
    SgRandom m_random;

    //----------------------------------------------------------------------

    HexPoint PickRandomPatternMove(const PatternState& pastate, 
                                   const HashedPatternSet& patterns, 
                                   HexColor toPlay, 
                                   HexPoint lastMove);

    HexPoint GeneratePatternMove(const PatternState& pastate, HexColor color, 
                                 HexPoint lastMove);

    HexPoint GenerateRandomMove(const StoneBoard& brd);
};

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // HEXUCTPOLICY_H
