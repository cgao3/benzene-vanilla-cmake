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
#include "SequenceHash.hpp"

using namespace benzene;

/** Prints output during knowledge computation. */
static const bool DEBUG_KNOWLEDGE = false;

/** Prints hash sequence before computing knowledge. 
    Use to see what threads are doing knowledge computations. */
static const bool TRACK_KNOWLEDGE = false;

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

/** Returns INVALID_POINT if history is empty, otherwise last move
    played to the board, ie, skips swap move. */
HexPoint LastMoveFromHistory(const MoveSequence& history)
{
    HexPoint lastMove = INVALID_POINT;
    if (!history.empty()) 
    {
	lastMove = history.back().Point();
	if (lastMove == SWAP_PIECES) 
        {
            BenzeneAssert(history.size() == 2);
            lastMove = history.front().Point();
	}
    }
    return lastMove;
}

//----------------------------------------------------------------------------

} // namespace

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
      m_policy(sharedPolicy, m_board),
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
    m_lastMovePlayed = LastMoveFromHistory(m_sharedData->gameSequence);
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
    m_board.PlayMove(cell, ColorToPlay());
    m_state->PlayMove(cell);
    m_toPlay = m_state->ToPlay();
    m_lastMovePlayed = cell;
    m_atRoot = false;

    if (m_usingKnowledge)
    {
        MoHexSharedData::StateData data;
        if(m_sharedData->stateData.Get(m_state->Hash(), data))
        {
            m_state->Position() = data.position;
            m_board = data.board;
        }
    }
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
            m_priorKnowledge.ProcessPosition(moves, false);
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
            m_priorKnowledge.ProcessPosition(moves, true);
            if (moves.empty())
            {
                m_sharedData->treeStatistics.priorProven++;
                provenType = SG_PROVEN_LOSS;
                //LogInfo() << m_state->Position() << '\n'
                //          << "winner=" << !ColorToPlay() << '\n';
            }
            m_sharedData->treeStatistics.priorMovesAfter += moves.size();

            //if (moves.size() < oldSize)
            // {
            //     bitset_t bs;
            //     for (size_t i = 0; i < moves.size(); ++i)
            //         bs.set(moves[i].m_move);
            //     LogInfo() << m_state->Position().Write(bs) << '\n';
            // }
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
