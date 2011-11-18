//----------------------------------------------------------------------------
/** @file WolvePlayer.cpp */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "SgSearchControl.h"
#include "SgSearchValue.h"

#include "BitsetIterator.hpp"
#include "BoardUtil.hpp"
#include "VCSet.hpp"
#include "HexEval.hpp"
#include "Misc.hpp"
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

//----------------------------------------------------------------------------

}

//----------------------------------------------------------------------------

WolvePlayer::WolvePlayer()
    : BenzenePlayer(),
      m_hashTable(new SgSearchHashTable(1 << 20)),
      m_maxTime(10),
      m_minDepth(1),
      m_maxDepth(99),
      m_useTimeManagement(false),
      m_useEarlyAbort(false),
      m_ponder(false)
{
}

WolvePlayer::~WolvePlayer()
{
}

//----------------------------------------------------------------------------

/** Generates a move using WolveSearch. */
HexPoint WolvePlayer::Search(const HexState& state, const Game& game,
                             HexBoard& brd, const bitset_t& consider,
                             double maxTime, double& outScore)
{
    UNUSED(game);
    m_search.SetRootMovesToConsider(consider);
    m_search.SetWorkBoard(&brd);
    m_search.SetHashTable(m_hashTable.get());
    m_search.SetToPlay(HexSgUtil::HexColorToSgColor(state.ToPlay()));
    SgVector<SgMove> PV;
    WolveSearchControl timeControl(maxTime, m_useEarlyAbort, PV);
    m_search.SetSearchControl(&timeControl);
    std::size_t minDepth = MinDepth();
    std::size_t maxDepth = MaxDepth();
    if (!m_search.SpecificPlyWidths().empty())
    {
        LogInfo() << "Using specific plywidths!!\n";
        if (maxDepth > m_search.SpecificPlyWidths().size())
        {
            maxDepth = m_search.SpecificPlyWidths().size();
            LogWarning() << "Max depth exceeds depth specified in ply_width!\n"
                         << "Capping maxDepth to be safe.\n";
        }
    }
    LogInfo() << "minDepth=" << minDepth << ' ' 
              << "maxDepth=" << maxDepth << '\n';
    int score = m_search.IteratedSearch(int(minDepth), int(maxDepth),
                                        -SgSearchValue::MIN_PROVEN_VALUE,
                                        +SgSearchValue::MIN_PROVEN_VALUE, &PV,
                                        false);
    if (m_search.GuiFx())
        WolveSearchUtil::DumpGuiFx(state, *m_hashTable);
    LogInfo() << PrintStatistics(score, PV);
    outScore = score;
    if (PV.Length() > 0)
        return static_cast<HexPoint>(PV[0]);
    LogWarning() << "**** WolveSearch returned empty sequence!\n"
		 << "**** Returning random move!\n";
    return BoardUtil::RandomEmptyCell(state.Position());
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
       << SgWriteLabel("PV") << WolveSearch::PrintPV(pv) << '\n'
       << '\n'; 
    if (m_hashTable.get()) 
        os << *m_hashTable << '\n';
    return os.str();
}

//----------------------------------------------------------------------------

