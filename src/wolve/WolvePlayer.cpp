//----------------------------------------------------------------------------
/** @file WolvePlayer.cpp */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "SgSearchValue.h"

#include "BitsetIterator.hpp"
#include "VCSet.hpp"
#include "HexEval.hpp"
#include "Misc.hpp"
#include "EndgameUtils.hpp"
#include "SequenceHash.hpp"
#include "WolvePlayer.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

namespace {

//----------------------------------------------------------------------------

std::string PrintSgScore(int score)
{
    if (score >= +SgSearchValue::MIN_PROVEN_VALUE)
        return "win";
    if (score <= -SgSearchValue::MIN_PROVEN_VALUE)
        return "loss";
    std::ostringstream os;
    os << score;
    return os.str();
}

std::string PrintVector(const SgVector<SgMove>& vec)
{
    std::ostringstream os;
    for (int i = 0; i < vec.Length(); ++i)
        os << (i > 0 ? " " : "") << static_cast<HexPoint>(vec[i]);
    return os.str();
}

//----------------------------------------------------------------------------

}

//----------------------------------------------------------------------------

WolvePlayer::WolvePlayer()
    : BenzenePlayer(),
      m_hashTable(1 << 16),
      m_searchDepths()
{
    m_searchDepths.push_back(1);
    m_searchDepths.push_back(2);
    m_searchDepths.push_back(4);
}

WolvePlayer::~WolvePlayer()
{
}

//----------------------------------------------------------------------------

/** Generates a move using WolveSearch.    
    @todo Handle timelimit. 
    @todo Handle arbitrary search depths, ie, [1, 2, 4].
*/
HexPoint WolvePlayer::Search(const HexState& state, const Game& game,
                             HexBoard& brd, const bitset_t& consider,
                             double maxTime, double& outScore)
{
    UNUSED(game);
    UNUSED(maxTime);
    m_search.SetRootMovesToConsider(consider);
    m_search.SetWorkBoard(&brd);
    m_search.SetHashTable(&m_hashTable);
    m_search.SetToPlay(HexSgUtil::HexColorToSgColor(state.ToPlay()));
    // TODO: handle search depths properly
    const vector<std::size_t>& depths = m_searchDepths;
    std::size_t minDepth = *std::min_element(depths.begin(), depths.end());
    std::size_t maxDepth = *std::max_element(depths.begin(), depths.end());
    SgVector<SgMove> PV;
    int score = m_search.IteratedSearch(int(minDepth), int(maxDepth),
                                        -SgSearchValue::MIN_PROVEN_VALUE,
                                        +SgSearchValue::MIN_PROVEN_VALUE,
                                        &PV);
    BenzeneAssert(PV.Length() > 0);
    HexPoint bestMove = static_cast<HexPoint>(PV[0]);
    LogInfo() << PrintStatistics(score, PV) << '\n';
    outScore = score;
    return bestMove;
}

std::string WolvePlayer::PrintStatistics(int score, const SgVector<SgMove>& pv)
{
    SgSearchStatistics stats;
    m_search.GetStatistics(&stats);
    std::ostringstream os;
    os << '\n'
       << SgWriteLabel("NumNodes") << stats.NumNodes() << '\n'
       << SgWriteLabel("NumEvals") << stats.NumEvals() << '\n'
       << SgWriteLabel("DepthReached") << stats.DepthReached() << '\n'
       << SgWriteLabel("Elapsed") << stats.TimeUsed() << '\n'
       << SgWriteLabel("Nodes/s") << stats.NumNodesPerSecond() << '\n'
       << SgWriteLabel("Score") << PrintSgScore(score) << '\n'
       << SgWriteLabel("PV") << PrintVector(pv) << '\n'
       << '\n' 
       << m_hashTable << '\n';
    return os.str();
}

//----------------------------------------------------------------------------

