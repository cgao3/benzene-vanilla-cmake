//----------------------------------------------------------------------------
/** @file BoardUtil.cpp */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "SgRandom.h"

#include "BoardUtil.hpp"
#include "BitsetIterator.hpp"
#include "VCSet.hpp"
#include "GraphUtil.hpp"
#include "HexBoard.hpp"
#include "Pattern.hpp"
#include "HashedPatternSet.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

HexPoint BoardUtil::CoordsToPoint(const ConstBoard& brd, int x, int y)
{
    if (x <= -2 || x > brd.Width())      return INVALID_POINT;
    if (y <= -2 || y > brd.Height())     return INVALID_POINT;
    if ((x == -1 || x == brd.Width()) &&
	(y == -1 || y == brd.Height()))  return INVALID_POINT;

    if (y == -1)       return NORTH;
    if (y == brd.Height()) return SOUTH;
    if (x == -1)       return WEST;
    if (x == brd.Width())  return EAST;

    return HexPointUtil::coordsToPoint(x, y);
}

HexPoint BoardUtil::PointInDir(const ConstBoard& brd, 
                                HexPoint point, HexDirection dir)
{
    if (HexPointUtil::isEdge(point))
        return point;

    int x, y;
    BenzeneAssert(HexPointUtil::isInteriorCell(point));
    HexPointUtil::pointToCoords(point, x, y);
    x += HexPointUtil::DeltaX(dir);
    y += HexPointUtil::DeltaY(dir);
    return BoardUtil::CoordsToPoint(brd, x, y);
}

HexPoint BoardUtil::Rotate(const ConstBoard& brd, HexPoint p)
{
    BenzeneAssert(brd.IsValid(p));
    
    if (!brd.IsLocation(p)) return p;
    if (HexPointUtil::isEdge(p)) return HexPointUtil::oppositeEdge(p);
    
    int x, y;
    HexPointUtil::pointToCoords(p, x, y);
    return HexPointUtil::coordsToPoint(brd.Width()-1-x, brd.Height()-1-y);
}

HexPoint BoardUtil::Mirror(const ConstBoard& brd, HexPoint p)
{
    BenzeneAssert(brd.IsValid(p));
    BenzeneAssert(brd.Width() == brd.Height());
    
    if (!brd.IsLocation(p)) return p;
    
    if (HexPointUtil::isEdge(p)) {
	if (HexPointUtil::isColorEdge(p, VERTICAL_COLOR))
	    return HexPointUtil::rightEdge(p);
	else
	    return HexPointUtil::leftEdge(p);
    }
    
    int x, y;
    HexPointUtil::pointToCoords(p, x, y);
    return HexPointUtil::coordsToPoint(y, x);
}

HexPoint BoardUtil::CenterPoint(const ConstBoard& brd)
{
    BenzeneAssert((brd.Width() & 1) && (brd.Height() & 1));
    return CenterPointRight(brd);
}

HexPoint BoardUtil::CenterPointRight(const ConstBoard& brd)
{
    int x = brd.Width() / 2;
    int y = brd.Height() / 2;

    if (!(brd.Width() & 1) && !(brd.Height() & 1)) y--;

    return HexPointUtil::coordsToPoint(x, y);
}

HexPoint BoardUtil::CenterPointLeft(const ConstBoard& brd)
{
    int x = brd.Width() / 2;
    int y = brd.Height() / 2;

    if (!(brd.Width() & 1)) x--;
    if ((brd.Width() & 1) && !(brd.Height() & 1)) y--;

    return HexPointUtil::coordsToPoint(x, y);
}

HexPoint BoardUtil::RandomEmptyCell(const StoneBoard& brd)
{
    bitset_t moves = brd.GetEmpty() & brd.Const().GetCells();
    int count = static_cast<int>(moves.count());
    if (count == 0) 
        return INVALID_POINT;
    
    int randMove = SgRandom::Global().Int(count) + 1;
    for (BitsetIterator p(moves); p; ++p) 
        if (--randMove==0) return *p;

    BenzeneAssert(false);
    return INVALID_POINT;
}

//----------------------------------------------------------------------------

bitset_t BoardUtil::PackBitset(const ConstBoard& brd, 
                                const bitset_t& in)
{
    int j=0;
    bitset_t ret;
    for (BoardIterator it(brd.Interior()); it; ++it, ++j) {
        if (in.test(*it)) 
            ret.set(j);
    }
    return ret;
}

bitset_t BoardUtil::UnpackBitset(const ConstBoard& brd, 
                                  const bitset_t& in)
{
    int j=0;
    bitset_t ret;
    for (BoardIterator it(brd.Interior()); it; ++it, ++j) {
        if (in.test(j))
            ret.set(*it);
    }
    return ret;
}

bitset_t BoardUtil::Rotate(const ConstBoard& brd, 
                            const bitset_t& bs)
{
    bitset_t ret;
    for (BitsetIterator it(bs); it; ++it) {
        ret.set(Rotate(brd, *it));
    }
    return ret;
}

bitset_t BoardUtil::Mirror(const ConstBoard& brd, 
                            const bitset_t& bs)
{
    bitset_t ret;
    for (BitsetIterator it(bs); it; ++it) {
        ret.set(Mirror(brd, *it));
    }
    return ret;
}

bool BoardUtil::ShiftBitset(const ConstBoard& brd, const bitset_t& bs, 
                             HexDirection dir, bitset_t& out)
{
    out.reset();
    bool still_inside = true;
    for (BitsetIterator p(bs); p; ++p) {
        HexPoint s = PointInDir(brd, *p, dir);
        if (!HexPointUtil::isEdge(*p) && HexPointUtil::isEdge(s))
            still_inside = false;
        out.set(s);
    }
    return still_inside;
}

bool BoardUtil::ConnectedOnBitset(const ConstBoard& brd, 
                                   const bitset_t& carrier,
                                   HexPoint p1, HexPoint p2)
{
    BenzeneAssert(carrier.test(p1));
    BenzeneAssert(carrier.test(p2));
    bitset_t seen = ReachableOnBitset(brd, carrier, EMPTY_BITSET, p1);
    return seen.test(p2);
}

bitset_t BoardUtil::ReachableOnBitset(const ConstBoard& brd, 
                                       const bitset_t& carrier,
                                       const bitset_t& stopset,
                                       HexPoint start)
{
    BenzeneAssert(carrier.test(start));
    bitset_t seen;
    std::queue<HexPoint> q;
    q.push(start);
    seen.set(start);
    while (!q.empty()) 
    {
        HexPoint p = q.front();
        q.pop();
        if (stopset.test(p)) 
            continue;
        for (BoardIterator nb(brd.Nbs(p)); nb; ++nb) 
        {
            if (carrier.test(*nb) && !seen.test(*nb)) 
            {
                q.push(*nb);
                seen.set(*nb);
            }
        }
    }
    return seen;
}

//----------------------------------------------------------------------------

std::string BoardUtil::GuiDumpOutsideConsiderSet(const StoneBoard& brd, 
                                                  const bitset_t& consider,
                                                  const bitset_t& remove)
{
    std::ostringstream os;
    bitset_t outside = brd.GetEmpty() - (remove | consider);
    for (BitsetIterator p(outside); p; ++p) 
        os << " " << *p << " x";
    return os.str();
}

//----------------------------------------------------------------------------
