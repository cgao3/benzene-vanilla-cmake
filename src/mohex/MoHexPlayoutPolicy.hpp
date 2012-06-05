//----------------------------------------------------------------------------
/** @file MoHexPlayoutPolicy.hpp */
//----------------------------------------------------------------------------

#ifndef MOHEXPLAYOUTPOLICY_HPP
#define MOHEXPLAYOUTPOLICY_HPP

#include "SgSystem.h"
#include "SgRandom.h"

#include "WeightedRandom.hpp"
#include "MoHexBoard.hpp"
#include "MoHexPatterns.hpp"

_BEGIN_BENZENE_NAMESPACE_

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
                       const MoHexPatterns& globalPatterns,
                       const MoHexPatterns& localPatterns);

    ~MoHexPlayoutPolicy();

    /** Updates weight of move. 
        Call this after updating the keys. */
    void UpdateWeights(const HexPoint p, const HexColor toPlay);
    
    /** Generates a move. */
    HexPoint GenerateMove(const HexColor toPlay, const HexPoint lastMove);

    /** Plays move. */
    void PlayMove(const HexPoint move, const HexColor toPlay);

    /** Initializes for fast playing of moves during playout.
        Must be called before any calls to GenerateMove(). */
    void InitializeForPlayout(const StoneBoard& brd);

    void InitializeForSearch();

    /** Fills weights with weight for each move.
        Call after GenerateMove(). */
    void GetWeightsForLastMove(std::vector<float>& weights, 
                               HexColor toPlay) const;

    /** Used during pattern initialization. */
    static float PlayoutGlobalGammaFunction(int type, float gamma);
    static float PlayoutLocalGammaFunction(int type, float gamma);

private:
    struct LocalMoves
    {
        std::vector<HexPoint> move;
        std::vector<float> gamma;
        std::vector<const MoHexPatterns::Data*> ptr;
        float gammaTotal;

        void Clear()
        {
            move.clear();
            gamma.clear();
            ptr.clear();
            gammaTotal = 0.0f;
        }
    };

    MoHexSharedPolicy* m_shared;

    MoHexBoard& m_board;

    /** Generator for this policy. */
    SgRandom m_random;

    WeightedRandom* m_weights;

    LocalMoves m_localMoves;

    const MoHexPatterns& m_globalPatterns;

    const MoHexPatterns& m_localPatterns;

    HexPoint GeneratePatternMove(const HexColor toPlay, HexPoint lastMove);
    HexPoint GenerateLocalPatternMove(const HexColor toPlay, HexPoint lastMove);

    HexPoint GenerateRandomMove(const HexColor toPlay);
};

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // MOHEXPLAYOUTPOLICY_HPP
