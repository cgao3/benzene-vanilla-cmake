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
        float value = search.InverseEval(child.Mean());
        // Scale to [-1,+1], black positive
        double influence = value * 2 - 1;
        if (toPlay == SG_WHITE)
            influence *= -1;
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
    return HexPointUtil::toString(move);
}

SgBlackWhite HexUctUtil::ToSgBlackWhite(HexColor c)
{
    if (c == BLACK)
	return SG_BLACK;
    HexAssert(c == WHITE);
    return SG_WHITE;
}

//----------------------------------------------------------------------------

