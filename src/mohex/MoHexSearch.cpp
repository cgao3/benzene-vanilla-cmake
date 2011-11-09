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
#include "PatternState.hpp"

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
                                         hexSearch.TreeUpdateRadius(),
                                         hexSearch.PlayoutUpdateRadius());
    state->SetPolicy(new MoHexPlayoutPolicy(m_shared_policy));
    return state;
}

//----------------------------------------------------------------------------

MoHexSearch::MoHexSearch(SgUctThreadStateFactory* factory, int maxMoves)
    : SgUctSearch(factory, maxMoves),
      m_keepGames(false),
      m_liveGfx(false),
      m_treeUpdateRadius(2),
      m_playoutUpdateRadius(1),
      m_brd(0),
      m_fillinMapBits(16),
      m_sharedData(new MoHexSharedData(m_fillinMapBits)),
      m_root(0)
{
    SetBiasTermConstant(0.0);
    SetExpandThreshold(1);
    {
        std::vector<SgUctValue> thresholds;
        thresholds.push_back(400);
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
    SetRandomizeRaveFrequency(20);
    SetWeightRaveUpdates(false);
    SetRaveWeightInitial(1.0);
    SetRaveWeightFinal(20000.0);
}

MoHexSearch::~MoHexSearch()
{
    if (m_root != 0)
        m_root->DeleteTree();
    m_root = 0;
}

/** Merges last game into the tree of games. */
void MoHexSearch::AppendGame(const std::vector<SgMove>& sequence)
{
    BenzeneAssert(m_root != 0);
    SgNode* node = m_root->RightMostSon();
    HexColor color = m_sharedData->rootState.ToPlay();
    std::vector<SgPoint>::const_iterator it = sequence.begin();
    // Find first move that starts a new variation
    for (; it != sequence.end(); ++it) 
    {
        if (!node->HasSon())
            break;
        bool found = false;
        for (SgNode* child = node->LeftMostSon(); ; 
             child = child->RightBrother()) 
        {
            HexPoint move = HexSgUtil::SgPointToHexPoint(child->NodeMove(),
                                                         m_brd->Height());
            // Found it! Recurse down this branch
            if (move == *it) 
            {
                node = child;
                found = true;
                break;
            }
            if (!child->HasRightBrother()) 
                break;
        }            
        // Abort if we need to start a new variation
        if (!found) 
            break;
        color = !color;
    }
    // Add the remainder of the sequence to this node
    for (; it != sequence.end(); ++it) 
    {
        SgNode* child = node->NewRightMostSon();
        HexSgUtil::AddMoveToNode(child, color, static_cast<HexPoint>(*it), 
                                 m_brd->Height());
        color = !color;
        node = child;
    }
}

void MoHexSearch::OnStartSearch()
{
    BenzeneAssert(m_brd);
    if (m_root != 0)
        m_root->DeleteTree();
    if (m_keepGames) 
    {
        m_root = new SgNode();
        SgNode* position = m_root->NewRightMostSon();
        HexSgUtil::SetPositionInNode(position, 
                                     m_sharedData->rootState.Position(),
                                     m_sharedData->rootState.ToPlay());
    }
    // Limit to avoid very long games (no need in Hex)
    int size = m_brd->Width() * m_brd->Height();
    int maxGameLength = size+10;
    SetMaxGameLength(maxGameLength);
    m_lastPositionSearched = m_brd->GetPosition();
    m_nextLiveGfx = 1000;
}

void MoHexSearch::SaveGames(const std::string& filename) const
{
    if (m_root == 0)
        throw SgException("No games to save");
    HexSgUtil::WriteSgf(m_root, filename.c_str(), m_brd->Height()); 
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
        HexColor initial_toPlay = m_sharedData->rootState.ToPlay();
        MoHexUtil::GoGuiGfx(*this, 
                            MoHexUtil::ToSgBlackWhite(initial_toPlay),
                            os);
        os << "\n";
        std::cout << os.str();
        std::cout.flush();
        LogFine() << os.str() << '\n';
    }
    if (m_root != 0)
    {
        for (std::size_t i = 0; i < LastGameInfo().m_sequence.size(); ++i)
            AppendGame(LastGameInfo().m_sequence[i]);
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

