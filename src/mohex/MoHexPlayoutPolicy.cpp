//----------------------------------------------------------------------------
/** @file MoHexPlayoutPolicy.cpp */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "MoHexPlayoutPolicy.hpp"
#include "Misc.hpp"
#include "PatternState.hpp"

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

MoHexSharedPolicy::MoHexSharedPolicy()
    : m_config()
{
    LoadPatterns();
}

MoHexSharedPolicy::~MoHexSharedPolicy()
{
}

void MoHexSharedPolicy::LoadPatterns()
{
    LoadPlayPatterns();
}

void MoHexSharedPolicy::LoadPlayPatterns()
{
    std::ifstream inFile;
    try {
        std::string file = MiscUtil::OpenFile("mohex-patterns.txt", inFile);
        LogConfig() << "MoHexSharedPolicy: reading patterns from '" 
                    << file << "'.\n";
    }
    catch (BenzeneException& e) {
        throw BenzeneException() << "MoHexSharedPolicy: " << e.what();
    }
    std::vector<Pattern> patterns;
    Pattern::LoadPatternsFromStream(inFile, patterns);
    LogConfig() << "MoHexSharedPolicy: parsed " << patterns.size() 
                << " patterns.\n";
    BenzeneAssert(m_patterns[BLACK].empty()); // Can only load patterns once!
    for (std::size_t i = 0; i < patterns.size(); ++i) 
    {
        Pattern p = patterns[i];
        switch(p.GetType()) 
        {
        case Pattern::MOHEX:
            m_patterns[BLACK].push_back(p);
            p.FlipColors();
            m_patterns[WHITE].push_back(p);
            break;
        default:
            throw BenzeneException() 
                  << "MoHexSharedPolicy: unknown pattern type '" 
                  << p.GetType() << "'\n";
        }
    }
    for (BWIterator color; color; ++color)
        m_hashPatterns[*color].Hash(m_patterns[*color]);
}

//----------------------------------------------------------------------------

MoHexPlayoutPolicy::MoHexPlayoutPolicy(const MoHexSharedPolicy* shared)
    : m_shared(shared)
#if COLLECT_PATTERN_STATISTICS
    , m_statistics()
#endif
{
}

MoHexPlayoutPolicy::~MoHexPlayoutPolicy()
{
}

//----------------------------------------------------------------------------

/** @todo Pass initial tree and initialize off of that? */
void MoHexPlayoutPolicy::InitializeForSearch()
{
    for (int i = 0; i < BITSETSIZE; ++i)
    {
        m_response[BLACK][i].clear();
        m_response[WHITE][i].clear();
    }
}

void MoHexPlayoutPolicy::InitializeForPlayout(const StoneBoard& brd)
{
    BitsetUtil::BitsetToVector(brd.GetEmpty(), m_moves);
    ShuffleVector(m_moves, m_random);
}

HexPoint MoHexPlayoutPolicy::GenerateMove(PatternState& pastate, 
                                          HexColor toPlay, HexPoint lastMove)
{
    HexPoint move = INVALID_POINT;
    bool pattern_move = false;
    const MoHexPlayoutPolicyConfig& config = m_shared->Config();
#if COLLECT_PATTERN_STATISTICS
    MoHexPlayoutPolicyStatistics& stats = m_statistics;
#endif
    // Patterns applied probabilistically (if heuristic is turned on)
    if (config.patternHeuristic 
        && PercentChance(config.patternCheckPercent, m_random))
    {
        move = GeneratePatternMove(pastate, toPlay, lastMove);
    }
    if (move == INVALID_POINT && config.responseHeuristic)
    {
        move = GenerateResponseMove(toPlay, lastMove, pastate.Board());
    }
    // Select random move if no move was selected by the heuristics
    if (move == INVALID_POINT) 
    {
#if COLLECT_PATTERN_STATISTICS
	stats.randomMoves++;
#endif
        move = GenerateRandomMove(pastate.Board());
    } 
    else 
    {
	pattern_move = true;
#if COLLECT_PATTERN_STATISTICS
        stats.patternMoves++;
#endif
    }
    
    BenzeneAssert(pastate.Board().IsEmpty(move));
#if COLLECT_PATTERN_STATISTICS
    stats.totalMoves++;
#endif
    return move;
}

#if COLLECT_PATTERN_STATISTICS
std::string MoHexPlayoutPolicy::DumpStatistics()
{
    std::ostringstream os;
    os << '\n'
       << "Pattern statistics:" << '\n'
       << std::setw(12) << "Name" << "  "
       << std::setw(10) << "Black" << ' '
       << std::setw(10) << "White" << ' '
       << std::setw(10) << "Black" << ' '
       << std::setw(10) << "White" << '\n'
       << "     ------------------------------------------------------\n";
    MoHexPlayoutPolicyStatistics stats = Statistics();
    const PatternSet& blackPat = m_shared->PlayPatterns(BLACK);
    const PatternSet& whitePat = m_shared->PlayPatterns(WHITE);
    for (std::size_t i = 0; i < blackPat.size(); ++i) 
        os << std::setw(12) << blackPat[i].GetName() << ": "
           << std::setw(10) << stats.patternCounts[BLACK][&blackPat[i]] << ' '
           << std::setw(10) << stats.patternCounts[WHITE][&whitePat[i]] << ' ' 
           << std::setw(10) << stats.patternPicked[BLACK][&blackPat[i]] << ' '
           << std::setw(10) << stats.patternPicked[WHITE][&whitePat[i]]<< '\n';
    os << "     ------------------------------------------------------\n"
       << '\n'
       << std::setw(12) << "Pattern" << ": " 
       << std::setw(10) << stats.patternMoves << ' '
       << std::setw(10) << std::setprecision(3) << 
        double(stats.patternMoves) * 100.0 / double(stats.totalMoves) << "%" 
       << '\n'
       << std::setw(12) << "Random" << ": " 
       << std::setw(10) << stats.randomMoves << ' '
       << std::setw(10) << std::setprecision(3) << 
        double(stats.randomMoves) * 100.0 / double(stats.totalMoves) << "%"  
       << '\n'
       << std::setw(12) << "Total" << ": " 
       << std::setw(10) << stats.totalMoves << '\n'
       << '\n';
    return os.str();
}
#endif

//--------------------------------------------------------------------------

HexPoint MoHexPlayoutPolicy::GenerateResponseMove(HexColor toPlay, 
                                                  HexPoint lastMove,
                                                  const StoneBoard& brd)
{
    std::size_t num = m_response[toPlay][lastMove].size();
    if (num > m_shared->Config().responseThreshold)
    {
        HexPoint move = m_response[toPlay][lastMove][m_random.Int(num)];
        if (brd.IsEmpty(move))
            return move;
    }
    return INVALID_POINT;
}

/** Selects random move among the empty cells on the board. */
HexPoint MoHexPlayoutPolicy::GenerateRandomMove(const StoneBoard& brd)
{
    HexPoint ret = INVALID_POINT;
    while (true) 
    {
	BenzeneAssert(!m_moves.empty());
        ret = m_moves.back();
        m_moves.pop_back();
        if (brd.IsEmpty(ret))
            break;
    }
    return ret;
}

/** Randomly picks a pattern move from the set of patterns that hit
    the last move, weighted by the pattern's weight. 
    If no pattern matches, returns INVALID_POINT. */
HexPoint MoHexPlayoutPolicy::PickRandomPatternMove(const PatternState& pastate, 
                                             const HashedPatternSet& patterns, 
                                             HexColor toPlay, HexPoint lastMove)
{
    UNUSED(toPlay);
    if (lastMove == INVALID_POINT)
	return INVALID_POINT;
    PatternHits hits;
    pastate.MatchOnCell(patterns, lastMove, PatternState::MATCH_ALL, hits);
    if (hits.empty())
        return INVALID_POINT;
    int num = 0;
    std::size_t patternIndex[MAX_VOTES];
    HexPoint patternMoves[MAX_VOTES];
    for (std::size_t i = 0; i < hits.size(); ++i) 
    {
#if COLLECT_PATTERN_STATISTICS
        m_statistics.patternCounts[toPlay][hits[i].GetPattern()]++;
#endif
        // Number of entries added to array is equal to the pattern's weight
        BenzeneAssert(num + hits[i].GetPattern()->GetWeight() < MAX_VOTES);
        for (int j = 0; j < hits[i].GetPattern()->GetWeight(); ++j) 
        {
            patternIndex[num] = i;
            patternMoves[num] = hits[i].Moves1()[0];
            num++;
        }
    }
    BenzeneAssert(num > 0);
    int i = m_random.Int(num);
#if COLLECT_PATTERN_STATISTICS
    m_statistics.patternPicked[toPlay][hits[patternIndex[i]].GetPattern()]++;
#endif
    return patternMoves[i];
}

/** Uses PickRandomPatternMove() with the shared PlayPatterns(). */
HexPoint MoHexPlayoutPolicy::GeneratePatternMove(const PatternState& pastate, 
                                           HexColor toPlay, HexPoint lastMove)
{
    return PickRandomPatternMove(pastate, m_shared->HashedPlayPatterns(toPlay),
                                 toPlay, lastMove);
}

//----------------------------------------------------------------------------
