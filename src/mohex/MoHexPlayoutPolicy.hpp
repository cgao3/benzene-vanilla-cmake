//----------------------------------------------------------------------------
/** @file MoHexPlayoutPolicy.hpp */
//----------------------------------------------------------------------------

#ifndef MOHEXPLAYOUTPOLICY_HPP
#define MOHEXPLAYOUTPOLICY_HPP

#include "SgSystem.h"
#include "SgRandom.h"

#include "MoHexSearch.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Whether statistics on patterns should be collected or not. 
    Only use if debugging the policy as collecting the statistics greatly
    impacts performance. */
#define COLLECT_PATTERN_STATISTICS 0

//----------------------------------------------------------------------------

/** Configuration options for policies. */
struct MoHexPlayoutPolicyConfig
{
    /** Generate pattern moves. */
    bool patternHeuristic;

    /** Percent chance to check for pattern moves. */
    int patternCheckPercent;

    /** Play learned responses. */    
    bool responseHeuristic;

    /** Threshold at which the reponse heuristic is used. */
    std::size_t responseThreshold;

    MoHexPlayoutPolicyConfig();
};

inline MoHexPlayoutPolicyConfig::MoHexPlayoutPolicyConfig()
    : patternHeuristic(true),
      patternCheckPercent(100),
      responseHeuristic(false),
      responseThreshold(100)
{
}

//----------------------------------------------------------------------------

/** Statistics for a policy. */
struct MoHexPlayoutPolicyStatistics
{
    std::size_t totalMoves;

    std::size_t randomMoves;

    std::size_t patternMoves;

    std::map<const Pattern*, size_t> patternCounts[BLACK_AND_WHITE];

    std::map<const Pattern*, size_t> patternPicked[BLACK_AND_WHITE];

    MoHexPlayoutPolicyStatistics();
};

inline MoHexPlayoutPolicyStatistics::MoHexPlayoutPolicyStatistics()
    : totalMoves(0),
      randomMoves(0),
      patternMoves(0)
{
}

//----------------------------------------------------------------------------

/** Policy information shared among all threads. */
class MoHexSharedPolicy
{
public:
    MoHexSharedPolicy();

    ~MoHexSharedPolicy();

    /** Loads patterns from shared directory. */
    void LoadPatterns();

    /** Returns set of patterns used to guide playouts. */
    const HashedPatternSet& HashedPlayPatterns(HexColor color) const;

    /** Returns set of patterns used to guide playouts. */
    const PatternSet& PlayPatterns(HexColor color) const;

    /** Returns reference to configuration settings controlling all
        policies. */
    MoHexPlayoutPolicyConfig& Config();

    /** Returns constant reference to configuration settings
        controlling all policies. */
    const MoHexPlayoutPolicyConfig& Config() const;

private:
    MoHexPlayoutPolicyConfig m_config;

    std::vector<Pattern> m_patterns[BLACK_AND_WHITE];

    HashedPatternSet m_hashPatterns[BLACK_AND_WHITE];

    void LoadPlayPatterns();
};

inline MoHexPlayoutPolicyConfig& MoHexSharedPolicy::Config()
{
    return m_config;
}

inline const MoHexPlayoutPolicyConfig& MoHexSharedPolicy::Config() const
{
    return m_config;
}

inline const PatternSet& MoHexSharedPolicy::PlayPatterns(HexColor color) const
{
    return m_patterns[color];
}

inline const HashedPatternSet& 
MoHexSharedPolicy::HashedPlayPatterns(HexColor color) const
{
    return m_hashPatterns[color];
}

//----------------------------------------------------------------------------

/** Generates moves during the random playout phase of UCT search.
    Uses local configuration and pattern data in MoHexSharedPolicy.
    Everything in this class must be thread-safe. */
class MoHexPlayoutPolicy : public MoHexSearchPolicy
{
public:
    /** Creates a policy. */
    MoHexPlayoutPolicy(const MoHexSharedPolicy* shared);

    ~MoHexPlayoutPolicy();

    /** Generates a move. */
    HexPoint GenerateMove(PatternState& pastate, HexColor color, 
                          HexPoint lastMove);

    /** Initializes for fast playing of moves during playout.
        Must be called before any calls to GenerateMove(). */
    void InitializeForPlayout(const StoneBoard& brd);

    void InitializeForSearch();

    void AddResponse(HexColor toPlay, HexPoint lastMove, HexPoint response);

#if COLLECT_PATTERN_STATISTICS    
    /** Returns a string containing formatted statistics
        information. */
    std::string DumpStatistics();

    /** Returns the collected statistics. */
    const MoHexPlayoutPolicyStatistics& Statistics() const;
#endif

private:
    static const int MAX_VOTES = 1024;

    const MoHexSharedPolicy* m_shared;

    std::vector<HexPoint> m_moves;

    std::vector<HexPoint> m_response[BLACK_AND_WHITE][BITSETSIZE];

    /** Generator for this policy. */
    SgRandom m_random;

#if COLLECT_PATTERN_STATISTICS
    MoHexPlayoutPolicyStatistics m_statistics;
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

inline void MoHexPlayoutPolicy::AddResponse(HexColor toPlay, HexPoint lastMove,
                                      HexPoint response)
{
    if (m_shared->Config().responseHeuristic)
        m_response[toPlay][lastMove].push_back(response);
}

#if COLLECT_PATTERN_STATISTICS
inline const MoHexPlayoutPolicyStatistics& 
MoHexPlayoutPolicy::Statistics() const
{
    return m_statistics;
}
#endif

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // MOHEXPLAYOUTPOLICY_HPP
