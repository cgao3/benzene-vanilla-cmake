//----------------------------------------------------------------------------
/** @file SgBookBuilder.hpp
 */
//----------------------------------------------------------------------------

#ifndef SG_BOOKBUILDER_HPP
#define SG_BOOKBUILDER_HPP

#include <cmath>
#include "Book.hpp"
#include "Time.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** @page bookrefresh Book Refresh
    @ingroup openingbook

    Due to transpositions, it is possible that a node's value changes,
    but because the node has not been revisited yet the information is
    not passed to its parent. Refreshing the book forces these
    propagations.

    SgBookBuilder::Refresh() computes the correct propagation value for
    all internal nodes given the current set of leaf nodes. A node in
    which BookNode::IsLeaf() is true is treated as a leaf even
    if it has children in the book (ie, children from transpositions)
*/

//----------------------------------------------------------------------------

/** Expands a Book using the given player to evaluate
    game positions positions. 

    Supports multithreaded evaluation of states -- NOT CURRENTLY!

    @ingroup openingbook
*/
class SgBookBuilder
{
public:

    /** Constructor. Takes a reference to the player that will
        evaluate states, which must be a reference to a
        BenzenePlayer. */
    SgBookBuilder();
    
    /** Destructor. */
    virtual ~SgBookBuilder();

    //---------------------------------------------------------------------

    /** Expands the book by expanding numExpansions leaves. */
    void Expand(Book& book, int numExpansions);

    /** Propagates leaf values up through the entire tree.  
        @ref bookrefresh. */
    void Refresh(Book& book);

    /** Performs widening on all internal nodes that require it. Use
        this after increasing ExpandWidth() or decreasing
        ExpandThreshold() on an already existing book to update all
        the internal nodes with the new required width. Will do
        nothing unless parameters were changed accordingly.
        
        Does not propagate values up tree, run Refresh() afterwards to
        do so. */
    void IncreaseWidth(Book& book);

    //---------------------------------------------------------------------    

    float ComputePriority(const BookNode& parent, const float childValue,
                          const float childPriority) const;

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

protected:

    /** Book this builder is expanding */
    Book* m_book;
   
    /** See Alpha() */
    float m_alpha;

    /** See UseWidening() */
    bool m_use_widening;

    /** See UseWidening() */
    std::size_t m_expand_width;

    /** See UseWidening() */
    std::size_t m_expand_threshold;
    
    /** Number of iterations after which the db is flushed to disk. */
    std::size_t m_flush_iterations;

    //------------------------------------------------------------------------

    /** Returns the evaluation from other player's perspective. */
    virtual float InverseEval(float eval) const = 0;

    virtual bool IsLoss(float eval) const = 0;

    virtual void PlayMove(SgMove move) = 0;

    virtual void UndoMove(SgMove move) = 0;

    /** Returns the value of the state according this node.
        Ie, takes into account swap moves, etc. */
    virtual float Value(const BookNode& node) const = 0;

    /** Reads node. Returns false if node does not exist. */
    virtual bool GetNode(BookNode& node) const = 0;

    /** Writes node. */
    virtual void WriteNode(const BookNode& node) = 0;

    /** If current state does not exist, evaluate it and store in the
        book. */
    virtual void EnsureRootExists() = 0;

    /** Generates the set of moves to use in the book for this state. */
    virtual bool GenerateMoves(std::vector<SgMove>& moves, HexEval& value) = 0;

    /** Returns all legal moves; should be a superset of those moves 
        returned by GenerateMoves() */
    virtual void GetAllLegalMoves(std::vector<SgMove>& moves) = 0;

    /** Evaluate the children of the current state, return the values
        in a vector of pairs. */
    virtual void EvaluateChildren(const std::vector<SgMove>& childrenToDo,
                    std::vector<std::pair<SgMove, HexEval> >& scores) = 0;

    /** Hook function: called before any work is done. */
    virtual void Init() = 0;

    virtual void BeforeEvaluateChildren();

    virtual void AfterEvaluateChildren();

    /** Hook function: called after all work is complete. */
    virtual void Fini() = 0;

private:

    std::size_t m_num_evals;

    std::size_t m_num_widenings;

    std::size_t m_value_updates;

    std::size_t m_priority_updates;

    std::size_t m_internal_nodes;

    std::size_t m_leaf_nodes;

    std::size_t m_terminal_nodes;

    //---------------------------------------------------------------------

    std::size_t NumChildren(const std::vector<SgMove>& legal);

    void UpdateValue(BookNode& node, const std::vector<SgMove>& legal);

    void UpdateValue(BookNode& node);

    SgMove UpdatePriority(BookNode& node);

    void DoExpansion(std::vector<SgMove>& pv);

    bool Refresh(bool root);

    void IncreaseWidth(bool root);
    
    bool ExpandChildren(std::size_t count);
};

//----------------------------------------------------------------------------

inline float SgBookBuilder::Alpha() const
{
    return m_alpha;
}

inline void SgBookBuilder::SetAlpha(float alpha)
{
    m_alpha = alpha;
}

inline bool SgBookBuilder::UseWidening() const
{
    return m_use_widening;
}

inline void SgBookBuilder::SetUseWidening(bool flag)
{
    m_use_widening = flag;
}

inline std::size_t SgBookBuilder::ExpandWidth() const
{
    return m_expand_width;
}

inline void SgBookBuilder::SetExpandWidth(std::size_t width)
{
    m_expand_width = width;
}

inline std::size_t SgBookBuilder::ExpandThreshold() const
{
    return m_expand_threshold;
}

inline void SgBookBuilder::SetExpandThreshold(std::size_t threshold)
{
    m_expand_threshold = threshold;
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // SG_BOOKBUILDER_HPP
