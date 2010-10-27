//----------------------------------------------------------------------------
/** @file HexUctSearch.cpp
 */
//----------------------------------------------------------------------------

#include "SgSystem.h"

#include "SgException.h"
#include "SgNode.h"
#include "SgMove.h"
#include "SgSList.h"

#include "BoardUtils.hpp"
#include "BitsetIterator.hpp"
#include "HexSgUtil.hpp"
#include "HexUctPolicy.hpp"
#include "HexUctSearch.hpp"
#include "HexUctState.hpp"
#include "HexUctUtil.hpp"
#include "PatternState.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

HexThreadStateFactory::HexThreadStateFactory(HexUctSharedPolicy* shared)
    : m_shared_policy(shared)
{
}

HexThreadStateFactory::~HexThreadStateFactory()
{
}

SgUctThreadState* 
HexThreadStateFactory::Create(std::size_t threadId, const SgUctSearch& search)
{
    SgUctSearch& srch = const_cast<SgUctSearch&>(search);
    HexUctSearch& hexSearch = dynamic_cast<HexUctSearch&>(srch);
    LogInfo() << "Creating thread " << threadId << '\n';
    HexUctState* state = new HexUctState(threadId, hexSearch,
                                         hexSearch.TreeUpdateRadius(),
                                         hexSearch.PlayoutUpdateRadius());
    state->SetPolicy(new HexUctPolicy(m_shared_policy));
    return state;
}

//----------------------------------------------------------------------------

HexUctSearch::HexUctSearch(SgUctThreadStateFactory* factory, int maxMoves)
    : SgUctSearch(factory, maxMoves),
      m_keepGames(false),
      m_liveGfx(false),
      m_liveGfxInterval(20000),
      m_treeUpdateRadius(2),
      m_playoutUpdateRadius(1),
      m_brd(0),
      m_shared_data(),
      m_root(0)
{
    SetBiasTermConstant(0.0);
    SetExpandThreshold(1);
    {
        std::vector<std::size_t> thresholds;
        thresholds.push_back(400);
        SetKnowledgeThreshold(thresholds);
    }
    SetLockFree(true);    
    SetMaxNodes(15000000);
    SetMoveSelect(SG_UCTMOVESELECT_COUNT);
    SetNumberThreads(1);    
    SetRave(true);
    SetRandomizeRaveFrequency(20);
    SetWeightRaveUpdates(false);
    SetRaveWeightInitial(1.0);
    SetRaveWeightFinal(20000.0);
}

HexUctSearch::~HexUctSearch()
{
    if (m_root != 0)
        m_root->DeleteTree();
    m_root = 0;
}

/** Merges last game into the tree of games. */
void HexUctSearch::AppendGame(const std::vector<SgMove>& sequence)
{
    HexAssert(m_root != 0);
    SgNode* node = m_root->RightMostSon();
    HexColor color = m_shared_data.root_to_play;
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

void HexUctSearch::OnStartSearch()
{
    HexAssert(m_brd);
    if (m_root != 0)
        m_root->DeleteTree();
    if (m_keepGames) 
    {
        m_root = new SgNode();
        SgNode* position = m_root->NewRightMostSon();
        HexSgUtil::SetPositionInNode(position, m_brd->GetPosition(),
                                     m_shared_data.root_to_play);
    }
    // Limit to avoid very long games (no need in Hex)
    int size = m_brd->Width() * m_brd->Height();
    int maxGameLength = size+10;
    SetMaxGameLength(maxGameLength);
    m_lastPositionSearched = m_brd->GetPosition();
}

void HexUctSearch::SaveGames(const std::string& filename) const
{
    if (m_root == 0)
        throw SgException("No games to save");
    HexSgUtil::WriteSgf(m_root, "MoHex", filename.c_str(), m_brd->Height()); 
}

void HexUctSearch::SaveTree(std::ostream& out, int maxDepth) const
{
    HexUctUtil::SaveTree(Tree(), m_lastPositionSearched, 
                         m_shared_data.root_to_play, out, maxDepth);
}

void HexUctSearch::OnSearchIteration(std::size_t gameNumber, int threadId,
                                     const SgUctGameInfo& info)
{
    UNUSED(threadId);
    UNUSED(info);
    if (m_liveGfx && gameNumber % m_liveGfxInterval == 0)
    {
        std::ostringstream os;
        os << "gogui-gfx:\n";
        os << "uct\n";
        HexColor initial_toPlay = m_shared_data.root_to_play;
        HexUctUtil::GoGuiGfx(*this, 
                             HexUctUtil::ToSgBlackWhite(initial_toPlay),
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

float HexUctSearch::UnknownEval() const
{
    // Note: 0.5 is not a possible value for a Bernoulli variable, better
    // use 0?
    return 0.5;
}

float HexUctSearch::InverseEval(float eval) const
{
    return (1 - eval);
}

std::string HexUctSearch::MoveString(SgMove move) const
{
    return HexPointUtil::ToString(static_cast<HexPoint>(move));
}

//----------------------------------------------------------------------------

