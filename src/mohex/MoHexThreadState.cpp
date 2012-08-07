//----------------------------------------------------------------------------
/** @file MoHexThreadState.cpp

    @note Use SG_ASSERT so that the assertion handler is used to dump
    the state of each thread when an assertion fails.

    @bug Running with assertions, and a non-zero knowledge threshold
    in lock-free mode will cause some assertions to fail: it is
    possible for threads to play into filled-in cells during the
    in-tree phase. */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "SgException.h"
#include "SgMove.h"

#include "BoardUtil.hpp"
#include "BitsetIterator.hpp"
#include "MoHexSearch.hpp"
#include "MoHexThreadState.hpp"
#include "MoHexUtil.hpp"
#include "EndgameUtil.hpp"
#include "VCUtil.hpp"

using namespace benzene;

/** Prints output during knowledge computation. */
static const bool DEBUG_KNOWLEDGE = false;

/** Prints hash sequence before computing knowledge. 
    Use to see what threads are doing knowledge computations. */
static const bool TRACK_KNOWLEDGE = false;

/** Check correctness of prior pruning.
    Builds VCs in position and compares results. */
#define DEBUG_PRIOR_PRUNING  0

//----------------------------------------------------------------------------

namespace
{

/** Returns true if game is over and sets provenType appropriately. */
bool IsProvenState(const MoHexBoard& board, HexColor toPlay, 
                   SgUctProvenType& provenType)
{
    HexColor winner = board.GetWinner();
    if (winner != EMPTY)
    {
        provenType = (winner == toPlay)
            ? SG_PROVEN_WIN : SG_PROVEN_LOSS;
        return true;
    }
    return false;
}

//----------------------------------------------------------------------------

} // namespace

//----------------------------------------------------------------------------

std::string MoHexSharedData::TreeStatistics::ToString() const
{
    std::ostringstream os;
    os << "Tree Statistics:\n"
       << "Prior Positions     " << priorPositions << '\n'
       << "Prior Proven        " << priorProven << '\n'                
       << "Prior Avg Moves     " << std::setprecision(3)
       << (double)priorMoves / (double)priorPositions << '\n'
       << "Prior Avg After     " << std::setprecision(3) 
       << (double)priorMovesAfter / (double)priorPositions << '\n'
        
       << "Know Positions      " << knowPositions << '\n'
       << "Know Proven         " << knowProven << '\n'
       << "Know Avg After      " << std::setprecision(3)
       << (knowPositions > 0 ? 
           (double)knowMovesAfter / (double)knowPositions : 0) << '\n'

       << "VCM Probes          " << vcmProbes << '\n'
       << "VCM Expanded        " << vcmExpanded << '\n'
       << "VCM Expanded Later  " << vcmExpandedLater << '\n'
       << "VCM Avg Responses   " << std::setprecision(3)
       << (vcmExpanded + vcmExpandedLater > 0 
           ? (double)vcmResponses / (double)(vcmExpanded + vcmExpandedLater)
           : 0);
       
    return os.str();
}

//----------------------------------------------------------------------------

MoHexThreadState::AssertionHandler::AssertionHandler
(const MoHexThreadState& state)
    : m_state(state)
{
}

void MoHexThreadState::AssertionHandler::Run()
{
    LogSevere() << m_state.Dump() << '\n';
}

//----------------------------------------------------------------------------

MoHexThreadState::MoHexThreadState(const unsigned int threadId,
                                   MoHexSearch& sch, 
                                   MoHexSharedPolicy* sharedPolicy)
    : SgUctThreadState(threadId, MoHexUtil::ComputeMaxNumMoves()),
      m_assertionHandler(*this),
      m_state(0),
      m_vcBrd(0),
      m_policy(sharedPolicy, m_board, 
               sch.PlayoutGlobalPatterns(), 
               sch.PlayoutLocalPatterns()),
      m_sharedData(0),
      m_priorKnowledge(*this),
      m_search(sch),
      m_isInPlayout(false)
{
}

MoHexThreadState::~MoHexThreadState()
{
}

std::string MoHexThreadState::Dump() const
{
    std::ostringstream os;
    os << "MoHexThreadState[" << m_threadId << "] ";
    if (m_isInPlayout) 
    {
        os << "[playout] ";
        os << "board: " << m_board.Write();
    } 
    else
        os << "board:" << m_state->Position().Write();
    return os.str();
}

/** CURRENTLY NOT USED */
SgBlackWhite MoHexThreadState::ToPlay() const
{
    return MoHexUtil::ToSgBlackWhite(ColorToPlay());
}

/** Called by LazyDelete() during tree phase. */
bool MoHexThreadState::IsValidMove(SgMove move)
{
    return m_state->Position().IsEmpty(static_cast<HexPoint>(move));
}

/** Evaluate state.
    Called during tree-phase (at terminal nodes) and at the end of
    each playout. */
SgUctValue MoHexThreadState::Evaluate()
{
    SG_ASSERT(m_board.GameOver());
    SgUctValue score = (m_board.GetWinner() == ColorToPlay()) ? 1.0 : 0.0;
    return score;
}

//----------------------------------------------------------------------------

/** @page mohextree MoHex Tree Phase
    
    Both m_board (a MoHexBoard) and m_state (a HexState) are played
    into during the in-tree phase. If a knowledge node is encountered
    m_board and m_state are overwritten with the data from the
    knowledge hashtable.

    m_state is used only to feed m_vcBrd->ComputeAll() (during a
    knowledge computation) and to initialize the playout policy at the
    start of a playout (it's easy to grab the empty cells from a
    StoneBoard). If MoHexBoard is given these capabilities, then
    m_state can be done away with entirely.
 */

/** Initialize for a new search. */
void MoHexThreadState::StartSearch()
{
    LogInfo() << "StartSearch()[" << m_threadId <<"]\n";
    m_usingKnowledge = !m_search.KnowledgeThreshold().empty();
    m_sharedData = &m_search.SharedData();
    // TODO: Fix the interface to HexBoard so this can be constant!
    // The problem is that VCBuilder (which is inside of HexBoard)
    // expects a non-const reference to a VCBuilderParam object.
    HexBoard& brd = const_cast<HexBoard&>(m_search.Board());
    if (!m_state.get() 
        || m_state->Position().Width() != brd.Width() 
        || m_state->Position().Height() != brd.Height())
    {
        m_state.reset(new HexState(brd.GetPosition(), BLACK));
        m_vcBrd.reset(new HexBoard(brd.Width(), brd.Height(), 
                                   brd.ICE(), brd.VCBuilderParameters()));
    }
    m_policy.InitializeForSearch();
}

void MoHexThreadState::GameStart()
{
    m_atRoot = true;
    m_isInPlayout = false;
    m_lastMovePlayed = MoveSequenceUtil::LastMoveFromHistory
        (m_sharedData->gameSequence);
    *m_state = m_sharedData->rootState;
    m_board = m_sharedData->rootBoard;
    m_toPlay = m_state->ToPlay();
}

/** Execute tree move. */
void MoHexThreadState::Execute(SgMove sgmove)
{
    HexPoint cell = static_cast<HexPoint>(sgmove);

    // Lock-free mode: It is possible we are playing into a filled-in
    // cell during the in-tree phase. This can occur if the thread
    // happens upon this state after fillin was published but before
    // the tree was pruned.
    //   If assertions are off, this results in a board possibly
    // containing cells of both colors and erroneous pattern state
    // info, resulting in an inaccurate playout value. In practice,
    // this does not seem to matter too much.
    //   If assertions are on, this will cause the search to abort
    // needlessly.
    // TODO: Handle case when assertions are on.
    SG_ASSERT(m_state->Position().IsEmpty(cell));
    m_hashForLastState = m_state->Hash();
    m_board.PlayMove(cell, ColorToPlay());
    m_state->PlayMove(cell);
    m_toPlay = m_state->ToPlay();
    m_lastMovePlayed = cell;
    m_atRoot = false;
    bool loadedState = false;

    if (m_usingKnowledge)
    {
        MoHexSharedData::StateData data;
        if(m_sharedData->stateData.Get(m_state->Hash(), data))
        {
            m_state->Position() = data.position;
            m_board = data.board;
            loadedState = true;
        }
    }

#if 0
    if (!loadedState)
    {
        AddTriangleFillin(cell, !ColorToPlay() /* note flipped color! */);
    }
#endif
}

bool MoHexThreadState::GenerateAllMoves(SgUctValue count, 
                                        std::vector<SgUctMoveInfo>& moves,
                                        SgUctProvenType& provenType)
{
    moves.clear();
    if (m_atRoot)
    {
        // Handle root node as a special case: using consider set
        // passed to us from MoHexPlayer.
        for (BitsetIterator it(m_sharedData->rootConsider); it; ++it)
            moves.push_back(SgUctMoveInfo(*it));
        if (count == 0)
        {
            m_sharedData->treeStatistics.priorPositions++;
            m_priorKnowledge.ProcessPosition(moves, m_lastMovePlayed, false);
        }
        return false;
    }
    else if (count <= 0)
    {
        // First time we have been to this node. If solid winning
        // chain exists then mark as proven and abort. Otherwise,
        // every empty cell is a potentially valid move.
        if (IsProvenState(m_board, ColorToPlay(), provenType))
            return false;
        for (BitsetIterator it(m_state->Position().GetEmpty()); it; ++it)
            moves.push_back(SgUctMoveInfo(*it));
        // If count is negative, then we are not actually expanding
        // this node, so do not compute prior knowledge.
        if (count == 0)
        {
            size_t oldSize = moves.size();
            m_sharedData->treeStatistics.priorPositions++;
            m_sharedData->treeStatistics.priorMoves += oldSize;
            m_priorKnowledge.ProcessPosition(moves, m_lastMovePlayed,
                                             m_search.PriorPruning());
            m_sharedData->treeStatistics.priorMovesAfter += moves.size();

#if 0
            // Debug: display moves after pruning
            if (!moves.empty() && moves.size() < oldSize)
            {
                 bitset_t bs;
                 for (size_t i = 0; i < moves.size(); ++i)
                     bs.set(moves[i].m_move);
                 LogInfo() << m_state->Position().Write(bs) << '\n';
            }
#endif
            // Mark state as loss if no moves remain
            if (moves.empty())
            {
                m_sharedData->treeStatistics.priorProven++;
                provenType = SG_PROVEN_LOSS;
                //LogInfo() << m_state->Position() << '\n'
                //          << "winner=" << !ColorToPlay() << '\n';
#if DEBUG_PRIOR_PRUNING
                m_vcBrd->GetPosition().SetPosition(m_state->Position());
                m_vcBrd->ComputeAll(ColorToPlay());
                if (EndgameUtil::IsDeterminedState(*m_vcBrd, ColorToPlay()))
                {
                    if (!EndgameUtil::IsLostGame(*m_vcBrd, ColorToPlay()))
                    {
                        LogSevere() << m_state->Position() << "toPlay=" 
                                    << ColorToPlay() << '\n';
                        throw BenzeneException("Not a proven loss!");
                    }
                }
                else
                {
                    LogSevere() << m_state->Position() << "toPlay=" 
                                << ColorToPlay() << '\n';
                    throw BenzeneException("Not actually a proven state!!");
                }
#endif
            }
            else if (m_usingKnowledge)
            {
                // Apply pre-computed vcm responses from parent's knowledge
                VCMFromParent(moves);
            }
        }
        return false;
    }
    else
    {
        // Re-visiting this state after a certain number of playouts.
        // If VC-win exists then mark as proven; otherwise, prune
        // moves outside of mustplay and store fillin. We must
        // truncate the child subtrees because of the fillin if lazy
        // delete is not on.
        BenzeneAssert(m_usingKnowledge);
        bitset_t moveset = m_state->Position().GetEmpty() 
            & ComputeKnowledge(provenType);
        for (BitsetIterator it(moveset); it; ++it)
            moves.push_back(SgUctMoveInfo(*it));
        // Truncate tree only if not using lazy delete
        return !m_search.LazyDelete();
    }
    BenzeneAssert(false);
    return false;
}

/** Computes moves to consider and stores fillin in the shared
    data. Sets provenType if state is determined by VCs. */
bitset_t MoHexThreadState::ComputeKnowledge(SgUctProvenType& provenType)
{
    provenType = SG_NOT_PROVEN;
    SgHashCode hash = m_state->Hash();
    MoHexSharedData::StateData data;
    if (m_sharedData->stateData.Get(hash, data))
    {
        if (TRACK_KNOWLEDGE)
            LogInfo() << "cached: " << hash << '\n';
        m_state->Position() = data.position;
        m_board = data.board;
        return data.consider;
    }
    if (TRACK_KNOWLEDGE)
        LogInfo() << "know: " << hash << '\n';
    m_sharedData->treeStatistics.knowPositions++;
    m_vcBrd->GetPosition().SetPosition(m_state->Position());
    m_vcBrd->ComputeAll(ColorToPlay());
    if (EndgameUtil::IsDeterminedState(*m_vcBrd, ColorToPlay()))
    {
        HexColor winner = ColorToPlay();
        provenType = SG_PROVEN_WIN;
        if (EndgameUtil::IsLostGame(*m_vcBrd, ColorToPlay()))
        {
            winner = !ColorToPlay();
            provenType = SG_PROVEN_LOSS;
        }
        m_sharedData->treeStatistics.knowProven++;
        if (DEBUG_KNOWLEDGE)
            LogInfo() << "Found win for " << winner << ":\n"
                      << *m_vcBrd << '\n';
        // Set the consider set to be all empty cells: doesn't really
        // matter since we are marking it as a proven node, so the
        // search will never decend past this node again.
        return m_state->Position().GetEmpty();
    }
    data.consider = EndgameUtil::MovesToConsider(*m_vcBrd, ColorToPlay());
    data.position = m_vcBrd->GetPosition();
    data.board.SetPosition(data.position);
    VCMInTree(*m_vcBrd, data.consider, ColorToPlay(), data.vcm);
    //VCMerge(*m_vcBrd, data.consider, ColorToPlay());
    //VCExtend(*m_vcBrd, data.consider, ColorToPlay());
    
    m_sharedData->stateData.Add(m_state->Hash(), data);
    m_sharedData->treeStatistics.knowMovesAfter += data.consider.count();

    m_state->Position() = data.position;
    m_board = data.board;

    if (DEBUG_KNOWLEDGE)
        LogInfo() << "===================================\n"
                  << "Recomputed state:" << '\n' << data.position << '\n'
                  << "Consider:" << data.position.Write(data.consider) << '\n';

    return data.consider;
}

void MoHexThreadState::TakeBackInTree(std::size_t nuMoves)
{
    SG_UNUSED(nuMoves);
}
    
//----------------------------------------------------------------------------

void MoHexThreadState::VCMInTree(const HexBoard& vcbrd, 
                                 const bitset_t consider,
                                 const HexColor toPlay,
                                 vector<MoHexSharedData::VCMPair >& vcm)
{
    const SgUctNode* node = m_gameInfo.m_nodes.back();
    for (SgUctChildIterator ip(m_search.Tree(), *node); ip; ++ip)
    {
        const SgUctNode& p = *ip;
        const HexPoint probe = (HexPoint)p.Move();
        if (!consider.test(probe))
            continue;
        bitset_t responses;
        VCUtil::RespondToProbe(vcbrd, toPlay, probe, responses);
        if (responses.none())
            continue;
        m_sharedData->treeStatistics.vcmProbes++;
        if (! p.HasChildren())
        {
            // Record responses for when this child is expanded
            vcm.push_back(MoHexSharedData::VCMPair((uint8_t)probe));
            for (BitsetIterator it(responses); it; ++it)
                vcm.back().responses.push_back(*it);
            continue;
        }
        SgUctValue totalGamma = 0.0f;
        m_sharedData->treeStatistics.vcmExpanded++;
        for (SgUctChildIterator ir(m_search.Tree(), p); ir; ++ir)
        {
            const SgUctNode& r = *ir;
            const SgUctValue gamma = r.Gamma();
            totalGamma += gamma;
            if (responses.test(r.Move()))
            {
                // LogInfo() << vcbrd.GetPosition().Write() << '\n' 
                //           << "probe=" << probe 
                //           << " respons=" << (HexPoint)r.Move() << '\n';
                const float bonusGamma = m_search.VCMGamma();
                const_cast<SgUctNode&>(r).SetGamma(gamma + bonusGamma);
                m_sharedData->treeStatistics.vcmResponses++;
                totalGamma += bonusGamma;
                //LogInfo() << "gamma=" << r.Gamma() << '\n';
            }
        }
        if (totalGamma > 0.0f)
        {
            for (SgUctChildIterator ir(m_search.Tree(), p); ir; ++ir)
            {
                const SgUctValue prior = (*ir).Gamma() / totalGamma;
                const_cast<SgUctNode&>(*ir).SetPrior(prior);
            }
        }
    }
}

void MoHexThreadState::VCMFromParent(std::vector<SgUctMoveInfo>& moves)
{
    MoHexSharedData::StateData data;
    if (!m_sharedData->stateData.Get(m_hashForLastState, data))
        return;
    // LogInfo() << "parent hash :" << m_hashForLastState << '\n';
    for (size_t i = 0; i < data.vcm.size(); ++i)
    {
        if (data.vcm[i].move != m_lastMovePlayed)
            continue;
        m_sharedData->treeStatistics.vcmExpandedLater++;
        SgUctValue totalGamma = 0.0f;
        const vector<uint8_t>& responses = data.vcm[i].responses;
        for (size_t j = 0; j < responses.size(); ++j)
        {
            for (size_t k = 0; k < moves.size(); ++k)
            {
                if (moves[k].m_move == responses[j])
                {
                    m_sharedData->treeStatistics.vcmResponses++;
                    moves[k].m_gamma += m_search.VCMGamma();
                    totalGamma += m_search.VCMGamma();
                }
            }
        }
        if (totalGamma > 0)
        {
            for (size_t k = 0; k < moves.size(); ++k)
                moves[k].m_prior = moves[k].m_gamma / totalGamma;
        }
        break;
    }
}

void MoHexThreadState::VCExtend(const HexBoard& vcbrd, const bitset_t consider,
                                const HexColor toPlay)
{
    UNUSED(consider);
    bitset_t extend;
    std::vector<SgUctValue> bonus(BITSETSIZE, 0.0f);
    static const SgUctValue SIZE_BONUS = 1.5f;
    const VCS& vcs = vcbrd.Cons(toPlay);
    const StoneBoard& brd = vcbrd.GetPosition();
    const Groups& groups = vcbrd.GetGroups();
    for (GroupIterator xg(groups, toPlay); xg; ++xg)
    {
        const HexPoint x = xg->Captain();
        for (BitsetIterator y(vcs.GetFullNbs(x)); y; ++y)
        {
            if (brd.GetColor(*y) != EMPTY)
                continue;
            extend.set(*y);
            size_t size = std::numeric_limits<size_t>::max();
            for (CarrierList::Iterator i(vcs.GetFullCarriers(x, *y)); i; ++i)
            {
                size_t count = i.Carrier().count();
                if (count < size)
                    size = count;
            }
            bonus[*y] += (float_t)(size * size) * SIZE_BONUS;
        }
    }
    if (extend.none())
        return;

#if 0
    LogInfo() << "extend:" << brd.Write(extend) << '\n';
    for (BitsetIterator i(extend); i; ++i)
        LogInfo() << '(' << *i << ' ' << bonus[*i] << ')';
    LogInfo() << '\n';
#endif

    // Update priors in the tree
    SgUctValue totalGamma = 0.0f;
    const SgUctNode* node = m_gameInfo.m_nodes.back();
    for (SgUctChildIterator i(m_search.Tree(), *node); i; ++i)
    {
        const HexPoint p = static_cast<HexPoint>((*i).Move());
        const SgUctValue gamma = (*i).Gamma();
        const_cast<SgUctNode&>(*i).SetGamma(gamma + bonus[p]);
        totalGamma += gamma + bonus[p];
    }
    if (totalGamma > 0.0f)
    {
        for (SgUctChildIterator i(m_search.Tree(), *node); i; ++i)
        {
            const SgUctValue prior = (*i).Gamma() / totalGamma;
            const_cast<SgUctNode&>(*i).SetPrior(prior);
        }
    }
}

void MoHexThreadState::VCMerge(const HexBoard& vcbrd, const bitset_t consider,
                               const HexColor toPlay)
{
    UNUSED(consider);
    bitset_t merge;
    const VCS& vcs = vcbrd.Cons(toPlay);
    const Groups& groups = vcbrd.GetGroups();
    for (GroupIterator xg(groups, toPlay); xg; ++xg)
    {
        const HexPoint x = xg->Captain();
        for (GroupIterator yg(groups, toPlay); 
             yg->Captain() != xg->Captain(); ++yg)
        {
            const HexPoint y = yg->Captain();
            if (vcs.FullExists(x, y)
                //|| !vcs.SemiExists(x, y)
                )
                continue;
            const bitset_t keys = vcs.GetFullNbs(x) & vcs.GetFullNbs(y);
            for (BitsetIterator z(keys); z; ++z)
            {
                for (CarrierList::Iterator s1(vcs.GetFullCarriers(x, *z)); 
                     s1; ++s1)
                {
                    const bitset_t xz = s1.Carrier();
                    if ((xz & vcs.FullIntersection(*z, y)).any())
                        continue;
                    for (CarrierList::Iterator s2(vcs.GetFullCarriers(*z, y));
                     s2; ++s2)
                    {
                        const bitset_t zy = s2.Carrier();
                        if ((xz & zy).none())
                            merge.set(*z);
                    }
                }
            }
#if 0
            for (CarrierList::Iterator s(vcs.GetSemiCarriers(x, y)); s; ++s)
            {
                const HexPoint key = vcs.SemiKey(s.Carrier(), x, y);
                //LogInfo() << "key=" << key
                //          << vcbrd.GetPosition().Write(s.Carrier()) << '\n';
                if (key != INVALID_POINT)
                    merge.set(key);
            }
#endif
        }
    }
    if (merge.count())
    {
        LogInfo() << "toPlay=" << toPlay << vcbrd.GetPosition().Write(merge)
                  << '\n';
    }
}

//----------------------------------------------------------------------------

/** @page mohexplayouts MoHex Playout Phase

    Playouts are initialized from m_state (for quick access to the set
    of empty cells), but played entirely on m_board. Hence m_state
    does not change during a playout. 
 */

/** Initialize for a set set of playouts. */
void MoHexThreadState::StartPlayouts()
{
    m_isInPlayout = true;
    if (m_search.NumberPlayouts() > 1)
    {
        // If doing more than one playout make a backup of this state
        m_playoutStartLastMove = m_lastMovePlayed;
        m_playoutStartBoard = m_board;
    }
}

void MoHexThreadState::StartPlayout()
{
    m_policy.InitializeForPlayout(m_state->Position());
}

/** Called by MoHexEngine.
    Not called by SgUctSearch; used by MoHexEngine to perform playouts
    directly for debugging, visualization, etc. */
void MoHexThreadState::StartPlayout(const HexState& state,
                                    HexPoint lastMovePlayed)
{
    const StoneBoard& brd = state.Position();
    if (!m_state.get() 
        || m_state->Position().Width() != brd.Width() 
        || m_state->Position().Height() != brd.Height())
    {
        m_state.reset(new HexState(state));
    }
    *m_state = state;
    m_board.SetPosition(m_state->Position());
    m_toPlay = m_state->ToPlay();
    m_lastMovePlayed = lastMovePlayed;
    StartPlayout();
}

SgMove MoHexThreadState::GeneratePlayoutMove(bool& skipRaveUpdate)
{
    skipRaveUpdate = false;
    const ConstBoard& cbrd = m_board.Const();

    // Uncomment line below to stop playout when win detected.
    // if (m_board.GameOver())
    //     return SG_NULLMOVE;

    // Stop when board is filled.
    if (m_board.NumMoves() == cbrd.Width() * cbrd.Height())
        return SG_NULLMOVE;

    SgPoint move = m_policy.GenerateMove(ColorToPlay(), m_lastMovePlayed);
    SG_ASSERT(move != SG_NULLMOVE);
    return move;
}

void MoHexThreadState::ExecutePlayout(SgMove sgmove)
{
    HexPoint cell = static_cast<HexPoint>(sgmove);
    SG_ASSERT(m_board.GetColor(cell) == EMPTY);
    m_policy.PlayMove(cell, ColorToPlay());
    m_board.PlayMove(cell, ColorToPlay());
    m_lastMovePlayed = cell;
    m_toPlay = !m_toPlay;
}

void MoHexThreadState::EndPlayout()
{
}

void MoHexThreadState::TakeBackPlayout(std::size_t nuMoves)
{
    SG_UNUSED(nuMoves);
    if (m_search.NumberPlayouts() > 1)
    {
        // If doing more than 1 playout, restore state at start of playout
        m_lastMovePlayed = m_playoutStartLastMove;
        m_board = m_playoutStartBoard;
        m_toPlay = m_state->ToPlay();
    }
}

//----------------------------------------------------------------------------

void MoHexThreadState::AddTriangleFill(const HexPoint cell,
                                       const HexColor color)
{
    // Check if move captures two cells
    const ConstBoard& cbrd = m_board.Const();
    for (int dd = 0; dd < 6; ++dd)
    {
        HexDirection d1 = (HexDirection)dd;
        HexDirection d2 = (HexDirection)((dd + 1) % 6);
        HexPoint e1 = cbrd.PointInDir(cell, d1);
        if (m_board.GetColor(e1) != EMPTY)
            continue;
        HexPoint e2 = cbrd.PointInDir(cell, d2);
        if (m_board.GetColor(e2) != EMPTY)
        {
            ++dd;  // skip next case where e2 is now e1
            continue;
        }
        if (m_board.GetColor(cbrd.PointInDir(e1, d1)) != color)
            continue;
        if (m_board.GetColor(cbrd.PointInDir(e1, d2)) != color)
            continue;
        if (m_board.GetColor(cbrd.PointInDir(e2, d2)) != color)
        {
            // skip next case: this will be in direction
            // -> d1 -> d1, and so must be color
            ++dd; 
            continue;
        }
        
        // Matched!!
        m_state->Position().SetColor(color, e1);
        m_state->Position().SetColor(color, e2);
        m_board.PlayMove(e1, color);
        m_board.PlayMove(e2, color);
        ++dd;  // skip next case where e2 is now e1

        // LogInfo() << "=============================="
        //           << m_state->Position().Write()
        //           << m_board.Write() << '\n';            
    }
}

//----------------------------------------------------------------------------
