//----------------------------------------------------------------------------
/** @file Book.hpp
 */
//----------------------------------------------------------------------------

#ifndef OPENINGBOOK_HPP
#define OPENINGBOOK_HPP

#include "Hex.hpp"
#include "HexBoard.hpp"
#include "HashDB.hpp"
#include "HexEval.hpp"
#include "StateDB.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** State in the Opening Book.
    Do not forget to update BOOK_DB_VERSION if this class changes in a
    way that invalidiates old books.
    @ingroup openingbook
 */
class BookNode
{
public:

    //------------------------------------------------------------------------

    /** Priority of newly created leaves. */
    static const float LEAF_PRIORITY = 0.0;

    //------------------------------------------------------------------------

    /** Heuristic value of this state. */
    float m_heurValue;

    /** Minmax value of this state. */
    float m_value;

    /** Expansion priority. */
    float m_priority;

    /** Number of times this node was explored. */
    unsigned m_count;
    
    //------------------------------------------------------------------------

    /** Constructors. */
    // @{

    BookNode();

    BookNode(float heuristicValue);

    // @}
    
    //------------------------------------------------------------------------

    /** @name Additional properties */
    // @{

    /** Returns true iff this node is a leaf in the opening book. */
    bool IsLeaf() const;

    /** Returns true if node's propagated value is a win or a loss. */
    bool IsTerminal() const;

    // @}

    //------------------------------------------------------------------------

    /** @name Update methods */
    // @{

    /** Increment the nodes counter. */
    void IncrementCount();

    // @}

    //------------------------------------------------------------------------

    /** @name Methods for StateDBConcept (so it can be stored in a StateDB) */
    // @{

    int PackedSize() const;

    byte* Pack() const;

    void Unpack(const byte* t);

    void Rotate(const ConstBoard& brd);

    // @}

    //------------------------------------------------------------------------

    /** Outputs node in string form. */
    std::string toString() const;

private:

};

inline BookNode::BookNode()
{
}

inline BookNode::BookNode(float heuristicValue)
    : m_heurValue(heuristicValue),
      m_value(heuristicValue),
      m_priority(LEAF_PRIORITY),
      m_count(0)
{
}

inline void BookNode::IncrementCount()
{
    m_count++;
}

inline int BookNode::PackedSize() const
{
    return sizeof(BookNode);
}

inline byte* BookNode::Pack() const
{
    return (byte*)this;
}

inline void BookNode::Unpack(const byte* t)
{
    *this = *(const BookNode*)t;
}

inline void BookNode::Rotate(const ConstBoard& brd)
{
    SG_UNUSED(brd);
    // No rotation-dependant data
}

//----------------------------------------------------------------------------

/** Extends standard stream output operator for BookNodes. */
inline std::ostream& operator<<(std::ostream& os, const BookNode& node)
{
    os << node.toString();
    return os;
}

//----------------------------------------------------------------------------

/** A book is just a database of BookNodes. */
class Book : public StateDB<BookNode>
{
public:
    static const std::string BOOK_DB_VERSION;

    Book(const std::string& filename) 
        : StateDB<BookNode>(filename, BOOK_DB_VERSION)
    { }
};

//----------------------------------------------------------------------------

/** Utilities on Books. 
    @ingroup openingbook
*/
namespace BookUtil
{
    /** Returns value of board, taking into account swap moves. */ 
    float Value(const BookNode& node, const HexState& brd);

    /** Returns score for this node, taking into account the amount of
        information in the subtree. Use to select moves when using
        book. Note the score is from the pov of the player moving into
        this position, not for the player to move in this position.
    */
    float Score(const BookNode& node, const HexState& brd, 
                float countWeight);

    /** Evaluation for other player. */
    float InverseEval(float eval);

    //------------------------------------------------------------------------

    /** Finds best response in book.
        @todo Does not consider SWAP_PIECES if it is available.
        Returns INVALID_POINT if not in book or if node's count is 
        less than minCount. */
    HexPoint BestMove(const Book& book, const HexState& state,
                      unsigned minCount, float countWeight);

    //-----------------------------------------------------------------------

    /** Writes a (score, depth) pair to output stream for each leaf in
        the book. Can be visualized with GnuPlot. */
    void DumpVisualizationData(const Book& book, const HexState& state, 
                               int depth, std::ostream& out);

    //-----------------------------------------------------------------------

    /** Writes variations leading to non-terminal leafs whose values
        differ from 0.5 by at least polarization. The given pv must be
        the variation leading to the current state of the board. */
    void DumpPolarizedLeafs(const Book& book, const HexState& state,
                            float polarization, PointSequence& pv, 
                            std::ostream& out, const StateSet& ignoreSet);

    /** Reads solved leaf positions from a file and adds them to the
        given book. Overwrites value of any existing states. */
    void ImportSolvedStates(Book& book, const ConstBoard& constBoard,
                            std::istream& positions);
    
    //-----------------------------------------------------------------------

    /** Returns the depth of the mainline from the given state. */
    int GetMainLineDepth(const Book& book, const HexState& state);

    /** Returns the number of nodes in the tree rooted at the current
        state. */
    std::size_t GetTreeSize(const Book& book, const HexState& state);
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // OPENINGBOOK_HPP
