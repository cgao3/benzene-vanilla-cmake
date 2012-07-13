//----------------------------------------------------------------------------
/** @file MoHexSearch.cpp */
//----------------------------------------------------------------------------

#include "SgSystem.h"

#include "SgException.h"
#include "SgNode.h"
#include "SgMove.h"
#include "SgPlatform.h"

#include "BoardUtil.hpp"
#include "BitsetIterator.hpp"
#include "HexSgUtil.hpp"
#include "MoHexPlayoutPolicy.hpp"
#include "MoHexSearch.hpp"
#include "MoHexThreadState.hpp"
#include "MoHexUtil.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

HexThreadStateFactory::HexThreadStateFactory(MoHexSharedPolicy* shared)
    : m_shared_policy(shared)
{
}

HexThreadStateFactory::~HexThreadStateFactory()
{
}

SgUctThreadState* 
HexThreadStateFactory::Create(unsigned int threadId, const SgUctSearch& search)
{
    SgUctSearch& srch = const_cast<SgUctSearch&>(search);
    MoHexSearch& hexSearch = dynamic_cast<MoHexSearch&>(srch);
    LogInfo() << "Creating thread " << threadId << '\n';
    MoHexThreadState* state = new MoHexThreadState(threadId, hexSearch,
                                                   m_shared_policy);
    return state;
}

//----------------------------------------------------------------------------

MoHexSearch::MoHexSearch(SgUctThreadStateFactory* factory, int maxMoves)
    : SgUctSearch(factory, maxMoves),
      m_liveGfx(false),
      m_brd(0),
      m_fillinMapBits(16),
      m_priorPruning(true),
      m_vcmGamma(1000.0f),
      m_sharedData(new MoHexSharedData(m_fillinMapBits)),
      m_globalPatterns(),
      m_localPatterns()
{
    SetBiasTermConstant(0.0);
    SetExpandThreshold(10);
    {
        std::vector<SgUctValue> thresholds;
        thresholds.push_back(256);
        SetKnowledgeThreshold(thresholds);
    }
    // Use 2 GB for search trees, but not more than half of the system memory
    // (note that SgUctSearch uses 2 trees)
    size_t systemMemory = SgPlatform::TotalMemory();
    if (systemMemory != 0)
    {
        size_t maxMemory = 2000000000;
        if (maxMemory > systemMemory / 2)
            maxMemory = systemMemory / 2;
        SetMaxNodes(maxMemory / sizeof(SgUctNode) / 2);
    }
    SetMoveSelect(SG_UCTMOVESELECT_COUNT);
    SetNumberThreads(1);    
    SetRave(true);
    SetFirstPlayUrgency(0.5f);
    SetRandomizeRaveFrequency(30);
    SetUctBiasConstant(0.22f);
    SetWeightRaveUpdates(false);
    SetRaveWeightInitial(2.12f);
    SetRaveWeightFinal(830.0f);
    SetProgressiveBiasConstant(2.47f);
    SetVCProgressiveBiasConstant(1.85f);
    SetLazyDelete(true);
    SetVirtualLoss(true);

    MoHexPatterns::InitializeZobrist();
    LoadPatterns();
}

MoHexSearch::~MoHexSearch()
{
}

//----------------------------------------------------------------------------

void MoHexSearch::LoadPatterns()
{
    LogInfo() << "Prior Patterns:\n";
    LogInfo() << "Global:\n";
    m_globalPatterns.ReadPatterns("mohex-global-pattern-gamma.txt", false);
    LogInfo() << "Local:\n";
    m_localPatterns.ReadPatterns("mohex-local-pattern-gamma.txt", false);

    LogInfo() << "Playout Patterns:\n";
    LogInfo() << "Global:\n";
    m_playoutGlobalPatterns
        .ReadPatterns("mohex-global-playout-pattern-gamma.txt",
                      true, MoHexPlayoutPolicy::PlayoutGlobalGammaFunction);
    LogInfo() << "Local:\n";
    m_playoutLocalPatterns
        .ReadPatterns("mohex-local-playout-pattern-gamma.txt", false,
                      MoHexPlayoutPolicy::PlayoutLocalGammaFunction);
    // Optimize for speed: store local gamma in global table for fast lookup
    MoHexPatterns::AddLocalToGlobal(m_playoutGlobalPatterns,
                                    m_playoutLocalPatterns);
}

void MoHexSearch::OnStartSearch()
{
    BenzeneAssert(m_brd);
    // Limit to avoid very long games (no need in Hex)
    int size = m_brd->Width() * m_brd->Height();
    int maxGameLength = size+10;
    SetMaxGameLength(maxGameLength);
    m_lastPositionSearched = m_brd->GetPosition();
    m_nextLiveGfx = 1000;
}

void MoHexSearch::SaveTree(std::ostream& out, int maxDepth) const
{
    MoHexUtil::SaveTree(Tree(), m_lastPositionSearched, 
                        m_sharedData->rootState.ToPlay(), out, maxDepth);
}

void MoHexSearch::OnSearchIteration(SgUctValue gameNumber, 
                                    const unsigned int threadId,
                                    const SgUctGameInfo& info)
{
    SgUctSearch::OnSearchIteration(gameNumber, threadId, info);
    if (m_liveGfx && threadId == 0 && gameNumber > m_nextLiveGfx)
    {
        m_nextLiveGfx = gameNumber + Statistics().m_gamesPerSecond;
        std::ostringstream os;
        os << "gogui-gfx:\n";
        os << "uct\n";
        HexColor toPlay = m_sharedData->rootState.ToPlay();
        MoHexUtil::GoGuiGfx(*this, MoHexUtil::ToSgBlackWhite(toPlay), os);
        os << '\n';
        std::cout << os.str();
        std::cout.flush();
    }
}

SgUctValue MoHexSearch::UnknownEval() const
{
    // Note: 0.5 is not a possible value for a Bernoulli variable, better
    // use 0?
    return 0.5;
}

SgUctValue MoHexSearch::InverseEval(SgUctValue eval) const
{
    return (1 - eval);
}

std::string MoHexSearch::MoveString(SgMove move) const
{
    return HexPointUtil::ToString(static_cast<HexPoint>(move));
}

//----------------------------------------------------------------------------

