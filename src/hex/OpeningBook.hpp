//----------------------------------------------------------------------------
/** @file OpeningBook.hpp
 */
//----------------------------------------------------------------------------

#ifndef OPENINGBOOK_HPP
#define OPENINGBOOK_HPP

#include "Hex.hpp"
#include "HexBoard.hpp"
#include "HashDB.hpp"
#include "HexException.hpp"
#include "HexEval.hpp"

//----------------------------------------------------------------------------
/** This code is based on Thomas R. Lincke's paper "Strategies for the
    Automatic Construction of Opening Books" published in 2001.
    
    We make the following adjustments:
    1) Neither side is assumed to be the book player, so the expansion
       formula is identical for all nodes (see page 80 of the paper). In other
       words, both sides can play sub-optimal moves.
    2) We do not include the swap rule as a move, since this would lead to
       redundant evaluation computations (such as a2-f6 and a2-swap-f6). This
       is fine since the value of the root position indicates whether or not
       to swap.
    3) Each node stores its successors for propagation value and expansion
       priority.
    4) These nodes will be referenced by a map or DB so that transpositions
       are not re-computed. So it is not a tree, but a DAG.
    
    We also think there is a typo with respect to the formula of epo_i on
    page 80. Namely, since p_i is the negamax of p_{s_j}s, then we should
    sum the values to find the distance from optimal, not subtract. That is,
    we use epo_i = 1 + min(s_j) (epb_{s_j} + alpha*(p_i + p_{s_j}) instead.

    TODO:
    - keep heurValue for interior nodes due to odd/even ply effect
    - incorporate swap rule into book as 2nd ply moves
    - will have single book for all openings
    - book expansion using game records (in addition to Lincke's formula)
    - need function for child/move selection, based on level of information
      and propagated heuristic value. The information level should be a
      function of the parent's information level, and not a direct comparison
      of siblings, because of transpositions into/from the mainline
*/
//----------------------------------------------------------------------------

/** A node in the Opening Book. */
class OpeningBookNode
{
public:

    //------------------------------------------------------------------------

    static const int DUMMY_VALUE = -9999999;
    static const int DUMMY_PRIORITY = 9999999;
    static const int LEAF_PRIORITY = 0;
    static const HexPoint DUMMY_SUCC = INVALID_POINT;
    static const HexPoint LEAF_SUCC = INVALID_POINT;

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

    /** Constructors. Note that we should only construct leaves. */
    // @{

    OpeningBookNode();

    OpeningBookNode(float heuristicValue);

    // @}
    
    //------------------------------------------------------------------------

    /** Returns value of board, taking into account swap moves. */ 
    float Value(const StoneBoard& brd) const;
    
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

    /** Methods for PackableConcept (so it can be used in a HashDB) */
    // @{

    int PackedSize() const;

    byte* Pack() const;

    void Unpack(const byte* t);

    // @}

    //------------------------------------------------------------------------

    /** Outputs node in string form. */
    std::string toString() const;

private:

};

inline OpeningBookNode::OpeningBookNode()
    : m_heurValue(DUMMY_VALUE),
      m_value(DUMMY_VALUE),
      m_priority(DUMMY_PRIORITY),
      m_count(0)
{
}

inline OpeningBookNode::OpeningBookNode(float heuristicValue)
    : m_heurValue(heuristicValue),
      m_value(heuristicValue),
      m_priority(LEAF_PRIORITY),
      m_count(0)
{
}

inline void OpeningBookNode::IncrementCount()
{
    m_count++;
}

inline int OpeningBookNode::PackedSize() const
{
    return sizeof(OpeningBookNode);
}

inline byte* OpeningBookNode::Pack() const
{
    return (byte*)this;
}

inline void OpeningBookNode::Unpack(const byte* t)
{
    *this = *(const OpeningBookNode*)t;
}

//----------------------------------------------------------------------------

/** Extends standard stream output operator for OpeningBookNodes. */
inline std::ostream& operator<<(std::ostream& os, const OpeningBookNode& node)
{
    os << node.toString();
    return os;
}

//----------------------------------------------------------------------------

/** Opening Book class. */
class OpeningBook
{
public:

    //------------------------------------------------------------------------

    /** Evaluation for other player. */
    static float InverseEval(float eval);

    //------------------------------------------------------------------------

    /** Settings for this book. */
    struct Settings
    {
        /** Board width for all states in this book. */
        int board_width;

        /** Board height for all states in this book. */
        int board_height;

        bool operator==(const Settings& o) const;
        
        bool operator!=(const Settings& o) const;

        std::string toString() const;
    };
    
    //---------------------------------------------------------------------

    /** Constructor. Creates an OpeningBook for boardsize (width,
        height) to be stored in the file filename.
    */
    OpeningBook(int width, int height, std::string filename)
        throw(HexException);

    /** Destructor. */
    ~OpeningBook();

    /** Returns a copy of the settings for this book. */
    Settings GetSettings() const;

    /** Reads node from db. Returns true if node exists in book, false
        otherwise. Node is touched only if it exists in book. */
    bool GetNode(const StoneBoard& brd, OpeningBookNode& node) const;

    /** Writes node to db. */
    void WriteNode(const StoneBoard& brd, const OpeningBookNode& node);

    /** Flushes the db to disk. */
    void Flush();

    //---------------------------------------------------------------------

    /** Returns the depth of the mainline from the given position. */
    int GetMainLineDepth(const StoneBoard& pos, HexColor color) const;

    /** Returns the number of nodes in the tree rooted at the current
        position. */
    std::size_t GetTreeSize(StoneBoard& brd, HexColor color) const;

private:

    /** Settings for this book. */
    Settings m_settings;

    /** Database for this book. */
    HashDB<OpeningBookNode> m_db;

    hash_t GetHash(const StoneBoard& brd) const;

    std::size_t TreeSize(StoneBoard& brd, HexColor color,
                         std::map<hash_t, std::size_t>& solved) const;

};

inline bool 
OpeningBook::Settings::operator==(const OpeningBook::Settings& o) const 
{
    return (board_width == o.board_width && 
            board_height == o.board_height);
}
        
inline bool 
OpeningBook::Settings::operator!=(const OpeningBook::Settings& o) const
{
    return !(*this == o);
}

inline std::string OpeningBook::Settings::toString() const
{
    std::ostringstream os;
    os << "["
       << "W=" << board_width << ", "
       << "H=" << board_height 
       << "]";
    return os.str();
}

inline OpeningBook::Settings OpeningBook::GetSettings() const
{
    return m_settings;
}

inline void OpeningBook::Flush()
{
    m_db.Flush();
}

//----------------------------------------------------------------------------

/** Utilities on OpeningBooks. */
namespace OpeningBookUtil
{

    /** Returns the priority of expanding the child node. */
    float ComputePriority(const StoneBoard& brd, 
                          const OpeningBookNode& parent,
                          const OpeningBookNode& child,
                          double alpha);
    
    /** Re-computes node's value by checking all children. Does
        nothing if node has no children. */
    void UpdateValue(const OpeningBook& book, OpeningBookNode& node, 
                     StoneBoard& brd);

    /** Re-computes node's priority and returns the best child to
        expand. Requires that UpdateValue() has been called on this
        node. Returns INVALID_POINT if node has no children. */
    HexPoint UpdatePriority(const OpeningBook& book, OpeningBookNode& node, 
                            StoneBoard& brd, float alpha);
}

//----------------------------------------------------------------------------

#endif // OPENINGBOOK_HPP
