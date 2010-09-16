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
#include "SgBookBuilder.h"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Class for writing SgBookNodes to the database. */
class HexBookNode : public SgBookNode
{
public:
    HexBookNode();

    HexBookNode(float heurValue);

    HexBookNode(const SgBookNode& node);

    /** @name Methods for StateDBConcept (so it can be stored in a StateDB) */
    // @{

    int PackedSize() const;

    byte* Pack() const;

    void Unpack(const byte* t);

    void Rotate(const ConstBoard& brd);

    // @}

private:
};

inline HexBookNode::HexBookNode()
    : SgBookNode()
{
}

inline HexBookNode::HexBookNode(float heurValue)
    : SgBookNode(heurValue)
{
}

inline HexBookNode::HexBookNode(const SgBookNode& node)
    : SgBookNode(node)
{
}

inline int HexBookNode::PackedSize() const
{
    return sizeof(HexBookNode);
}

inline byte* HexBookNode::Pack() const
{
    return (byte*)this;
}

inline void HexBookNode::Unpack(const byte* t)
{
    *this = *(const HexBookNode*)t;
}

inline void HexBookNode::Rotate(const ConstBoard& brd)
{
    SG_UNUSED(brd);
    // No rotation-dependant data
}

//----------------------------------------------------------------------------

/** A book is just a database of BookNodes. */
class Book : public StateDB<HexBookNode>
{
public:
    static const std::string BOOK_DB_VERSION;

    Book(const std::string& filename) 
        : StateDB<HexBookNode>(filename, BOOK_DB_VERSION)
    { }
};

//----------------------------------------------------------------------------

/** Utilities on Books. 
    @ingroup openingbook
*/
namespace BookUtil
{
    /** Returns value of board, taking into account swap moves. */ 
    float Value(const SgBookNode& node, const HexState& brd);

    /** Returns score for this node, taking into account the amount of
        information in the subtree. Use to select moves when using
        book. Note the score is from the pov of the player moving into
        this position, not for the player to move in this position.
    */
    float Score(const SgBookNode& node, const HexState& brd, 
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
