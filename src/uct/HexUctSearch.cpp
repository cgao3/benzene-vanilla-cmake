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
#include "PatternBoard.hpp"

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
    // Need to do this since Fuego expects it be constant
    SgUctSearch& srch = const_cast<SgUctSearch&>(search);
    HexUctSearch& hexSearch 
        = dynamic_cast<HexUctSearch&>(srch);

    LogInfo() << "Creating thread " << threadId << '\n';

    HexUctState* state = new HexUctState(threadId, hexSearch,
                                         hexSearch.TreeUpdateRadius(),
                                         hexSearch.PlayoutUpdateRadius());

    state->SetPolicy(new HexUctPolicy(m_shared_policy));
    return state;
}

//----------------------------------------------------------------------------

HexUctSearch::HexUctSearch(SgUctThreadStateFactory* factory,
			   int maxMoves)
    : SgUctSearch(factory, maxMoves),
      m_keepGames(false),
      m_liveGfx(false),
      m_liveGfxInterval(5000),
      m_treeUpdateRadius(2),
      m_playoutUpdateRadius(1),

      m_brd(0),
      m_shared_data(),
      m_root(0)
{
    SetBiasTermConstant(0.0);
    SetExpandThreshold(1);
    SetKnowledgeThreshold(400);
    SetLockFree(true);    
    SetMaxNodes(15000000);
    SetMoveSelect(SG_UCTMOVESELECT_COUNT);
    SetNumberThreads(1);    
    SetRave(true);
    SetRaveWeightInitial(1.0);
    SetRaveWeightFinal(20000.0);
}

HexUctSearch::~HexUctSearch()
{
    if (m_root != 0)
        m_root->DeleteTree();
    m_root = 0;
}

#if 0
void HexUctSearch::AppendGame(const std::vector<SgMove>& sequence)
{
    HexAssert(m_root != 0);
    SgNode* node = m_root;
    HexColor color = m_initial_toPlay;

    std::vector<SgPoint>::const_iterator it = sequence.begin();

    // merge this game into the tree of games;
    // so find first move that starts a new variation.
    if (true) {
        for (; it != sequence.end(); ++it) {
            
            if (!node->HasSon())
                break;

            bool found = false;
            for (SgNode* child = node->LeftMostSon(); ;
                 child = child->RightBrother()) 
            {
                HexPoint move = HexSgUtil::SgPointToHexPoint(child->NodeMove(),
                                                             m_brd->height());

                // found it! recurse down this branch.
                if (move == *it) {
                    node = child;
                    found = true;
                    break;
                }

                if (!child->HasRightBrother()) 
                    break;
            }            

            // abort if we need to start a new variation
            if (!found) break;

            color = !color;
        }
    }

    // add the remainder of the sequence to this node
    for (; it != sequence.end(); ++it) {
        SgNode* child = node->NewRightMostSon();
        HexSgUtil::AddMoveToNode(child, color, *it, m_brd->height());
        color = !color;
        node = child;
    }
}
#endif 

void HexUctSearch::OnStartSearch()
{
    HexAssert(m_brd);

    if (m_root != 0)
        m_root->DeleteTree();

    if (m_keepGames) {
        LogSevere() << "uct-save-games disabled!" << '\n';
        HexAssert(false);
#if 0        
        m_root = new SgNode();
        HexSgUtil::SetPositionInNode(m_root, *m_brd, m_initial_toPlay);
#endif
    }

    // Limit to avoid very long games (no need in Hex)
    int size = m_brd->width() * m_brd->height();
    int maxGameLength = size+10;
    SetMaxGameLength(maxGameLength);

    m_lastPositionSearched = *m_brd;
}

void HexUctSearch::SaveGames(const std::string& filename) const
{
    if (m_root == 0)
        throw SgException("No games to save");
    HexSgUtil::WriteSgf(m_root, "MoHex", filename.c_str(), m_brd->height()); 
}

void HexUctSearch::SaveTree(std::ostream& out) const
{
    UNUSED(out);

    LogWarning() << "SaveTree() not implemented yet!" << '\n';

#if 0
    HexUctUtil::SaveTree(Tree(), m_brd->Size(), m_stones, m_initial_toPlay, 
                         out, UseMoveCounts());
#endif
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
    
#if 0
    if (m_root != 0)
        AppendGame(LastGameInfo().m_sequence);
#endif
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
    return HexPointUtil::toString(static_cast<HexPoint>(move));
}

//----------------------------------------------------------------------------

