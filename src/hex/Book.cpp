//----------------------------------------------------------------------------
/** @file Book.cpp
*/
//----------------------------------------------------------------------------

#include <cmath>
#include <boost/numeric/conversion/bounds.hpp>

#include "BitsetIterator.hpp"
#include "HexException.hpp"
#include "Book.hpp"
#include "Time.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

/** Dump debug info. */
#define OUTPUT_OB_INFO 1

//----------------------------------------------------------------------------

float BookNode::Value(const StoneBoard& brd) const
{
    if (brd.isLegal(SWAP_PIECES))
        return std::max(m_value, Book::InverseEval(m_value));
    return m_value;
}

float BookNode::Score(const StoneBoard& brd, float countWeight) const
{
    float score = Book::InverseEval(Value(brd));
    if (!IsTerminal())
        score += log(m_count + 1) * countWeight;
    return score;	
}

bool BookNode::IsTerminal() const
{
    if (HexEvalUtil::IsWinOrLoss(m_value))
        return true;
    return false;
}

bool BookNode::IsLeaf() const
{
    return m_count == 0;
}

std::string BookNode::toString() const
{
    std::ostringstream os;
    os << std::showpos << std::fixed << std::setprecision(3);
    os << "Prop=" << m_value;
    os << std::noshowpos << ", ExpP=" << m_priority;
    os << std::showpos << ", Heur=" << m_heurValue << ", Cnt=" << m_count;
    return os.str();
}

//----------------------------------------------------------------------------

Book::Book(std::string filename)
    throw(HexException)
{
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

Book::~Book()
{
}

//----------------------------------------------------------------------------

float Book::InverseEval(float eval)
{
    if (HexEvalUtil::IsWinOrLoss(eval))
        return -eval;
    if (eval < 0 || eval > 1)
        LogInfo() << "eval = " << eval << '\n';
    HexAssert(0 <= eval && eval <= 1.0);
    return 1.0 - eval;
}

//----------------------------------------------------------------------------

bool Book::GetNode(const StoneBoard& brd, BookNode& node) const
{
    if (m_db.Get(BookUtil::GetHash(brd), node))
        return true;
    return false;
}

void Book::WriteNode(const StoneBoard& brd, const BookNode& node)
{
    m_db.Put(BookUtil::GetHash(brd), node);
}

int Book::GetMainLineDepth(const StoneBoard& pos) const
{
    int depth = 0;
    StoneBoard brd(pos);
    for (;;) 
    {
        BookNode node;
        if (!GetNode(brd, node))
            break;
        HexPoint move = INVALID_POINT;
        float value = -1e9;
        for (BitsetIterator p(brd.getEmpty()); p; ++p)
        {
            brd.playMove(brd.WhoseTurn(), *p);
            BookNode child;
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

std::size_t Book::GetTreeSize(const StoneBoard& board) const
{
    std::map<hash_t, std::size_t> solved;
    StoneBoard brd(board);
    return TreeSize(brd, solved);
}

std::size_t Book::TreeSize(StoneBoard& brd, std::map<hash_t, 
                           std::size_t>& solved) const
{
    hash_t hash = BookUtil::GetHash(brd);
    if (solved.find(hash) != solved.end())
        return solved[hash];

    BookNode node;
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

hash_t BookUtil::GetHash(const StoneBoard& brd)
{
    hash_t hash1 = brd.Hash();
    StoneBoard rotatedBrd(brd);
    rotatedBrd.rotateBoard();
    hash_t hash2 = rotatedBrd.Hash();
    return std::min(hash1, hash2);
}

unsigned BookUtil::NumChildren(const Book& book, const StoneBoard& board)
{
    unsigned num = 0;
    StoneBoard brd(board);
    for (BitsetIterator i(brd.getEmpty()); i; ++i) 
    {
	brd.playMove(brd.WhoseTurn(), *i);
	BookNode child;
        if (book.GetNode(brd, child))
            ++num;
        brd.undoMove(*i);
    }
    return num;
}

void BookUtil::UpdateValue(const Book& book, BookNode& node, StoneBoard& brd)
{
    bool hasChild = false;
    float bestValue = boost::numeric::bounds<float>::lowest();
    for (BitsetIterator i(brd.getEmpty()); i; ++i) 
    {
	brd.playMove(brd.WhoseTurn(), *i);
	BookNode child;
        if (book.GetNode(brd, child))
        {
            hasChild = true;
            float value = Book::InverseEval(child.Value(brd));
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
float BookUtil::ComputePriority(const StoneBoard& brd, 
                                const BookNode& parent,
                                const BookNode& child,
                                double alpha)
{
    // Must adjust child value for swap, but not the parent because we
    // are comparing with the best child's value, ie, the minmax
    // value.
    float delta = parent.m_value - Book::InverseEval(child.Value(brd));
    HexAssert(delta >= 0.0);
    HexAssert(child.m_priority >= BookNode::LEAF_PRIORITY);
    HexAssert(child.m_priority < BookNode::DUMMY_PRIORITY);
    return alpha * delta + child.m_priority + 1;
}

HexPoint BookUtil::UpdatePriority(const Book& book, BookNode& node, 
                                  StoneBoard& brd, float alpha)
{
    bool hasChild = false;
    float bestPriority = boost::numeric::bounds<float>::highest();
    HexPoint bestChild = INVALID_POINT;
    for (BitsetIterator i(brd.getEmpty()); i; ++i) 
    {
	brd.playMove(brd.WhoseTurn(), *i);
	BookNode child;
        if (book.GetNode(brd, child))
        {
            hasChild = true;
            float priority 
                = BookUtil::ComputePriority(brd, node, child, alpha);
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

HexPoint BookUtil::BestMove(const Book& book, const StoneBoard& pos,
                            unsigned minCount, float countWeight)
{
    BookNode node;
    if (!book.GetNode(pos, node) || node.m_count < minCount)
        return INVALID_POINT;

    float bestScore = -1e9;
    HexPoint bestChild = INVALID_POINT;
    StoneBoard brd(pos);
    for (BitsetIterator p(brd.getEmpty()); p; ++p)
    {
        brd.playMove(brd.WhoseTurn(), *p);
        BookNode child;
        if (book.GetNode(brd, child))
        {
            float score = child.Score(brd, countWeight);
            if (score > bestScore)
            {
                bestScore = score;
                bestChild = *p;
            }
        }
        brd.undoMove(*p);
    }
    HexAssert(bestChild != INVALID_POINT);
    return bestChild;
}

//----------------------------------------------------------------------------

void BookUtil::DumpVisualizationData(const Book& book, StoneBoard& brd, 
                                     int depth, std::ostream& out)
{
    BookNode node;
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

namespace {

void DumpPolarizedLeafs(const Book& book, StoneBoard& brd,
                        float polarization, std::set<hash_t>& seen,
                        PointSequence& pv, std::ostream& out,
                        const std::set<hash_t>& ignoreSet)
{
    hash_t hash = BookUtil::GetHash(brd);
    if (seen.find(hash) != seen.end())
        return;
    BookNode node;
    if (!book.GetNode(brd, node))
        return;
    if (fabs(node.Value(brd) - 0.5) >= polarization 
        && node.IsLeaf() && !node.IsTerminal()
        && !ignoreSet.count(hash))
    {
        out << HexPointUtil::ToString(pv) << '\n';
        seen.insert(hash);
    }
    else
    {
        if (node.IsLeaf() || node.IsTerminal())
            return;
        for (BitsetIterator i(brd.getEmpty()); i; ++i) 
        {
            brd.playMove(brd.WhoseTurn(), *i);
            pv.push_back(*i);
            DumpPolarizedLeafs(book, brd, polarization, seen, pv, out, ignoreSet);
            pv.pop_back();
            brd.undoMove(*i);
        }
        seen.insert(hash);
    } 
}

}

void BookUtil::DumpPolarizedLeafs(const Book& book, StoneBoard& brd,
                                  float polarization, PointSequence& pv, 
                                  std::ostream& out, 
                                  const std::set<hash_t>& ignoreSet)
{
    std::set<hash_t> seen;
    ::DumpPolarizedLeafs(book, brd, polarization, seen, pv, out, ignoreSet);
}

void BookUtil::ImportSolvedStates(Book& book, const ConstBoard& constBoard,
                                  std::istream& positions)
{
    StoneBoard brd(constBoard.Width(), constBoard.Height());
    std::string text;
    std::size_t lineNumber = 0;
    std::size_t numParsed = 0;
    std::size_t numReplaced = 0;
    std::size_t numNew = 0;
    while (std::getline(positions, text))
    {
        ++lineNumber;
        std::istringstream is(text);
        PointSequence points;
        HexColor winner = EMPTY;
        bool parsed = false;
        while (true)
        {
            std::string token;
            is >> token;
            if (token == "black")
            {
                winner = BLACK;
                parsed = true;
                break;
            } 
            else if (token == "white")
            {
                winner = WHITE;
                parsed = true;
                break;
            }
            else
            {
                HexPoint p = HexPointUtil::FromString(token);
                if (p == INVALID_POINT)
                    break;
                points.push_back(p);
            }
        }
        if (!parsed)
        {
            LogInfo() << "Skipping badly formed line " << lineNumber << ".\n";
            continue;
        }
        HexAssert(winner != EMPTY);
        
        ++numParsed;
        brd.startNewGame();
        for (std::size_t i = 0; i < points.size(); ++i)
            brd.playMove(brd.WhoseTurn(), points[i]);
        HexEval ourValue = (brd.WhoseTurn() == winner) 
            ? IMMEDIATE_WIN : IMMEDIATE_LOSS;
        BookNode node;
        if (book.GetNode(brd, node))
        {
            HexAssert(node.IsLeaf());
            HexAssert(!node.IsTerminal());
            node.m_value = ourValue;
            ++numReplaced;
        }
        else 
        {
            node = BookNode(ourValue);
            ++numNew;
        }
        book.WriteNode(brd, node);
    }
    book.Flush();
    LogInfo() << "   Lines: " << lineNumber << '\n';
    LogInfo() << "  Parsed: " << numParsed << '\n';
    LogInfo() << "Replaced: " << numReplaced << '\n';
    LogInfo() << "     New: " << numNew << '\n';
}

//----------------------------------------------------------------------------
