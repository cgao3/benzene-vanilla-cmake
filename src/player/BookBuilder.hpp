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
#include "PositionDB.hpp"
#include "Resistance.hpp"
#include "ThreadedWorker.hpp"
#include "Time.hpp"

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
class BookBuilder
{
public:

    /** Constructor. Takes a reference to the player that will
        evaluate states, which must be a reference to a
        BenzenePlayer. */
    BookBuilder(PLAYER& player);
    
    /** Destructor. */
    ~BookBuilder();

    //---------------------------------------------------------------------

    /** Expands the book by expanding numExpansions leaves. */
    void Expand(Book& book, const HexBoard& brd, int numExpansions);

    /** Propagates leaf values up through the entire tree.  
        @ref bookrefresh. */
    void Refresh(Book& book, HexBoard& board);

    /** Performs widening on all internal nodes that require it. Use
        this after increasing ExpandWidth() or decreasing
        ExpandThreshold() on an already existing book to update all
        the internal nodes with the new required width. Will do
        nothing unless parameters were changed accordingly.
        
        Does not propagate values up tree, run Refresh() afterwards to
        do so. */
    void IncreaseWidth(Book& book, HexBoard& board);

    //---------------------------------------------------------------------    

    /** The parameter alpha controls state expansion (big values give
        rise to deeper lines, while small values perform like a
        BFS). */
    float Alpha() const;

    /** See Alpha() */
    void SetAlpha(float alpha);

    /** Expand only the top ExpandWidth() children of a node
        initially, and after every ExpansionThreshold() visits add
        ExpandWidth() more children. */
    bool UseWidening() const;

    /** See UseWidening() */
    void SetUseWidening(bool flag);
    
    /** See UseWidening() */
    std::size_t ExpandWidth() const;

    /** See UseWidening() */
    void SetExpandWidth(std::size_t width);

    /** See UseWidening() */
    std::size_t ExpandThreshold() const;

    /** See UseWidening() */
    void SetExpandThreshold(std::size_t threshold);

    /** Number of players to use during leaf expansion. Each player
        may use a multi-threaded search. Should speed up the expansion
        of leaf states by a factor of (very close to) NumThreads(). */
    std::size_t NumThreads() const;

    /** See NumThreads() */
    void SetNumThreads(std::size_t num);

    /** Whether to prune out inferior cells from the book or not. */
    bool UseICE() const;

    /** See UseICE() */
    void SetUseICE(bool flag);

private:

    /** Copyable worker. */
    class Worker
    {
    public:
        Worker(std::size_t id, BenzenePlayer& player, HexBoard& brd); 

        HexEval operator()(const StoneBoard& position);

    private:

        std::size_t m_id;
        
        HexBoard* m_brd;
        
        BenzenePlayer* m_player;
    };

    bool GetNode(const StoneBoard& brd, BookNode& node) const;

    void WriteNode(const StoneBoard& brd, const BookNode& node);

    void EnsureRootExists(const StoneBoard& brd);

    bool GenerateMoves(const StoneBoard& brd, std::vector<HexPoint>& moves,
                       HexEval& value);

    bool ExpandChildren(StoneBoard& brd, std::size_t count);

    void UpdateValue(BookNode& node, StoneBoard& brd);

    void DoExpansion(StoneBoard& brd, PointSequence& pv);

    bool Refresh(StoneBoard& brd, PositionSet& seen, bool root);

    void IncreaseWidth(StoneBoard& brd, PositionSet& seen, bool root);

    void CreateWorkers();

    void DestroyWorkers();

    //---------------------------------------------------------------------

    /** Book this builder is expanding, passed in by user in Expand(). */
    Book* m_book;

    /** Player passed to constructor. */
    PLAYER& m_orig_player;
    
    /** Work board used for move ordering, passed in by user in Expand(). */
    HexBoard* m_brd;

    /** See Alpha() */
    float m_alpha;

    /** See UseWidening() */
    bool m_use_widening;

    /** See UseICE() */
    bool m_use_ice;

    /** See UseWidening() */
    std::size_t m_expand_width;

    /** See UseWidening() */
    std::size_t m_expand_threshold;
    
    /** Number of iterations after which the db is flushed to disk. */
    std::size_t m_flush_iterations;

    /** See NumberThreads() */
    std::size_t m_num_threads;

    //------------------------------------------------------------------------

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

    ThreadedWorker<StoneBoard,HexEval,Worker>* m_threadedWorker;
};

//----------------------------------------------------------------------------

template<class PLAYER>
BookBuilder<PLAYER>::BookBuilder(PLAYER& player)
    : m_book(0),
      m_orig_player(player),
      m_brd(0),
      m_alpha(50),
      m_use_widening(true),
      m_use_ice(false),
      m_expand_width(16),
      m_expand_threshold(1000),
      m_flush_iterations(100),
      m_num_threads(1)
{
}

template<class PLAYER>
BookBuilder<PLAYER>::~BookBuilder()
{
}

//----------------------------------------------------------------------------

template<class PLAYER>
inline float BookBuilder<PLAYER>::Alpha() const
{
    return m_alpha;
}

template<class PLAYER>
inline void BookBuilder<PLAYER>::SetAlpha(float alpha)
{
    m_alpha = alpha;
}

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
inline bool BookBuilder<PLAYER>::UseWidening() const
{
    return m_use_widening;
}

template<class PLAYER>
inline void BookBuilder<PLAYER>::SetUseWidening(bool flag)
{
    m_use_widening = flag;
}

template<class PLAYER>
inline std::size_t BookBuilder<PLAYER>::ExpandWidth() const
{
    return m_expand_width;
}

template<class PLAYER>
inline void BookBuilder<PLAYER>::SetExpandWidth(std::size_t width)
{
    m_expand_width = width;
}

template<class PLAYER>
inline std::size_t BookBuilder<PLAYER>::ExpandThreshold() const
{
    return m_expand_threshold;
}

template<class PLAYER>
inline void BookBuilder<PLAYER>::SetExpandThreshold(std::size_t threshold)
{
    m_expand_threshold = threshold;
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

template<class PLAYER>
void BookBuilder<PLAYER>::Expand(Book& book, const HexBoard& board, 
                                 int numExpansions)
{
    m_book = &book;
    m_brd = const_cast<HexBoard*>(&board);
    StoneBoard brd(board.GetState());
    double s = Time::Get();
    m_num_evals = 0;
    m_num_widenings = 0;

    CreateWorkers();
    
    EnsureRootExists(brd);

    int num = 0;
    for (; num < numExpansions; ++num) 
    {
	LogInfo() << "\n--Iteration " << num << "--" << '\n';
	
        // Flush the db if we've performed enough iterations
        if (num && (num % m_flush_iterations) == 0) 
        {
            LogInfo() << "Flushing DB..." << '\n';
            m_book->Flush();
        }
        
	// If root position becomes a known win or loss, then there's
	// no point in continuing to expand the opening book.
        {
            BookNode root;
            GetNode(brd, root);
            if (root.IsTerminal()) 
            {
                LogInfo() << "Position solved!" << '\n';
                break;
            }
        }
	
        PointSequence pv;
        DoExpansion(brd, pv);
    }

    LogInfo() << "Flushing DB..." << '\n';
    m_book->Flush();

    double e = Time::Get();

    DestroyWorkers();

    LogInfo() << '\n'
              << "  Total Time: " << Time::Formatted(e - s) << '\n'
              << "  Expansions: " << num 
              << std::fixed << std::setprecision(2) 
              << " (" << (num / (e - s)) << "/s)" << '\n'
              << " Evaluations: " << m_num_evals 
              << std::fixed << std::setprecision(2)
              << " (" << (m_num_evals / (e - s)) << "/s)" << '\n'
              << "   Widenings: " << m_num_widenings << '\n';
}

template<class PLAYER>
void BookBuilder<PLAYER>::Refresh(Book& book, HexBoard& board)
{
    m_book = &book;
    m_brd = const_cast<HexBoard*>(&board);
    StoneBoard brd(board.GetState());
    double s = Time::Get();
    m_num_evals = 0;
    m_num_widenings = 0;
    m_value_updates = 0;
    m_priority_updates = 0;
    m_internal_nodes = 0;
    m_leaf_nodes = 0;
    m_terminal_nodes = 0;

    CreateWorkers();

    LogInfo() << "Refreshing DB..." << '\n';
    PositionSet seen;
    Refresh(brd, seen, true);

    LogInfo() << "Flushing DB..." << '\n';
    m_book->Flush();

    double e = Time::Get();

    DestroyWorkers();

    LogInfo() << '\n'
              << "      Total Time: " << Time::Formatted(e - s) << '\n'
              << "   Value Updates: " << m_value_updates << '\n'
              << "Priority Updates: " << m_priority_updates << '\n'
              << "  Internal Nodes: " << m_internal_nodes << '\n'
              << "  Terminal Nodes: " << m_terminal_nodes << '\n'
              << "      Leaf Nodes: " << m_leaf_nodes << '\n'
              << "     Evaluations: " << m_num_evals 
              << std::fixed << std::setprecision(2)
              << " (" << (m_num_evals / (e - s)) << "/s)" << '\n'
              << "       Widenings: " << m_num_widenings << '\n';
}

template<class PLAYER>
void BookBuilder<PLAYER>::IncreaseWidth(Book& book, HexBoard& board)
{
    if (!m_use_widening)
    {
        LogInfo() << "Widening not enabled!" << '\n';
        return;
    }

    m_book = &book;
    m_brd = const_cast<HexBoard*>(&board);
    StoneBoard brd(board.GetState());
    double s = Time::Get();
    m_num_evals = 0;
    m_num_widenings = 0;

    CreateWorkers();

    LogInfo() << "Increasing DB's width..." << '\n';
    PositionSet seen;
    IncreaseWidth(brd, seen, true);

    LogInfo() << "Flushing DB..." << '\n';
    m_book->Flush();

    double e = Time::Get();

    DestroyWorkers();

    LogInfo() << '\n'
              << "      Total Time: " << Time::Formatted(e - s) << '\n'
              << "       Widenings: " << m_num_widenings << '\n'
              << "     Evaluations: " << m_num_evals 
              << std::fixed << std::setprecision(2)
              << " (" << (m_num_evals / (e - s)) << "/s)" << '\n';
}

/** Copies the player and board and creates the threads. */
template<class PLAYER>
void BookBuilder<PLAYER>::CreateWorkers()
{
    LogInfo() << "BookBuilder::CreateWorkers()" << '\n';
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
        = new ThreadedWorker<StoneBoard,HexEval,Worker>(m_workers);
}

/** Destroys copied players, boards, and threads. */
template<class PLAYER>
void BookBuilder<PLAYER>::DestroyWorkers()
{
    LogInfo() << "BookBuilder::DestroyWorkers()" << '\n';
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
HexEval BookBuilder<PLAYER>::Worker::operator()(const StoneBoard& position)
{
    // stupid crap to meet interface of player
    StoneBoard blah(position);
    Game game(blah);

    HexEval score;
    m_brd->GetState().SetState(position);
    m_player->GenMove(*m_brd, game, m_brd->GetState().WhoseTurn(), 
                      99999, score);
    return score;
}

//----------------------------------------------------------------------------

/** Reads node for given board state. Returns false if state does not
    exist in the book. */
template<class PLAYER>
bool BookBuilder<PLAYER>::GetNode(const StoneBoard& brd, BookNode& node) const
{
    return m_book->Get(brd, node);
}

/** Writes node to book's db. */
template<class PLAYER>
void BookBuilder<PLAYER>::WriteNode(const StoneBoard& brd, 
                                    const BookNode& node)
{
    m_book->Put(brd, node);
}

/** Creates root node if necessary. */
template<class PLAYER>
void BookBuilder<PLAYER>::EnsureRootExists(const StoneBoard& brd)
{
    BookNode root;
    if (!GetNode(brd, root))
    {
        LogInfo() << "Creating root node. " << '\n';
        HexEval value = m_workers[0](brd);
        WriteNode(brd, BookNode(value));
    }
}

/** Computes an ordered set of moves to consider. Returns true if
    state is determined, with the value set in value and moves
    untouched. Returns false otherwise, in which case moves will
    contain the sorted moves and value will be untouched. */
template<class PLAYER>
bool BookBuilder<PLAYER>::GenerateMoves(const StoneBoard& brd, 
                                        std::vector<HexPoint>& moves,
                                        HexEval& value)
{
    // Turn off ICE (controlled by method UseICE()): compute the moves
    // to consider without using any ice, so that we do not leave the
    // book if opponent plays an inferior move.
    HexColor toMove = brd.WhoseTurn();
    bool useICE = m_brd->UseICE();
    m_brd->SetUseICE(m_use_ice);
    m_brd->GetState().SetState(brd);
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
    for (std::size_t i=0; i < ordered.size(); ++i)
        moves.push_back(ordered[i].point());
    return false;
}

/** Creates a node for each of the leaf's first count children that
    have not been created yet. Returns true if at least one new node
    was created, false otherwise. */
template<class PLAYER>
bool BookBuilder<PLAYER>::ExpandChildren(StoneBoard& brd, std::size_t count)
{
    // It is possible the state is determined, even though it was
    // already evaluated. This is not very likey if the evaluation
    // function is reasonably heavyweight, but if just using fillin
    // and vcs, it is possible that the fillin prevents a winning vc
    // from being created.
    HexEval value = 0;
    std::vector<HexPoint> children;
    if (GenerateMoves(brd, children, value))
    {
        LogInfo() << "ExpandChildren: State is determined!" << '\n';
        WriteNode(brd, BookNode(value));
        return false;
    }

    std::size_t limit = std::min(count, children.size());
    std::vector<StoneBoard> workToDo;
    bitset_t childrenToDo;
    for (std::size_t i = 0; i < limit; ++i)
    {
        brd.PlayMove(brd.WhoseTurn(), children[i]);
        BookNode child;
        if (!GetNode(brd, child))
        {
            workToDo.push_back(brd);
            childrenToDo.set(children[i]);
        }
        brd.UndoMove(children[i]);
    }
    if (!workToDo.empty())
    {
        LogInfo() << "Will evaluate these children: " 
                  << brd.Write(childrenToDo) << '\n';
        std::vector<std::pair<StoneBoard, HexEval> > scores;
        m_threadedWorker->DoWork(workToDo, scores);
        for (std::size_t i = 0; i < scores.size(); ++i)
            WriteNode(scores[i].first, scores[i].second);
        m_num_evals += workToDo.size();
        return true;
    }
    else
        LogInfo() << "Children already evaluated." << '\n';
    return false;
}

/** Updates the node's value, taking special care if the value is a
    loss. In this case, widenings are performed until a non-loss child
    is added or no new children are added. The node is then set with
    the proper value. */
template<class PLAYER>
void BookBuilder<PLAYER>::UpdateValue(BookNode& node, StoneBoard& brd)
{
    while (true)
    {
        BookUtil::UpdateValue(*m_book, node, brd);
        if (!HexEvalUtil::IsLoss(node.Value(brd)))
            break;

        // Round up to next nearest multiple of m_expand_width that is
        // larger than the current number of children.
        unsigned numChildren = BookUtil::NumChildren(*m_book, brd);
        std::size_t width = (numChildren / m_expand_width + 1) 
            * m_expand_width;

        LogInfo() << "Forced Widening[" << numChildren << "->" 
                  << width << "]" << '\n' << brd << '\n';
        if (!ExpandChildren(brd, width))
            break;

        ++m_num_widenings;
    }
}

template<class PLAYER>
void BookBuilder<PLAYER>::DoExpansion(StoneBoard& brd, PointSequence& pv)
{
    BookNode node;
    if (!GetNode(brd, node))
        HexAssert(false);

    if (node.IsTerminal())
        return;

    if (node.IsLeaf())
    {
        // Expand this leaf's children
        LogInfo() << "Expanding: " << HexPointUtil::ToString(pv) << '\n';
        ExpandChildren(brd, m_expand_width);
    }
    else
    {
        // Widen this non-terminal non-leaf node if necessary
        if (m_use_widening && (node.m_count % m_expand_threshold == 0))
        {
            std::size_t width = (node.m_count / m_expand_threshold + 1)
                              * m_expand_width;
            LogInfo() << "Widening[" << width << "]: " 
                      << HexPointUtil::ToString(pv) << '\n';
            ++m_num_widenings;
            ExpandChildren(brd, width);
        }

        // Compute value and priority. It's possible this node is newly
        // terminal if one of its new children is a winning move.
        GetNode(brd, node);
        UpdateValue(node, brd);
        HexPoint mostUrgent 
            = BookUtil::UpdatePriority(*m_book, node, brd, m_alpha);
        WriteNode(brd, node);

        // Recurse on most urgent child only if non-terminal.
        if (!node.IsTerminal())
        {
            brd.PlayMove(brd.WhoseTurn(), mostUrgent);
            pv.push_back(mostUrgent);
            DoExpansion(brd, pv);
            pv.pop_back();
            brd.UndoMove(mostUrgent);
        }
    }

    GetNode(brd, node);
    UpdateValue(node, brd);
    BookUtil::UpdatePriority(*m_book, node, brd, m_alpha);
    node.IncrementCount();
    WriteNode(brd, node);
}

//----------------------------------------------------------------------------

/** Refresh's each child of the given state. UpdateValue() and
    UpdatePriority() are called on internal nodes. Returns true if
    state exists in book.  
    @ref bookrefresh
*/
template<class PLAYER>
bool BookBuilder<PLAYER>::Refresh(StoneBoard& brd, PositionSet& seen,
                                  bool root)
{
    if (seen.Exists(brd))
        return true;
    BookNode node;
    if (!GetNode(brd, node))
        return false;
    if (node.IsLeaf())
    {
        m_leaf_nodes++;
        if (node.IsTerminal())
            m_terminal_nodes++;
        return true;
    }
    double oldValue = node.Value(brd);
    double oldPriority = node.m_priority;
    for (BitsetIterator it(brd.GetEmpty()); it; ++it)
    {
        brd.PlayMove(brd.WhoseTurn(), *it);
        Refresh(brd, seen, false);
        if (root)
            LogInfo() << "Finished " << *it << '\n';
        brd.UndoMove(*it);
    }
    UpdateValue(node, brd);
    BookUtil::UpdatePriority(*m_book, node, brd, m_alpha);
    if (fabs(oldValue - node.Value(brd)) > 0.0001)
        m_value_updates++;
    if (fabs(oldPriority - node.m_priority) > 0.0001)
        m_priority_updates++;
    WriteNode(brd, node);
    seen.Insert(brd);
    if (node.IsTerminal())
        m_terminal_nodes++;
    else
        m_internal_nodes++;
    return true;
}

//----------------------------------------------------------------------------

template<class PLAYER>
void BookBuilder<PLAYER>::IncreaseWidth(StoneBoard& brd, PositionSet& seen,
                                        bool root)
{
    if (seen.Exists(brd))
        return;
    BookNode node;
    if (!GetNode(brd, node))
        return;
    if (node.IsTerminal() || node.IsLeaf())
        return;
    for (BitsetIterator it(brd.GetEmpty()); it; ++it)
    {
        brd.PlayMove(brd.WhoseTurn(), *it);
        IncreaseWidth(brd, seen, false);
        if (root)
            LogInfo() << "Finished " << *it << '\n';
        brd.UndoMove(*it);
    }
    std::size_t width = (node.m_count / m_expand_threshold + 1)
        * m_expand_width;
    if (ExpandChildren(brd, width))
        ++m_num_widenings;
    seen.Insert(brd);
    if ((seen.Size() % 500) == 0)
    {
        m_book->Flush();
        LogInfo() << "Flushed book." << '\n';
    }
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // BOOKBUILDER_HPP
