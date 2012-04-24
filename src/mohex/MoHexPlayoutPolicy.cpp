//----------------------------------------------------------------------------
/** @file MoHexPlayoutPolicy.cpp */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "SgRandom.h"
#include "SgHash.h"
#include "MoHexPlayoutPolicy.hpp"
#include "Misc.hpp"
#include "PatternState.hpp"
#include "BoardUtil.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

namespace {

//----------------------------------------------------------------------------

/** Shuffle a vector with the given random number generator. 
    @todo Refactor this out somewhere. */
template<typename T>
void ShuffleVector(std::vector<T>& v, SgRandom& random)
{
    for (int i = static_cast<int>(v.size() - 1); i > 0; --i) 
    {
        int j = random.Int(i+1);
        std::swap(v[i], v[j]);
    }
}

/** Returns true 'percent' of the time. */
bool PercentChance(int percent, SgRandom& random)
{
    if (percent >= 100) 
        return true;
    unsigned int threshold = random.PercentageThreshold(percent);
    return random.RandomEvent(threshold);
}

//----------------------------------------------------------------------------

} // annonymous namespace

//----------------------------------------------------------------------------

std::string MoHexPlayoutPolicyStatistics::ToString() const
{
    std::ostringstream os;
    os << "Playout Statistics:\n"
       << "Total          " << totalMoves << '\n'
       << "Pattern        " << patternMoves << " ("
       << std::setprecision(3) << double(patternMoves) * 100.0 
        / double(totalMoves) << "%)\n" 
       << "Random         " << randomMoves << " ("
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

MoHexPlayoutPolicy::MoHexPlayoutPolicy(MoHexSharedPolicy* shared)
    : m_shared(shared)
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
    m_moves.clear();
    for (BitsetIterator it(brd.GetEmpty()); it; ++it)
    {
        m_moves.push_back(*it);
        m_color[*it] = EMPTY;
    }
    for (BitsetIterator it(brd.GetBlack()); it; ++it)
        m_color[*it] = BLACK;
    for (BitsetIterator it(brd.GetWhite()); it; ++it)
        m_color[*it] = WHITE;
    ShuffleVector(m_moves, m_random);
}

HexPoint MoHexPlayoutPolicy::GenerateMove(const HexState& state, 
                                          HexPoint lastMove)
{
    HexPoint move = INVALID_POINT;
    const MoHexPlayoutPolicyConfig& config = m_shared->Config();
    MoHexPlayoutPolicyStatistics& stats = m_shared->Statistics();
    // Patterns applied probabilistically (if heuristic is turned on)
    if (lastMove != INVALID_POINT 
        && config.patternHeuristic 
        && PercentChance(config.patternCheckPercent, m_random))
    {
        move = GeneratePatternMove(state, lastMove);
    }
    // Select random move if no move was selected by the heuristics
    if (move == INVALID_POINT) 
    {
	stats.randomMoves++;
        move = GenerateRandomMove(state.Position());
    } 
    else 
        stats.patternMoves++;
    BenzeneAssert(state.Position().IsEmpty(move));
    stats.totalMoves++;

    
    return move;
}

void MoHexPlayoutPolicy::PlayMove(HexPoint move, HexColor toPlay)
{
    m_color[move] = toPlay;
}

//--------------------------------------------------------------------------

/** Selects random move among the empty cells on the board. */
HexPoint MoHexPlayoutPolicy::GenerateRandomMove(const StoneBoard& brd)
{
    UNUSED(brd);
    HexPoint ret = INVALID_POINT;
    while (true) 
    {
	BenzeneAssert(!m_moves.empty());
        ret = m_moves.back();
        m_moves.pop_back();
        if (m_color[ret] == EMPTY)
            break;
        //if (brd.IsEmpty(ret))
        //  break;
    }
    return ret;
}

/** Checks the save-bridge pattern. */
HexPoint MoHexPlayoutPolicy::GeneratePatternMove(const HexState& state, 
                                                 HexPoint lastMove)
{
    const ConstBoard& brd = state.Position().Const();
    const HexColor toPlay = state.ToPlay();
    // State machine: s is number of cells matched.
    // In clockwise order, need to match CEC, where C is the color to
    // play and E is an empty cell. We start at a random direction and
    // stop at first match, which handles the case of multiple bridge
    // patterns occuring at once.
    // TODO: speed this up?
    int s = 0;
    const int k = m_random.Int(6);
    HexPoint ret = INVALID_POINT;
    for (int j = 0; j < 8; ++j)
    {
        const int i = (j + k) % 6;
        const HexPoint p = brd.PointInDir(lastMove, (HexDirection)i);
        const bool mine = m_color[p] == toPlay;
        if (s == 0)
        {
            if (mine) s = 1;
        }
        else if (s == 1)
        {
            if (mine) s = 1;
            else if (m_color[p] == !toPlay) s = 0;
            else
            {
                s = 2;
                ret = p;
            }
        }
        else if (s == 2)
        {
            if (mine) return ret; // matched!!
            else s = 0;
        }
    }
    return INVALID_POINT;
}

//----------------------------------------------------------------------------
