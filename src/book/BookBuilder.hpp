//----------------------------------------------------------------------------
/** @file BookBuilder.hpp
 */
//----------------------------------------------------------------------------

#ifndef BOOKBUILDER_HPP
#define BOOKBUILDER_HPP

#include <cmath>
#include "BenzenePlayer.hpp"
#include "BitsetIterator.hpp"
#include "HashDB.hpp"
#include "Book.hpp"
#include "EndgameUtils.hpp"
#include "StateDB.hpp"
#include "Resistance.hpp"
#include "ThreadedWorker.hpp"
#include "SgBookBuilder.h"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** @page bookrefresh Book Refresh
    @ingroup openingbook

    Due to transpositions, it is possible that a node's value changes,
    but because the node has not been revisited yet the information is
    not passed to its parent. Refreshing the book forces these
    propagations.

    BookBuilder::Refresh() computes the correct propagation value for
    all internal nodes given the current set of leaf nodes. A node in
    which BookNode::IsLeaf() is true is treated as a leaf even
    if it has children in the book (ie, children from transpositions)
*/

//----------------------------------------------------------------------------

/** Expands a Book using the given player to evaluate
    game positions positions. 

    Supports multithreaded evaluation of states.

    @ingroup openingbook
*/
template<class PLAYER>
class BookBuilder : public SgBookBuilder
{
public:

    /** Constructor. Takes a reference to the player that will
        evaluate states, which must be a reference to a
        BenzenePlayer. */
    BookBuilder(PLAYER& player);
    
    /** Destructor. */
    ~BookBuilder();

    //---------------------------------------------------------------------

    /** Sets the state to start work from. */
    void SetState(const HexState& state);

    /** Sets the board the book builder can use to perform work. */
    void SetWorkBoard(HexBoard& brd);

    //---------------------------------------------------------------------    

    /** Whether to prune out inferior cells from the book or not. */
    bool UseICE() const;

    /** See UseICE() */
    void SetUseICE(bool flag);

    /** Number of players to use during leaf expansion. Each player
        may use a multi-threaded search. Should speed up the expansion
        of leaf states by a factor of (very close to) NumThreads(). */
    std::size_t NumThreads() const;

    /** See NumThreads() */
    void SetNumThreads(std::size_t num);

protected:

    float InverseEval(float eval) const;

    bool IsLoss(float eval) const;

    void PlayMove(SgMove move);

    void UndoMove(SgMove move);

    bool GetNode(BookNode& node) const;

    float Value(const BookNode& node) const;

    void WriteNode(const BookNode& node);

    void EnsureRootExists();

    bool GenerateMoves(std::vector<SgMove>& moves, HexEval& value);

    void GetAllLegalMoves(std::vector<SgMove>& moves);

    void EvaluateChildren(const std::vector<SgMove>& childrenToDo,
                          std::vector<std::pair<SgMove, HexEval> >& scores);
    void Init();

    void BeforeEvaluateChildren();

    void AfterEvaluateChildren();

    void Fini();
        
private:

    /** Copyable worker. */
    class Worker
    {
    public:
        Worker(std::size_t id, BenzenePlayer& player, HexBoard& brd);

        void SetState(const HexState& state);

        HexEval operator()(const SgMove& move);

    private:

        std::size_t m_id;
        
        HexBoard* m_brd;
        
        BenzenePlayer* m_player;

        HexState m_state;
    };

    /** Player passed to constructor. */
    PLAYER& m_orig_player;

    HexBoard* m_brd;

    HexState m_state;

    /** See UseICE() */
    bool m_use_ice;

    /** See NumberThreads() */
    std::size_t m_num_threads;

    std::size_t m_num_evals;

    std::size_t m_num_widenings;

    std::size_t m_value_updates;

    std::size_t m_priority_updates;

    std::size_t m_internal_nodes;

    std::size_t m_leaf_nodes;

    std::size_t m_terminal_nodes;

    /** Boards for each thread. */
    std::vector<HexBoard*> m_boards;
    
    /** Players for each thread. */
    std::vector<BenzenePlayer*> m_players;

    std::vector<Worker> m_workers;

    ThreadedWorker<SgMove,HexEval,Worker>* m_threadedWorker;

    void CreateWorkers();

    void DestroyWorkers();
};

//----------------------------------------------------------------------------

template<class PLAYER>
BookBuilder<PLAYER>::BookBuilder(PLAYER& player)
    : SgBookBuilder(), 
      m_orig_player(player),
      m_brd(0),
      m_use_ice(false),
      m_num_threads(1)
{
}

template<class PLAYER>
BookBuilder<PLAYER>::~BookBuilder()
{
}

//----------------------------------------------------------------------------

template<class PLAYER>
inline bool BookBuilder<PLAYER>::UseICE() const
{
    return m_use_ice;
}

template<class PLAYER>
inline void BookBuilder<PLAYER>::SetUseICE(bool flag)
{
    m_use_ice = flag;
}

template<class PLAYER>
inline std::size_t BookBuilder<PLAYER>::NumThreads() const
{
    return m_num_threads;
}

template<class PLAYER>
inline void BookBuilder<PLAYER>::SetNumThreads(std::size_t num)
{
    m_num_threads = num;
}

//----------------------------------------------------------------------------

/** Copies the player and board and creates the threads. */
template<class PLAYER>
void BookBuilder<PLAYER>::CreateWorkers()
{
    LogInfo() << "BookBuilder::CreateWorkers()\n";
    for (std::size_t i = 0; i < m_num_threads; ++i)
    {
        PLAYER* newPlayer = new PLAYER();
        /** @todo Use concept checking to verify this method exists. */
        newPlayer->CopySettingsFrom(m_orig_player);
        newPlayer->SetSearchSingleton(true);
        m_players.push_back(newPlayer);
        m_boards.push_back(new HexBoard(*m_brd));
        m_workers.push_back(Worker(i, *m_players[i], *m_boards[i]));
    }
    m_threadedWorker 
        = new ThreadedWorker<SgMove,HexEval,Worker>(m_workers);
}

/** Destroys copied players, boards, and threads. */
template<class PLAYER>
void BookBuilder<PLAYER>::DestroyWorkers()
{
    LogInfo() << "BookBuilder::DestroyWorkers()\n";
    for (std::size_t i = 0; i < m_num_threads; ++i)
    {
        delete m_boards[i];
        delete m_players[i];
    }
    delete m_threadedWorker;
    m_workers.clear();
    m_boards.clear();
    m_players.clear();
}

template<class PLAYER>
void BookBuilder<PLAYER>::Init()
{
    CreateWorkers();
}

template<class PLAYER>
void BookBuilder<PLAYER>::Fini()
{
    DestroyWorkers();
}

//----------------------------------------------------------------------------

template<class PLAYER>
BookBuilder<PLAYER>::Worker::Worker(std::size_t id, BenzenePlayer& player, 
                                    HexBoard& brd)

    : m_id(id), 
      m_brd(&brd),
      m_player(&player)
{
}

template<class PLAYER>
void BookBuilder<PLAYER>::Worker::SetState(const HexState& state)
{
    m_state = state;
}

template<class PLAYER>
HexEval BookBuilder<PLAYER>::Worker::operator()(const SgMove& move)
{
    HexState state(m_state);
    if (move >= 0)
        state.PlayMove(static_cast<HexPoint>(move));

    // stupid crap to meet interface of player
    StoneBoard blah(state.Position());
    Game game(blah);
    
    LogInfo() << "Evaluating: " << state.Position() << '\n';

    HexEval score;
    m_brd->GetPosition().SetPosition(state.Position());
    m_player->GenMove(state, game, *m_brd, 99999, score);
    return score;
}

//----------------------------------------------------------------------------

template<class PLAYER>
inline void BookBuilder<PLAYER>::SetState(const HexState& state)
{
    m_state = state;
}

template<class PLAYER>
inline void BookBuilder<PLAYER>::SetWorkBoard(HexBoard& workBoard)
{
    m_brd = &workBoard;
}

template<class PLAYER>
inline float BookBuilder<PLAYER>::InverseEval(float eval) const
{
    return BookUtil::InverseEval(eval);
}

template<class PLAYER>
inline bool BookBuilder<PLAYER>::IsLoss(float eval) const
{
    return HexEvalUtil::IsLoss(eval);
}

template<class PLAYER>
void BookBuilder<PLAYER>::PlayMove(SgMove move)
{
    m_state.PlayMove(static_cast<HexPoint>(move));
}

template<class PLAYER>
void BookBuilder<PLAYER>::UndoMove(SgMove move)
{
    m_state.UndoMove(static_cast<HexPoint>(move));
}

template<class PLAYER>
bool BookBuilder<PLAYER>::GetNode(BookNode& node) const
{
    return m_book->Get(m_state, node);
}

template<class PLAYER>
void BookBuilder<PLAYER>::WriteNode(const BookNode& node)
{
    m_book->Put(m_state, node);
}

template<class PLAYER>
float BookBuilder<PLAYER>::Value(const BookNode& node) const
{
    return BookUtil::Value(node, m_state);
}

template<class PLAYER>
void BookBuilder<PLAYER>::GetAllLegalMoves(std::vector<SgMove>& moves)
{
    for (BitsetIterator it(m_state.Position().GetEmpty()); it; ++it)
        moves.push_back(*it);
}

/** Creates root node if necessary. */
template<class PLAYER>
void BookBuilder<PLAYER>::EnsureRootExists()
{
    BookNode root;
    if (!GetNode(root))
    {
        m_workers[0].SetState(m_state);
        HexEval value = m_workers[0](SG_NULLMOVE);
        WriteNode(BookNode(value));
    }
}

/** Computes an ordered set of moves to consider. Returns true if
    state is determined, with the value set in value and moves
    untouched. Returns false otherwise, in which case moves will
    contain the sorted moves and value will be untouched. */
template<class PLAYER>
bool BookBuilder<PLAYER>::GenerateMoves(std::vector<SgMove>& moves,
                                        HexEval& value)
{
    // Turn off ICE (controlled by method UseICE()): compute the moves
    // to consider without using any ice, so that we do not leave the
    // book if opponent plays an inferiogr move.
    HexColor toMove = m_state.ToPlay();
    bool useICE = m_brd->UseICE();
    m_brd->SetUseICE(m_use_ice);
    m_brd->GetPosition().SetPosition(m_state.Position());
    m_brd->ComputeAll(toMove);
    m_brd->SetUseICE(useICE);

    if (EndgameUtils::IsDeterminedState(*m_brd, toMove, value))
        return true;

    bitset_t children = EndgameUtils::MovesToConsider(*m_brd, toMove);
    HexAssert(children.any());
    
    Resistance resist;
    resist.Evaluate(*m_brd);
    std::vector<HexMoveValue> ordered;
    for (BitsetIterator it(children); it; ++it)
        // use negative so higher values go to front
        ordered.push_back(HexMoveValue(*it, -resist.Score(*it)));
    std::stable_sort(ordered.begin(), ordered.end());
    for (std::size_t i = 0; i < ordered.size(); ++i)
        moves.push_back(ordered[i].point());
    return false;
}

template<class PLAYER>
void BookBuilder<PLAYER>::BeforeEvaluateChildren()
{
    for (std::size_t i = 0; i < m_workers.size(); ++i)
        m_workers[i].SetState(m_state);        
}

template<class PLAYER>
void BookBuilder<PLAYER>
::EvaluateChildren(const std::vector<SgMove>& childrenToDo,
                   std::vector<std::pair<SgMove, HexEval> >& scores)
{
    m_threadedWorker->DoWork(childrenToDo, scores);
}

template<class PLAYER>
void BookBuilder<PLAYER>::AfterEvaluateChildren()
{
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // BOOKBUILDER_HPP
