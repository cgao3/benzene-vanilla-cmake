//----------------------------------------------------------------------------
/** @file MoHexPlayoutPolicy.hpp */
//----------------------------------------------------------------------------

#ifndef MOHEXPLAYOUTPOLICY_HPP
#define MOHEXPLAYOUTPOLICY_HPP

#include "SgSystem.h"
#include "SgRandom.h"

#include "WeightedRandom.hpp"
#include "MoHexBoard.hpp"

_BEGIN_BENZENE_NAMESPACE_

class MoHexPatterns;

//----------------------------------------------------------------------------

/** Configuration options for policies. */
struct MoHexPlayoutPolicyConfig
{
    /** Generate pattern moves. */
    bool patternHeuristic;

    MoHexPlayoutPolicyConfig();
};

inline MoHexPlayoutPolicyConfig::MoHexPlayoutPolicyConfig()
    : patternHeuristic(true)
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
    MoHexPlayoutPolicy(MoHexSharedPolicy* shared, 
                       MoHexBoard& board,
                       const MoHexPatterns& localPatterns);

    ~MoHexPlayoutPolicy();

    /** Generates a move. */
    HexPoint GenerateMove(const HexColor toPlay, const HexPoint lastMove);

    /** Plays move move. */
    void PlayMove(const HexPoint move, const HexColor toPlay);

    /** Initializes for fast playing of moves during playout.
        Must be called before any calls to GenerateMove(). */
    void InitializeForPlayout(const StoneBoard& brd);

    void InitializeForSearch();

private:
    MoHexSharedPolicy* m_shared;

    MoHexBoard& m_board;

    /** Generator for this policy. */
    SgRandom m_random;

    WeightedRandom m_weights;

    const MoHexPatterns& m_localPatterns;

    HexPoint GeneratePatternMove(const HexColor toPlay, HexPoint lastMove);
    HexPoint GenerateLocalPatternMove(const HexColor toPlay, HexPoint lastMove);

    HexPoint GenerateRandomMove();
};

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // MOHEXPLAYOUTPOLICY_HPP
