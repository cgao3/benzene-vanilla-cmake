//----------------------------------------------------------------------------
/** @file MoHexPlayoutPolicy.cpp */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "SgRandom.h"
#include "SgHash.h"
#include "MoHexPlayoutPolicy.hpp"
#include "MoHexPatterns.hpp"
#include "Misc.hpp"
#include "PatternState.hpp"
#include "BoardUtil.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

std::string MoHexPlayoutPolicyStatistics::ToString() const
{
    std::ostringstream os;
    os << "Playout Statistics:\n"
       << "Total               " << totalMoves << '\n'
       << "Pattern             " << patternMoves << " ("
       << std::setprecision(3) << double(patternMoves) * 100.0 
        / double(totalMoves) << "%)\n" 
       << "Random              " << randomMoves << " ("
       << std::setprecision(3) << double(randomMoves) * 100.0 
        / double(totalMoves) << "%)";
    return os.str();
}

//----------------------------------------------------------------------------

MoHexSharedPolicy::MoHexSharedPolicy()
    : m_config()
{
}

MoHexSharedPolicy::~MoHexSharedPolicy()
{
}

//----------------------------------------------------------------------------

MoHexPlayoutPolicy::MoHexPlayoutPolicy(MoHexSharedPolicy* shared,
                                       MoHexBoard& board, 
                                       const MoHexPatterns& globalPatterns,
                                       const MoHexPatterns& localPatterns)
    : m_shared(shared),
      m_board(board),
      m_globalPatterns(globalPatterns),
      m_localPatterns(localPatterns)
{
    m_weights = new WeightedRandom[2];
}

MoHexPlayoutPolicy::~MoHexPlayoutPolicy()
{
    delete [] m_weights;
}

//----------------------------------------------------------------------------

void MoHexPlayoutPolicy::InitializeForSearch()
{
}

float MoHexPlayoutPolicy::PlayoutGlobalGammaFunction(int type, float gamma)
{
    switch(type)
    {
    case 0: // normal
        return std::min(gamma, 10.0f);
    case 1: // opponent captured
    case 2: // vulnerable
        return 0.00001f;
    case 3: // dominated
        return 0.0001f;
    }
    return gamma;
}

float MoHexPlayoutPolicy::PlayoutLocalGammaFunction(int type, float gamma)
{
    switch(type)
    {
    case 0: // normal
        return gamma;
    case 1: // opponent captured
    case 2: // vulnerable
        return 0.00001f;
    case 3: // dominated
        return 0.0001f;
    }
    return gamma;
}

void MoHexPlayoutPolicy::InitializeForPlayout(const StoneBoard& brd)
{
    m_weights[0].Clear();
    m_weights[1].Clear();
    for (BitsetIterator it(brd.GetEmpty()); it; ++it)
    {
        m_weights[0][*it] = 
            m_globalPatterns.GammaFromKeysBoth(m_board.Keys(*it), BLACK);
        m_weights[1][*it] = 
            m_globalPatterns.GammaFromKeysBoth(m_board.Keys(*it), WHITE);
    }
    m_weights[0].Build();
    m_weights[1].Build();
}

HexPoint MoHexPlayoutPolicy::GenerateMove(const HexColor toPlay, 
                                          const HexPoint lastMove)
{
    HexPoint move = INVALID_POINT;
    const MoHexPlayoutPolicyConfig& config = m_shared->Config();
    MoHexPlayoutPolicyStatistics& stats = m_shared->Statistics();
    if (lastMove != INVALID_POINT && config.patternHeuristic)
    {
        move = GenerateLocalPatternMove(toPlay, lastMove);
        //move = GeneratePatternMove(toPlay, lastMove);
    }
    if (move == INVALID_POINT) 
    {
	stats.randomMoves++;
        move = GenerateRandomMove(toPlay);
    } 
    else 
        stats.patternMoves++;

    stats.totalMoves++;
    BenzeneAssert(m_board.GetColor(move) == EMPTY);
    return move;
}

void MoHexPlayoutPolicy::PlayMove(const HexPoint move, const HexColor toPlay)
{
    UNUSED(toPlay);
    m_weights[0].SetWeight(move, 0.0f);
    m_weights[1].SetWeight(move, 0.0f);
}

//--------------------------------------------------------------------------

/** Selects random move among the empty cells on the board. */
HexPoint MoHexPlayoutPolicy::GenerateRandomMove(const HexColor toPlay)
{
    // m_weights[toPlay].Build();
    // HexPoint ret = static_cast<HexPoint>(m_weights[toPlay].Choose(m_random));

    HexPoint ret = static_cast<HexPoint>(m_weights[toPlay]
                                         .ChooseLinear(m_random));
    if (m_board.GetColor(ret) != EMPTY)
        throw BenzeneException() << "Weighted move not empty!\n";
    return ret;
}

/** Checks the save-bridge pattern. */
HexPoint MoHexPlayoutPolicy::GeneratePatternMove(const HexColor toPlay,
                                                 const HexPoint lastMove)
{
    return m_board.SaveBridge(lastMove, toPlay, m_random);
}

void MoHexPlayoutPolicy::UpdateWeights(const HexPoint p, const HexColor toPlay)
{
    float gamma;
    gamma = m_globalPatterns.GammaFromKeysBoth(m_board.Keys(p), !toPlay);
    m_weights[!toPlay].SetWeight(p, gamma);

    const MoHexPatterns::Data* data;
    m_globalPatterns.MatchWithKeysBoth(m_board.Keys(p), toPlay, &data);
    if (data != NULL)
    {
        m_weights[toPlay].SetWeight(p, data->gamma);
        if (data->localGamma > 0.0f)
        {
            m_localMoves.gammaTotal += data->localGamma;
            m_localMoves.ptr.push_back(data);
            m_localMoves.move.push_back(p);
            m_localMoves.gamma.push_back(m_localMoves.gammaTotal);
        }
    } 
    else
        m_weights[toPlay].SetWeight(p, 1.0f);
}

HexPoint MoHexPlayoutPolicy::GenerateLocalPatternMove(const HexColor toPlay,
                                                      const HexPoint lastMove)
{
    m_localMoves.Clear();
    for (int i = 1; i <= 12; ++i)
    {
        const HexPoint n = m_board.Const().PatternPoint(lastMove, i);
        if (m_board.GetColor(n) == EMPTY)
            UpdateWeights(n, toPlay);
    }
    // LogInfo() << m_board.Write() << '\n'
    //           << "lastMove=" << lastMove
    //           << " global=" << m_weights[toPlay].Total()
    //           << " local=" << m_localMoves.gammaTotal 
    //           << " numLocal=" << m_localMoves.move.size() << '\n';
    HexPoint move = INVALID_POINT;
    if (!m_localMoves.move.empty())
    {
        float random = m_random.Float(m_weights[toPlay].Total() + 
                                      m_localMoves.gammaTotal);
        if (random < m_localMoves.gammaTotal)
        {
            m_localMoves.gamma.back() += 9999; // ensure it doesn't go past end
            int i = 0;
            while (m_localMoves.gamma[i] < random)
                ++i;
            move = m_localMoves.move[i];
        }
    }
    return move;
}

//----------------------------------------------------------------------------

void MoHexPlayoutPolicy::GetWeightsForLastMove
(std::vector<float>& weights, HexColor toPlay) const
{
    weights.resize(BITSETSIZE);
    for (int i = 0; i < BITSETSIZE; ++i)
        weights[i] = m_weights[toPlay][i];
    for (size_t i = 0; i < m_localMoves.move.size(); ++i)
        weights[ m_localMoves.move[i] ] += m_localMoves.ptr[i]->localGamma;
}

//----------------------------------------------------------------------------

