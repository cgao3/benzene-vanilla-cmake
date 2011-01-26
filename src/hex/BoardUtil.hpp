//----------------------------------------------------------------------------
/** @file BoardUtil.hpp */
//----------------------------------------------------------------------------

#ifndef BOARDUTIL_HPP
#define BOARDUTIL_HPP

#include "Hex.hpp"
#include "HexBoard.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Utilities on Boards. */
namespace BoardUtil
{
    /** @name Cells */
    // @{

    /** Returns HexPoint at the coordinates (x, y). Note that edges
        have a single coordinate equal to -1 or width()/height().
        That is, (-1, ?) is WEST, (?, -1) is NORTH, etc. Returns
        INVALID_POINT if (x, y) do not correspond to a valid point. */
    HexPoint CoordsToPoint(const ConstBoard& brd, int x, int y);

    /** Returns HexPoint in direction dir from the point p. 
        If p is an edge, returns p. */
    HexPoint PointInDir(const ConstBoard& brd, HexPoint p, 
                        HexDirection dir);

    /** Rotates the given point 180' about the center of the board. */
    HexPoint Rotate(const ConstBoard& brd, HexPoint p);

    /** Mirrors the given point in the diagonal joining acute corners.
	Note that this function requires square boards! */
    HexPoint Mirror(const ConstBoard& brd, HexPoint p);

    /** Returns the center point on boards where both dimensions are
	odd (fails on all other boards). */
    HexPoint CenterPoint(const ConstBoard& brd);

    /** These two methods return the two points nearest the center of
	the board. On boards with one or more even dimensions, out of
	the center-most points the "top right" and "bottom left"
	points are returned as they relate to the main
	diagonals/visually. On boards with two odd dimensions, both of
	these methods return the same point as centerPoint(). */
    HexPoint CenterPointRight(const ConstBoard& brd);

    /** See centerPointRight(). */
    HexPoint CenterPointLeft(const ConstBoard& brd);

    /** Returns a random empty cell or INVALID_POINT if the board is
        full. */
    HexPoint RandomEmptyCell(const StoneBoard& brd);

    // @}

    //-------------------------------------------------------------------------

    /** @name Bitsets */
    // @{

    /** Packs a bitset on this boardsize. Output bitset has a bit for
        each cell on the board, starting at a1. That is, an 8x8 board
        can fit into exactly 64 bits. If in has a bit set at a1, then
        the packed bitset will have its 0th bit set; if in has a bit
        at a2, the second bit is set, etc, etc. */
    bitset_t PackBitset(const ConstBoard& brd, const bitset_t& in);

    /** Unpacks a bitset to the canonical representation. 
        See PackBitset() */
    bitset_t UnpackBitset(const ConstBoard& brd, const bitset_t& in);

    /** Shifts bitset in direction dir using PointInDir(). Returns
        true if each interior point is still an interior point. */
    bool ShiftBitset(const ConstBoard& brd, const bitset_t& bs, 
                     HexDirection dir, bitset_t& out);

    /** Rotates the given bitset 180' about the center of the
        board. */
    bitset_t Rotate(const ConstBoard& brd, const bitset_t& bs);

    /** Mirrors the given bitset in the acute diagonal (requires that
	width equals height). */
    bitset_t Mirror(const ConstBoard& brd, const bitset_t& bs);

    /** Returns true if p1 is connected to p2 on the bitset; p1 and p2
        are assumed to be inside the bitset.  */
    bool ConnectedOnBitset(const ConstBoard& brd, const bitset_t& carrier,
                           HexPoint p1, HexPoint p2);

    /** Returns a subset of carrier: the points that are reachable
        from start. Flow will enter and leave cells in carrier, and
        enter but not leave cells in stopset. */
    bitset_t ReachableOnBitset(const ConstBoard& brd, const bitset_t& carrier,
                               const bitset_t& stopset, HexPoint start);
    // @}

    //----------------------------------------------------------------------

    /** Dumps all cells outside the consider set and the remove set
        in a format the gui expects. */
    std::string GuiDumpOutsideConsiderSet(const StoneBoard& brd, 
                                          const bitset_t& consider,
                                          const bitset_t& remove);
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // BOARDUTIL_HPP
