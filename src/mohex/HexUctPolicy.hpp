//----------------------------------------------------------------------------
/** @file HexUctPolicy.hpp */
//----------------------------------------------------------------------------

#ifndef HEXUCTPOLICY_H
#define HEXUCTPOLICY_H

#include "SgSystem.h"
#include "SgRandom.h"

#include "HexUctSearch.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Whether statistics on patterns should be collected or not. 
    Only use if debugging the policy as collecting the statistics greatly
    impacts performance. */
#define COLLECT_PATTERN_STATISTICS 0

//----------------------------------------------------------------------------

/** Configuration options for policies. */
struct HexUctPolicyConfig
{
    /** Generate pattern moves. */
    bool patternHeuristic;

    /** Percent chance to check for pattern moves. */
    int patternCheckPercent;

    /** Play learned responses. */    
    bool responseHeuristic;

    /** Threshold at which the reponse heuristic is used. */
    std::size_t responseThreshold;

    HexUctPolicyConfig();
};

inline HexUctPolicyConfig::HexUctPolicyConfig()
    : patternHeuristic(true),
      patternCheckPercent(100),
      responseHeuristic(false),
      responseThreshold(100)
{
}

//----------------------------------------------------------------------------

/** Statistics for a policy. */
struct HexUctPolicyStatistics
{
    std::size_t totalMoves;

    std::size_t randomMoves;

    std::size_t patternMoves;

    std::map<const Pattern*, size_t> patternCounts[BLACK_AND_WHITE];

    std::map<const Pattern*, size_t> patternPicked[BLACK_AND_WHITE];

    HexUctPolicyStatistics();
};

inline HexUctPolicyStatistics::HexUctPolicyStatistics()
    : totalMoves(0),
      randomMoves(0),
      patternMoves(0)
{
}

//----------------------------------------------------------------------------

/** Policy information shared among all threads. */
class HexUctSharedPolicy
{
public:
    HexUctSharedPolicy();

    ~HexUctSharedPolicy();

    /** Loads patterns from shared directory. */
    void LoadPatterns();

    /** Returns set of patterns used to guide playouts. */
    const HashedPatternSet& HashedPlayPatterns(HexColor color) const;

    /** Returns set of patterns used to guide playouts. */
    const PatternSet& PlayPatterns(HexColor color) const;

    /** Returns reference to configuration settings controlling all
        policies. */
    HexUctPolicyConfig& Config();

    /** Returns constant reference to configuration settings
        controlling all policies. */
    const HexUctPolicyConfig& Config() const;

private:
    HexUctPolicyConfig m_config;

    std::vector<Pattern> m_patterns[BLACK_AND_WHITE];

    HashedPatternSet m_hashPatterns[BLACK_AND_WHITE];

    void LoadPlayPatterns();
};

inline HexUctPolicyConfig& HexUctSharedPolicy::Config()
{
    return m_config;
}

inline const HexUctPolicyConfig& HexUctSharedPolicy::Config() const
{
    return m_config;
}

inline const PatternSet& HexUctSharedPolicy::PlayPatterns(HexColor color) const
{
    return m_patterns[color];
}

inline const HashedPatternSet& 
HexUctSharedPolicy::HashedPlayPatterns(HexColor color) const
{
    return m_hashPatterns[color];
}

//----------------------------------------------------------------------------

/** Generates moves during the random playout phase of UCT search.
    Uses local configuration and pattern data in HexUctSharedPolicy.
    Everything in this class must be thread-safe. */
class HexUctPolicy : public HexUctSearchPolicy
{
public:
    /** Creates a policy. */
    HexUctPolicy(const HexUctSharedPolicy* shared);

    ~HexUctPolicy();

    /** Generates a move. */
    HexPoint GenerateMove(PatternState& pastate, HexColor color, 
                          HexPoint lastMove);

    /** Initializes for fast playing of moves during playout.
        Must be called before any calls to GenerateMove(). */
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
