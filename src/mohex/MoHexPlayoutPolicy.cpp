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
                                       const MoHexPatterns& localPatterns)
    : m_shared(shared),
      m_board(board),
      m_weights(BITSETSIZE),
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

void MoHexPlayoutPolicy::InitializeForPlayout(const StoneBoard& brd)
{
    m_weights.Clear();
    for (BitsetIterator it(brd.GetEmpty()); it; ++it)
        m_weights[*it] = 1.0f;
    m_weights.Build();
    //LogInfo() << brd.Write() << "\nTotal() " << m_weights.Total() << '\n';
}

HexPoint MoHexPlayoutPolicy::GenerateMove(const HexColor toPlay, 
                                          const HexPoint lastMove)
{
    HexPoint move = INVALID_POINT;
    const MoHexPlayoutPolicyConfig& config = m_shared->Config();
    MoHexPlayoutPolicyStatistics& stats = m_shared->Statistics();
    if (lastMove != INVALID_POINT && config.patternHeuristic)
    {
        //LogInfo() << brd.Write() << '\n' << "lastMove=" << lastMove << '\n';

        HexPoint localMove[8];
        float localGamma[8];
        float localTotal = 0.0f;
        MoHexPatterns::Data data;
        const ConstBoard& cbrd = m_board.Const();
        int num = 0;
        for (BoardIterator n(cbrd.Nbs(lastMove)); n; ++n)
        {
            if (m_board.GetColor(*n) == EMPTY)
            {
                data.type = 0;
                data.gamma = 1.0f;
                //m_localPatterns.Match(m_board, 6, *n, toPlay, &data);
                m_localPatterns.MatchWithKeys(m_board.Keys(*n), 6, toPlay, &data);
                // if (data.type)
                //     LogInfo() << "lastMove=" << lastMove 
                //               << " point=" << *n << " type=" << data.type
                //               << m_board.Write() << '\n';
                if (data.gamma != 1.0f) 
                {
                    localTotal += data.gamma;
                    localMove[num] = *n;
                    localGamma[num] = localTotal;
                    // LogInfo() << "i=" << num << " move=" << localMove[num]
                    //           << " gamma=" << gamma  
                    //           << " gamma[i]=" << localGamma[num] << '\n';
                    ++num;
                }
            }            
        }
        if (num > 0)
        {
            float random = m_random.Float(m_weights.Total() + localTotal);
            //LogInfo() << "random= " << random << '\n';
            if (random < localTotal)
            {
                localGamma[num - 1] += 9999; // ensure it doesn't go past end
                int i = 0;
                while (localGamma[i] < random)
                    ++i;
                move = localMove[i];
                //LogInfo() << "Local! move=" << move << '\n';
            }
        }
        //move = GeneratePatternMove(toPlay, lastMove);
    }
    if (move == INVALID_POINT) 
    {
	stats.randomMoves++;
        move = GenerateRandomMove();
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
    m_weights.SetWeightAndUpdate(move, 0.0f);
}

//--------------------------------------------------------------------------

/** Selects random move among the empty cells on the board. */
HexPoint MoHexPlayoutPolicy::GenerateRandomMove()
{
    BenzeneAssert(m_weights.Total() >= 0.99f);
    if (m_weights.Total() < 1)
        throw BenzeneException() << "Total() < 1!!\n";
    HexPoint ret = static_cast<HexPoint>(m_weights.Choose(m_random));
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

//----------------------------------------------------------------------------
