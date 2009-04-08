//----------------------------------------------------------------------------
// $Id: HexUctPolicy.hpp 1855 2009-01-21 02:22:43Z broderic $ 
//----------------------------------------------------------------------------

#ifndef HEXUCTPOLICY_H
#define HEXUCTPOLICY_H

#include "SgSystem.h"
#include "SgRandom.h"

#include "HexUctSearch.hpp"

//----------------------------------------------------------------------------

/** Whether statistics on patterns should be collected or not.  This
    information is pretty much useless and slows down the search. 
*/
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

    /** @todo Are these threadsafe? */
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

    /** Loads patterns; config_dir is the location of bin/config/. */
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

    //----------------------------------------------------------------------
    
    HexUctPolicyConfig m_config;

#if COLLECT_PATTERN_STATISTICS
    HexUctPolicyStatistics m_statistics;
#endif

    //----------------------------------------------------------------------

    /** Loads patterns for use in rollouts. */
    void LoadPlayPatterns(const std::string& filename);

    std::vector<Pattern> m_patterns[BLACK_AND_WHITE];
    HashedPatternSet m_hash_patterns[BLACK_AND_WHITE];
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
    virtual ~HexUctPolicy();

    /** Implementation of SgUctSearch::GenerateRandomMove().
        - Pattern move (if enabled)
        - Purely random
    */
    virtual HexPoint GenerateMove(PatternBoard& brd, 
                                  HexColor color, 
                                  HexPoint lastMove);

    //------------------------------------------------------------

    /** Initializes the moves to generate from the empty cells on the
        given board. Should be called with the boardstate before any
        calls to GenerateMove(). */
    void InitializeForRollout(const StoneBoard& brd);

private:

    static const int MAX_VOTES = 1024;

    //----------------------------------------------------------------------

    /** Randomly picks a pattern move from the set of patterns that hit
        the last move, weighted by the pattern's weight. 
        If no pattern matches, returns INVALID_POINT. */
    HexPoint PickRandomPatternMove(const PatternBoard& brd, 
                                   const HashedPatternSet& patterns, 
                                   HexColor toPlay, 
                                   HexPoint lastMove);

    /** Uses PickRandomPatternMove() with the shared PlayPatterns(). */
    HexPoint GeneratePatternMove(const PatternBoard& brd, HexColor color, 
                                 HexPoint lastMove);

    //----------------------------------------------------------------------

    /** Selects random move among the empty cells on the board. */
    HexPoint GenerateRandomMove(const PatternBoard& brd);

    //----------------------------------------------------------------------

    HexUctSharedPolicy* m_shared;

    std::vector<HexPoint> m_moves;

    /** Generator for this policy. */
    SgRandom m_random;
};

//----------------------------------------------------------------------------

#endif // HEXUCTPOLICY_H
