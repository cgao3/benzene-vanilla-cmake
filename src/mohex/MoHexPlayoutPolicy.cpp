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
      m_weights(2),
      m_globalPatterns(globalPatterns),
      m_localPatterns(localPatterns)
{
}

MoHexPlayoutPolicy::~MoHexPlayoutPolicy()
{
}

//----------------------------------------------------------------------------

void MoHexPlayoutPolicy::InitializeForSearch()
{
}

inline float UsePatternWeight(const MoHexPatterns::Data* data)
{
    if (data != NULL)
    {
        switch(data->type)
        {
        case 0: // normal
            return std::min(data->gamma, 10.0f);
        case 1: // opponent captured
        case 2: // vulnerable
            return 0.00001f;
        case 3: // dominated
            return 0.0001f;
        }
    }
    return 1.0f;
}

void MoHexPlayoutPolicy::InitializeForPlayout(const StoneBoard& brd)
{
    m_weights[0].Clear();
    m_weights[1].Clear();
    const MoHexPatterns::Data* data;
    for (BitsetIterator it(brd.GetEmpty()); it; ++it)
    {
        m_globalPatterns.MatchWithKeysBoth(m_board.Keys(*it), BLACK, &data);
        m_weights[0][*it] = UsePatternWeight(data);
        m_globalPatterns.MatchWithKeysBoth(m_board.Keys(*it), WHITE, &data);
        m_weights[1][*it] = UsePatternWeight(data);
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
    m_weights[0].SetWeightAndUpdate(move, 0.0f);
    m_weights[1].SetWeightAndUpdate(move, 0.0f);
}

//--------------------------------------------------------------------------

/** Selects random move among the empty cells on the board. */
HexPoint MoHexPlayoutPolicy::GenerateRandomMove(const HexColor toPlay)
{
    HexPoint ret = static_cast<HexPoint>(m_weights[toPlay].Choose(m_random));
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

HexPoint MoHexPlayoutPolicy::GenerateLocalPatternMove(const HexColor toPlay,
                                                      const HexPoint lastMove)
{
    HexPoint move = INVALID_POINT;
    HexPoint localMove[20];
    float localGamma[20];
    float localTotal = 0.0f;
    int num = 0;
    const ConstBoard& cbrd = m_board.Const();
    for (int i = 1; i <= 12; ++i)
    {
        const HexPoint n = cbrd.PatternPoint(lastMove, i);
        if (m_board.GetColor(n) == EMPTY)
        {
            const MoHexPatterns::Data* data;
            m_globalPatterns.MatchWithKeysBoth(m_board.Keys(n), BLACK, &data);
            m_weights[0].SetWeightAndUpdate(n, UsePatternWeight(data));
            m_globalPatterns.MatchWithKeysBoth(m_board.Keys(n), WHITE, &data);
            m_weights[1].SetWeightAndUpdate(n, UsePatternWeight(data));

            m_localPatterns.MatchWithKeysBoth(m_board.Keys(n), toPlay, &data);
            if (data != NULL && data->type == 0) 
            {
                localTotal += data->gamma;
                localMove[num] = n;
                localGamma[num] = localTotal;
                ++num;
            }
        }            
    }
    // LogInfo() << m_board.Write() << '\n'
    //           << "lastMove=" << lastMove
    //           << " global=" << m_weights[toPlay].Total()
    //           << " local=" << localTotal << '\n';
    if (num > 0)
    {
        float random = m_random.Float(m_weights[toPlay].Total() + localTotal);
        if (random < localTotal)
        {
            localGamma[num - 1] += 9999; // ensure it doesn't go past end
            int i = 0;
            while (localGamma[i] < random)
                ++i;
            move = localMove[i];
        }
    }
    return move;
}

//----------------------------------------------------------------------------
