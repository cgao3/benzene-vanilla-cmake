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
    /** Generate pattern moves. */
    bool patternHeuristic;

    /** Play learned responses. */    
    bool responseHeuristic;

    int pattern_update_radius;

    /** Percent chance to check for pattern moves. */
    int pattern_check_percent;

    /** Threshold at which the reponse heuristic is used. */
    std::size_t response_threshold;

    HexUctPolicyConfig();
};

/** Statistics over all threads. */
struct HexUctPolicyStatistics
{
    std::size_t total_moves;

    std::size_t random_moves;

    std::size_t pattern_moves;

    std::map<const Pattern*, size_t> pattern_counts[BLACK_AND_WHITE];

    std::map<const Pattern*, size_t> pattern_picked[BLACK_AND_WHITE];

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

    //----------------------------------------------------------------------

    /** Returns reference to configuration settings controlling all
        policies. */
    HexUctPolicyConfig& Config();

    /** Returns constant reference to configuration settings
        controlling all policies. */
    const HexUctPolicyConfig& Config() const;

private:

    HexUctPolicyConfig m_config;

    std::vector<Pattern> m_patterns[BLACK_AND_WHITE];

    HashedPatternSet m_hash_patterns[BLACK_AND_WHITE];

    //----------------------------------------------------------------------

    void LoadPlayPatterns(const std::string& filename);
};

inline HexUctPolicyConfig& HexUctSharedPolicy::Config()
{
    return m_config;
}

inline const HexUctPolicyConfig& HexUctSharedPolicy::Config() const
{
    return m_config;
}

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
    HexUctPolicy(const HexUctSharedPolicy* shared);

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

    void InitializeForSearch();

    void AddResponse(HexColor toPlay, HexPoint lastMove, HexPoint response);

#if COLLECT_PATTERN_STATISTICS    
    /** Returns a string containing formatted statistics
        information. */
    std::string DumpStatistics();

    /** Returns the collected statistics. */
    const HexUctPolicyStatistics& Statistics() const;
#endif

private:

    static const int MAX_VOTES = 1024;

    const HexUctSharedPolicy* m_shared;

    std::vector<HexPoint> m_moves;

    std::vector<HexPoint> m_response[BLACK_AND_WHITE][BITSETSIZE];

    /** Generator for this policy. */
    SgRandom m_random;

#if COLLECT_PATTERN_STATISTICS
    HexUctPolicyStatistics m_statistics;
#endif

    //----------------------------------------------------------------------

    HexPoint PickRandomPatternMove(const PatternState& pastate, 
                                   const HashedPatternSet& patterns, 
                                   HexColor toPlay, 
                                   HexPoint lastMove);

    HexPoint GeneratePatternMove(const PatternState& pastate, HexColor color, 
                                 HexPoint lastMove);

    HexPoint GenerateResponseMove(HexColor toPlay, HexPoint lastMove,
                                  const StoneBoard& brd);

    HexPoint GenerateRandomMove(const StoneBoard& brd);
};

inline void HexUctPolicy::AddResponse(HexColor toPlay, HexPoint lastMove,
                                      HexPoint response)
{
    if (m_shared->Config().responseHeuristic)
        m_response[toPlay][lastMove].push_back(response);
}

#if COLLECT_PATTERN_STATISTICS
inline const HexUctPolicyStatistics& HexUctPolicy::Statistics() const
{
    return m_statistics;
}
#endif

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // HEXUCTPOLICY_H
