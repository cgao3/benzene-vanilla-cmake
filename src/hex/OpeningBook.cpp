//----------------------------------------------------------------------------
/** @file OpeningBook.cpp
*/
//----------------------------------------------------------------------------

#include <boost/numeric/conversion/bounds.hpp>

#include "BitsetIterator.hpp"
#include "HexException.hpp"
#include "OpeningBook.hpp"
#include "Time.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

/** Dump debug info. */
#define OUTPUT_OB_INFO 1

//----------------------------------------------------------------------------

float OpeningBookNode::Value(const StoneBoard& brd) const
{
    if (brd.isLegal(SWAP_PIECES))
        return std::max(m_value, OpeningBook::InverseEval(m_value));
    return m_value;
}

bool OpeningBookNode::IsTerminal() const
{
    if (HexEvalUtil::IsWinOrLoss(m_value))
        return true;
    return false;
}

bool OpeningBookNode::IsLeaf() const
{
    return m_count == 0;
}

std::string OpeningBookNode::toString() const
{
    std::ostringstream os;
    os << std::showpos << std::fixed << std::setprecision(3);
    os << "Prop=" << m_value;
    os << std::noshowpos << ", ExpP=" << m_priority;
    os << std::showpos << ", Heur=" << m_heurValue << ", Cnt=" << m_count;
    return os.str();
}

//----------------------------------------------------------------------------

OpeningBook::OpeningBook(int width, int height, std::string filename)
    throw(HexException)
{
    m_settings.board_width = width;
    m_settings.board_height = height;

    if (!m_db.Open(filename))
        throw HexException("Could not open database file!");

    // Load settings from database and ensure they match the current
    // settings.
    char key[] = "settings";
    Settings temp;
    if (m_db.Get(key, strlen(key)+1, &temp, sizeof(temp))) 
    {
        LogInfo() << "Old book." << '\n';
        if (m_settings != temp) 
        {
            LogInfo() << "Settings do not match book settings!" << '\n'
		      << "Book: " << temp.toString() << '\n'
		      << "Current: " << m_settings.toString() << '\n';
            throw HexException("Book settings don't match given settings!");
        } 
    } 
    else 
    {
        // Read failed: this is a new database. Store the settings.
        LogInfo() << "New book!" << '\n';
        if (!m_db.Put(key, strlen(key)+1, &m_settings, sizeof(m_settings)))
            throw HexException("Could not write settings!");
    }
}

OpeningBook::~OpeningBook()
{
}

//----------------------------------------------------------------------------

float OpeningBook::InverseEval(float eval)
{
    if (HexEvalUtil::IsWinOrLoss(eval))
        return -eval;
    if (eval < 0 || eval > 1)
        LogInfo() << "eval = " << eval << '\n';
    HexAssert(0 <= eval && eval <= 1.0);
    return 1.0 - eval;
}

//----------------------------------------------------------------------------

bool OpeningBook::GetNode(const StoneBoard& brd, OpeningBookNode& node) const
{
    if (m_db.Get(OpeningBookUtil::GetHash(brd), node))
        return true;
    return false;
}

void OpeningBook::WriteNode(const StoneBoard& brd, const OpeningBookNode& node)
{
    m_db.Put(OpeningBookUtil::GetHash(brd), node);
}

int OpeningBook::GetMainLineDepth(const StoneBoard& pos) const
{
    int depth = 0;
    StoneBoard brd(pos);
    for (;;) 
    {
        OpeningBookNode node;
        if (!GetNode(brd, node))
            break;
        HexPoint move = INVALID_POINT;
        float value = -1e9;
        for (BitsetIterator p(brd.getEmpty()); p; ++p)
        {
            brd.playMove(brd.WhoseTurn(), *p);
            OpeningBookNode child;
            if (GetNode(brd, child))
            {
                float curValue = InverseEval(child.Value(brd));
                if (curValue > value)
                {
                    value = curValue;
                    move = *p;
                }
            }
            brd.undoMove(*p);
        }
        if (move == INVALID_POINT)
            break;
        brd.playMove(brd.WhoseTurn(), move);
        depth++;
    }
    return depth;
}

std::size_t OpeningBook::GetTreeSize(const StoneBoard& board) const
{
    std::map<hash_t, std::size_t> solved;
    StoneBoard brd(board);
    return TreeSize(brd, solved);
}

std::size_t OpeningBook::TreeSize(StoneBoard& brd,
                                  std::map<hash_t, std::size_t>& solved) const
{
    hash_t hash = OpeningBookUtil::GetHash(brd);
    if (solved.find(hash) != solved.end())
        return solved[hash];

    OpeningBookNode node;
    if (!GetNode(brd, node))
        return 0;
   
    std::size_t ret = 1;
    for (BitsetIterator p(brd.getEmpty()); p; ++p) 
    {
        brd.playMove(brd.WhoseTurn(), *p);
        ret += TreeSize(brd, solved);
        brd.undoMove(*p);
    }
    solved[hash] = ret;
    return ret;
}

//----------------------------------------------------------------------------

hash_t OpeningBookUtil::GetHash(const StoneBoard& brd)
{
    hash_t hash1 = brd.Hash();
    StoneBoard rotatedBrd(brd);
    rotatedBrd.rotateBoard();
    hash_t hash2 = rotatedBrd.Hash();
    return std::min(hash1, hash2);
}

void OpeningBookUtil::UpdateValue(const OpeningBook& book, 
                                  OpeningBookNode& node, StoneBoard& brd)
{
    bool hasChild = false;
    float bestValue = boost::numeric::bounds<float>::lowest();
    for (BitsetIterator i(brd.getEmpty()); i; ++i) 
    {
	brd.playMove(brd.WhoseTurn(), *i);
	OpeningBookNode child;
        if (book.GetNode(brd, child))
        {
            hasChild = true;
            float value = OpeningBook::InverseEval(child.Value(brd));
            if (value > bestValue)
		bestValue = value;
	    
        }
        brd.undoMove(*i);
    }
    if (hasChild)
        node.m_value = bestValue;
}

/** @todo Maybe switch this to take a bestChildValue instead of of a
    parent node. This would require flipping the parent in the caller
    function and reverse the order of the subtraction. */
float OpeningBookUtil::ComputePriority(const StoneBoard& brd, 
                                       const OpeningBookNode& parent,
                                       const OpeningBookNode& child,
                                       double alpha)
{
    // Must adjust child value for swap, but not the parent because we
    // are comparing with the best child's value, ie, the minmax
    // value.
    float delta = parent.m_value - OpeningBook::InverseEval(child.Value(brd));
    HexAssert(delta >= 0.0);
    HexAssert(child.m_priority >= OpeningBookNode::LEAF_PRIORITY);
    HexAssert(child.m_priority < OpeningBookNode::DUMMY_PRIORITY);
    return alpha * delta + child.m_priority + 1;
}

HexPoint OpeningBookUtil::UpdatePriority(const OpeningBook& book,
                                         OpeningBookNode& node, 
                                         StoneBoard& brd,
                                         float alpha)
{
    bool hasChild = false;
    float bestPriority = boost::numeric::bounds<float>::highest();
    HexPoint bestChild = INVALID_POINT;
    for (BitsetIterator i(brd.getEmpty()); i; ++i) 
    {
	brd.playMove(brd.WhoseTurn(), *i);
	OpeningBookNode child;
        if (book.GetNode(brd, child))
        {
            hasChild = true;
            float priority 
                = OpeningBookUtil::ComputePriority(brd, node, child, alpha);
            if (priority < bestPriority)
            {
                bestPriority = priority;
                bestChild = *i;
            }
        }
        brd.undoMove(*i);
    }
    if (hasChild)
        node.m_priority = bestPriority;
    return bestChild;
}

//----------------------------------------------------------------------------

void OpeningBookUtil::DumpVisualizationData(const OpeningBook& book, 
                                            StoneBoard& brd, 
                                            int depth,
                                            std::ostream& out)
{
    OpeningBookNode node;
    if (!book.GetNode(brd, node))
        return;
    if (node.IsLeaf())
    {
        out << node.Value(brd) << " " << depth << '\n';
        return;
    }
    for (BitsetIterator i(brd.getEmpty()); i; ++i) 
    {
	brd.playMove(brd.WhoseTurn(), *i);
        DumpVisualizationData(book, brd, depth + 1, out);
        brd.undoMove(*i);
    }
}

//----------------------------------------------------------------------------
