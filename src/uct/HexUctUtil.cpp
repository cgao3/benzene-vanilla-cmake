//----------------------------------------------------------------------------
/** @file HexUctUtil.cpp
*/
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "Hex.hpp"
#include "HexUctUtil.hpp"

#include <iomanip>
#include <iostream>
#include "SgBWSet.h"
#include "SgPointSet.h"
#include "SgProp.h"
#include "SgUctSearch.h"

using namespace benzene;

//----------------------------------------------------------------------------

void GoGuiGfxStatus(const SgUctSearch& search, std::ostream& out)
{
    const SgUctTree& tree = search.Tree();
    const SgUctNode& root = tree.Root();
    const SgUctSearchStat& stat = search.Statistics();
    int abortPercent = static_cast<int>(stat.m_aborted.Mean() * 100);
    out << "TEXT N=" << static_cast<size_t>(root.MoveCount())
        << " V=" << std::setprecision(2) << root.Mean()
        << " Len=" << static_cast<int>(stat.m_gameLength.Mean())
        << " Tree=" << std::setprecision(1) << stat.m_movesInTree.Mean()
        << "/" << static_cast<int>(stat.m_movesInTree.Max())
        << " Abrt=" << abortPercent << '%'
        << " Gm/s=" << static_cast<int>(stat.m_gamesPerSecond) << '\n';
}

void HexUctUtil::GoGuiGfx(const SgUctSearch& search, SgBlackWhite toPlay,
                          std::ostream& out)
{
    const SgUctTree& tree = search.Tree();
    const SgUctNode& root = tree.Root();
    out << "VAR";
    std::vector<const SgUctNode*> bestValueChild;
    bestValueChild.push_back(search.FindBestChild(root));
    for (int i=0; i<4; i++) {
	if (bestValueChild[i] == 0) break;
	SgPoint move = bestValueChild[i]->Move();
	if (0 == (i % 2))
	    out << ' ' << (toPlay == SG_BLACK ? 'B' : 'W') << ' '
		<< HexUctUtil::MoveString(move);
	else
	    out << ' ' << (toPlay == SG_WHITE ? 'B' : 'W') << ' '
		<< HexUctUtil::MoveString(move);
	bestValueChild.push_back(search.FindBestChild(*(bestValueChild[i])));
    }
    out << "\n";
    out << "INFLUENCE";
    for (SgUctChildIterator it(tree, root); it; ++it)
    {
        const SgUctNode& child = *it;
        if (child.MoveCount() == 0)
            continue;
        float influence = search.InverseEval(child.Mean());
        SgPoint move = child.Move();
        out << ' ' << HexUctUtil::MoveString(move) << ' ' 
            << std::fixed << std::setprecision(2) << influence;
    }
    out << '\n'
        << "LABEL";
    int numChildren = 0, numZeroExploration = 0, numSmallExploration = 0;
    for (SgUctChildIterator it(tree, root); it; ++it)
    {
        const SgUctNode& child = *it;
        size_t count = static_cast<size_t>(child.MoveCount());
	numChildren++;
	if (count < 10)
	    numSmallExploration++;
        if (count == 0)
	    numZeroExploration++;
	out << ' ' << HexUctUtil::MoveString(child.Move())
	    << ' ' << count;
    }
    out << '\n';
    GoGuiGfxStatus(search, out);

    out << numSmallExploration << " root children minimally explored with "
	<< numZeroExploration << " zeroes of " << numChildren << " total."
        << '\n';
}

int HexUctUtil::ComputeMaxNumMoves()
{
    return FIRST_INVALID;
}

std::string HexUctUtil::MoveString(SgMove sgmove)
{
    HexPoint move = static_cast<HexPoint>(sgmove);
    
    // Simple process if just HexPoint
    HexAssert(0 <= move && move < FIRST_INVALID);
    return HexPointUtil::ToString(move);
}

SgBlackWhite HexUctUtil::ToSgBlackWhite(HexColor c)
{
    if (c == BLACK)
	return SG_BLACK;
    HexAssert(c == WHITE);
    return SG_WHITE;
}

//----------------------------------------------------------------------------

namespace 
{

void SaveNode(std::ostream& out, const SgUctTree& tree, const SgUctNode& node, 
              HexColor toPlay, int maxDepth, int depth)
{
    out << "C[MoveCount " << node.MoveCount()
        << "\nPosCount " << node.PosCount()
        << "\nMean " << std::fixed << std::setprecision(2) << node.Mean();
    if (!node.HasChildren())
    {
        out << "]\n";
        return;
    }
    out << "\n\nRave:";
    for (SgUctChildIterator it(tree, node); it; ++it)
    {
        const SgUctNode& child = *it;
        HexPoint move = static_cast<HexPoint>(child.Move());
        if (child.HasRaveValue())
        {
            out << '\n' << HexUctUtil::MoveString(move) << ' '
                << std::fixed << std::setprecision(2) << child.RaveValue()
                << " (" << child.RaveCount() << ')';
        }
    }
    out << "]\nLB";
    for (SgUctChildIterator it(tree, node); it; ++it)
    {
        const SgUctNode& child = *it;
        if (! child.HasMean())
            continue;
        HexPoint move = static_cast<HexPoint>(child.Move());
        out << "[" << HexUctUtil::MoveString(move) << ':' 
            << child.MoveCount() << '@' << child.Mean() << ']';
    }
    out << '\n';
    if (maxDepth >= 0 && depth >= maxDepth)
        return;
    for (SgUctChildIterator it(tree, node); it; ++it)
    {
        const SgUctNode& child = *it;
        if (! child.HasMean())
            continue;
        HexPoint move = static_cast<HexPoint>(child.Move());
        out << "(;" << (toPlay == BLACK ? 'B' : 'W') 
            << '[' << HexUctUtil::MoveString(move) << ']';
        SaveNode(out, tree, child, !toPlay, maxDepth, depth + 1);
        out << ")\n";
    }
}

}

void HexUctUtil::SaveTree(const SgUctTree& tree, const StoneBoard& brd, 
                     HexColor toPlay, std::ostream& out, int maxDepth)
{
    out << "(;FF[4]GM[11]SZ[" << brd.Width() << "]\n";
    out << ";AB";
    for (BoardIterator it(brd.Stones(BLACK)); it; ++it)
        out << '[' << *it << ']';
    out << '\n';
    out << "AW";
    for (BoardIterator it(brd.Stones(WHITE)); it; ++it)
        out << '[' << *it << ']';
    out << '\n';
    out << "AE";
    for (BoardIterator it(brd.Stones(EMPTY)); it; ++it)
        out << '[' << *it << ']';
    out << '\n';
    out << "PL[" << (toPlay == SG_BLACK ? "B" : "W") << "]\n";
    SaveNode(out, tree, tree.Root(), toPlay, maxDepth, 0);
    out << ")\n";
}

//----------------------------------------------------------------------------


