//----------------------------------------------------------------------------
/** @file Book.cpp */
//----------------------------------------------------------------------------

#include <cmath>
#include <boost/numeric/conversion/bounds.hpp>

#include "BitsetIterator.hpp"
#include "Book.hpp"
#include "HexModState.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

/** Dump debug info. */
#define OUTPUT_OB_INFO 1

//----------------------------------------------------------------------------

/** Current version for book databases. 
    Update this if BookNode changes to prevent old out-of-date books
    being loaded. */
const std::string Book::BOOK_DB_VERSION = "BENZENE_BOOK_VER_0001";

//----------------------------------------------------------------------------

float BookUtil::Value(const SgBookNode& node, const HexState& state)
{
    if (state.Position().IsLegal(SWAP_PIECES))
        return std::max(node.m_value, BookUtil::InverseEval(node.m_value));
    return node.m_value;
}

float BookUtil::Score(const SgBookNode& node, const HexState& state, 
                      float countWeight)
{
    float score = BookUtil::InverseEval(Value(node, state));
    if (!node.IsTerminal())
        score += log(node.m_count + 1) * countWeight;
    return score;	
}

float BookUtil::InverseEval(float eval)
{
    if (HexEvalUtil::IsWinOrLoss(eval))
        return -eval;
    if (eval < 0 || eval > 1)
        LogInfo() << "eval = " << eval << '\n';
    BenzeneAssert(0 <= eval && eval <= 1.0);
    return 1.0 - eval;
}

//----------------------------------------------------------------------------

int BookUtil::GetMainLineDepth(const Book& book, const HexState& origState)
{
    int depth = 0;
    HexState state(origState);
    for (;;) 
    {
        HexBookNode node;
        if (!book.Get(state, node))
            break;
        HexPoint move = INVALID_POINT;
        float value = -1e9;
        for (BitsetIterator p(state.Position().GetEmpty()); p; ++p)
        {
            state.PlayMove(*p);
            HexBookNode child;
            if (book.Get(state, child))
            {
                float curValue = InverseEval(BookUtil::Value(child, state));
                if (curValue > value)
                {
                    value = curValue;
                    move = *p;
                }
            }
            state.UndoMove(*p);
        }
        if (move == INVALID_POINT)
            break;
        state.PlayMove(move);
        depth++;
    }
    return depth;
}

namespace 
{

std::size_t TreeSize(const Book& book, HexState& state, 
                     StateMap<std::size_t>& solved)
{
    if (solved.Exists(state))
        return solved[state];
    HexBookNode node;
    if (!book.Get(state, node))
        return 0;
    std::size_t ret = 1;
    for (BitsetIterator p(state.Position().GetEmpty()); p; ++p) 
    {
        state.PlayMove(*p);
        ret += TreeSize(book, state, solved);
        state.UndoMove(*p);
    }
    solved[state] = ret;
    return ret;
}

}

std::size_t BookUtil::GetTreeSize(const Book& book, const HexState& origState)
{
    StateMap<std::size_t> solved;
    HexState state(origState);
    return TreeSize(book, state, solved);
}

//----------------------------------------------------------------------------

HexPoint BookUtil::BestMove(const Book& book, const HexState& origState,
                            unsigned minCount, float countWeight)
{
    HexBookNode node;
    if (!book.Get(origState, node) || node.m_count < minCount)
        return INVALID_POINT;

    float bestScore = -1e9;
    HexPoint bestChild = INVALID_POINT;
    HexState state(origState);
    for (BitsetIterator p(state.Position().GetEmpty()); p; ++p)
    {
        state.PlayMove(*p);
        HexBookNode child;
        if (book.Get(state, child))
        {
            float score = BookUtil::Score(child, state, countWeight);
            if (score > bestScore)
            {
                bestScore = score;
                bestChild = *p;
            }
        }
        state.UndoMove(*p);
    }
    BenzeneAssert(bestChild != INVALID_POINT);
    return bestChild;
}

//----------------------------------------------------------------------------

void BookUtil::DumpVisualizationData(const Book& book,
                                     const HexState& origState, int depth, 
                                     std::ostream& out)
{
    HexBookNode node;
    if (!book.Get(origState, node))
        return;
    if (node.IsLeaf())
    {
        out << BookUtil::Value(node, origState) << " " << depth << '\n';
        return;
    }
    HexModState modState(origState);
    HexState state = modState.State();
    for (BitsetIterator i(state.Position().GetEmpty()); i; ++i) 
    {
	state.PlayMove( *i);
        DumpVisualizationData(book, state, depth + 1, out);
        state.UndoMove(*i);
    }
}

namespace {

void DumpPolarizedLeafs(const Book& book, HexState& state,
                        float polarization, StateSet& seen,
                        PointSequence& pv, std::ostream& out,
                        const StateSet& ignoreSet)
{
    if (seen.Exists(state))
        return;
    HexBookNode node;
    if (!book.Get(state, node))
        return;
    if (fabs(BookUtil::Value(node, state) - 0.5) >= polarization 
        && node.IsLeaf() && !node.IsTerminal()
        && ignoreSet.Exists(state))
    {
        out << HexPointUtil::ToString(pv) << '\n';
        seen.Insert(state);
    }
    else
    {
        if (node.IsLeaf() || node.IsTerminal())
            return;
        for (BitsetIterator i(state.Position().GetEmpty()); i; ++i) 
        {
            state.PlayMove(*i);
            pv.push_back(*i);
            DumpPolarizedLeafs(book, state, polarization, seen, pv, out, 
                               ignoreSet);
            pv.pop_back();
            state.UndoMove(*i);
        }
        seen.Insert(state);
    } 
}

}

void BookUtil::DumpPolarizedLeafs(const Book& book, const HexState& origState,
                                  float polarization, PointSequence& pv, 
                                  std::ostream& out, 
                                  const StateSet& ignoreSet)
{
    StateSet seen;
    HexModState modState(origState);
    HexState state = modState.State();
    ::DumpPolarizedLeafs(book, state, polarization, seen, pv, out, ignoreSet);
}

void BookUtil::ImportSolvedStates(Book& book, const ConstBoard& constBoard,
                                  std::istream& positions)
{
    StoneBoard brd(constBoard.Width(), constBoard.Height());
    HexState state(brd, FIRST_TO_PLAY);
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
        BenzeneAssert(winner != EMPTY);
        
        ++numParsed;
        state.Position().StartNewGame();
        state.SetToPlay(FIRST_TO_PLAY);
        for (std::size_t i = 0; i < points.size(); ++i)
            state.PlayMove(points[i]);
        HexEval ourValue = (state.ToPlay() == winner) 
            ? IMMEDIATE_WIN : IMMEDIATE_LOSS;
        HexBookNode node;
        if (book.Get(state, node))
        {
            BenzeneAssert(node.IsLeaf());
            BenzeneAssert(!node.IsTerminal());
            node.m_value = ourValue;
            ++numReplaced;
        }
        else 
        {
            node = HexBookNode(ourValue);
            ++numNew;
        }
        book.Put(state, node);
    }
    book.Flush();
    LogInfo() << "   Lines: " << lineNumber << '\n';
    LogInfo() << "  Parsed: " << numParsed << '\n';
    LogInfo() << "Replaced: " << numReplaced << '\n';
    LogInfo() << "     New: " << numNew << '\n';
}

//----------------------------------------------------------------------------
