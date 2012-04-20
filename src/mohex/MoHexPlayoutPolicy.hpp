//----------------------------------------------------------------------------
/** @file MoHexPlayoutPolicy.hpp */
//----------------------------------------------------------------------------

#ifndef MOHEXPLAYOUTPOLICY_HPP
#define MOHEXPLAYOUTPOLICY_HPP

#include "SgSystem.h"
#include "SgRandom.h"

#include "HexState.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Configuration options for policies. */
struct MoHexPlayoutPolicyConfig
{
    /** Generate pattern moves. */
    bool patternHeuristic;

    /** Percent chance to check for pattern moves. */
    int patternCheckPercent;

    MoHexPlayoutPolicyConfig();
};

inline MoHexPlayoutPolicyConfig::MoHexPlayoutPolicyConfig()
    : patternHeuristic(true),
      patternCheckPercent(100)
{
}

//----------------------------------------------------------------------------

/** Statistics for a policy. */
struct MoHexPlayoutPolicyStatistics
{
    std::size_t totalMoves;

    std::size_t randomMoves;

    std::size_t patternMoves;

    MoHexPlayoutPolicyStatistics();

    std::string ToString() const;
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

    /** Returns reference to configuration settings controlling all
        policies. */
    MoHexPlayoutPolicyConfig& Config();

    /** Returns constant reference to configuration settings
        controlling all policies. */
    const MoHexPlayoutPolicyConfig& Config() const;

    /** Returns the collected statistics. */
    MoHexPlayoutPolicyStatistics& Statistics();

    const MoHexPlayoutPolicyStatistics& Statistics() const;

private:
    MoHexPlayoutPolicyConfig m_config;
    
    MoHexPlayoutPolicyStatistics m_statistics;
};

inline MoHexPlayoutPolicyConfig& MoHexSharedPolicy::Config()
{
    return m_config;
}

inline const MoHexPlayoutPolicyConfig& MoHexSharedPolicy::Config() const
{
    return m_config;
}

inline MoHexPlayoutPolicyStatistics& MoHexSharedPolicy::Statistics()
{
    return m_statistics;
}

inline const MoHexPlayoutPolicyStatistics& MoHexSharedPolicy::Statistics() const
{
    return m_statistics;
}

//----------------------------------------------------------------------------

/** Generates moves during the random playout phase of UCT search.
    Uses local configuration and pattern data in MoHexSharedPolicy.
    Everything in this class must be thread-safe. */
class MoHexPlayoutPolicy
{
public:
    /** Creates a policy. */
    MoHexPlayoutPolicy(MoHexSharedPolicy* shared, uint8_t* colorArray);

    ~MoHexPlayoutPolicy();

    /** Generates a move. */
    HexPoint GenerateMove(const HexState& state, HexPoint lastMove);

    /** Initializes for fast playing of moves during playout.
        Must be called before any calls to GenerateMove(). */
    void InitializeForPlayout(const StoneBoard& brd);

    void InitializeForSearch();

private:
    MoHexSharedPolicy* m_shared;

    uint8_t* m_color;

    std::vector<HexPoint> m_moves;

    /** Generator for this policy. */
    SgRandom m_random;

    HexPoint GeneratePatternMove(const HexState& state, HexPoint lastMove);

    HexPoint GenerateRandomMove(const StoneBoard& brd);
};

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // MOHEXPLAYOUTPOLICY_HPP
