//----------------------------------------------------------------------------
/** @file HexPoint.hpp */
//----------------------------------------------------------------------------

#ifndef HEXPOINT_HPP
#define HEXPOINT_HPP

#include <map>
#include <string>
#include <vector>
#include <utility>

#include "Benzene.hpp"
#include "BenzeneAssert.hpp"
#include "Bitset.hpp"
#include "HexColor.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** @page hexpoints HexPoints 

    There are three types of HexPoints: special, edges, and interior.
    Special points encode special moves that do not correspond to a
    physical location on a hex board. These are: INVALID_POINT,
    RESIGN, and SWAP_PIECES.  Edge points (NORTH, SOUTH, EAST, WEST)
    denote the edges of the board.  Internal points are the interior
    points of the board, the number of which is controlled by the
    constants MAX_WIDTH and MAX_HEIGHT.

    HexPoints are laid out in memory as follows:

    @verbatim

      0   INVALID_POINT
      1   RESIGN          
      2   SWAP_PIECES     
      3   NORTH
      4   EAST
      5   SOUTH
      6   WEST
      7   1st interior point
      8   2nd interior point
      ...
      ...
      ... FIRST_INVALID
    @endverbatim

    It is assumed that the special points (i.e. SWAP_PIECES and
    RESIGN) come immediately before the edge points (i.e. NORTH to
    WEST) which come immediately before the interior points.

    The interior points are laid out as follows.  The first MAX_WIDTH
    interior points get the name 'a1, b1, c1, ... , L1', where L is
    letter MAX_WIDTH-1 letters after 'a'. The next MAX_WIDTH points
    get a '2' suffix, then a '3' suffix, and so on, until MAX_HEIGHT
    is reached.

    This encoding allows an 11x11 hex board to fit into 128 bits if
    MAX_WIDTH = 11 and MAX_HEIGHT = 11.
*/

//----------------------------------------------------------------------------

/** @name Maximum dimensions.

    If you are going to change either of these constants, make sure to
    synchronize the printed names in HexPointData with the enumerated
    interior cell constants below.
*/
// @{ 

#if defined(SUPPORT_19x19)

/** The maximum width of a valid ConstBoard. */
static const int MAX_WIDTH  = 19;

/** The maximum height of a valid ConstBoard. */
static const int MAX_HEIGHT = 19;

#elif defined(SUPPORT_14x14)

/** The maximum width of a valid ConstBoard. */
static const int MAX_WIDTH  = 14;

/** The maximum height of a valid ConstBoard. */
static const int MAX_HEIGHT = 14;

#elif defined(SUPPORT_13x13)

/** The maximum width of a valid ConstBoard. */
static const int MAX_WIDTH  = 13;

/** The maximum height of a valid ConstBoard. */
static const int MAX_HEIGHT = 13;

#else

/** The maximum width of a valid ConstBoard. */
static const int MAX_WIDTH  = 11;

/** The maximum height of a valid ConstBoard. */
static const int MAX_HEIGHT = 11;

#endif

// @}

//----------------------------------------------------------------------------

#if defined(SUPPORT_19x19)

#include "HexPoints19x19.hpp"

#elif defined(SUPPORT_14x14)

#include "HexPoints14x14.hpp"

#elif defined(SUPPORT_13x13)

#include "HexPoints13x13.hpp"

#else

#include "HexPoints11x11.hpp"

#endif

/** The value of the first special HexPoint. */
static const HexPoint FIRST_SPECIAL = RESIGN;

/** The value of the first edge HexPoint. */
static const HexPoint FIRST_EDGE = NORTH;

/** The value of the first interior cell; this should always be A1. */
static const HexPoint FIRST_CELL = HEX_CELL_A1;
    
//---------------------------------------------------------------------------

/** A map of points to points. */
typedef std::map<HexPoint, HexPoint> PointToPoint;

/** Pair of HexPoints. */
typedef std::pair<HexPoint, HexPoint> HexPointPair;

/** Set of HexPoints. */
typedef std::set<HexPoint> HexPointSet;

/** Map of HexPoints to bitsets. */
typedef std::map<HexPoint, bitset_t> PointToBitset;

/** A sequence of HexPoints. */
typedef std::vector<HexPoint> PointSequence;

//---------------------------------------------------------------------------

/**
   Delta arrays.
 
   On a hex board, we can travel only in the following six directions:
   EAST, NORTH_EAST, NORTH, WEST, SOUTH_WEST, SOUTH.
 
   @verbatim 
             | /  
             |/
         --- o ---
            /|
           / |
   @endverbatim
*/
enum HexDirection 
{ 
    DIR_EAST=0, 
    DIR_NORTH_EAST, 
    DIR_NORTH, 
    DIR_WEST, 
    DIR_SOUTH_WEST, 
    DIR_SOUTH, 
    NUM_DIRECTIONS 
};

//----------------------------------------------------------------------------

/** Utilities on HexPoints: converting to/from strings, testing for edges, 
    converting to/from x/y coordinates, etc. */
namespace HexPointUtil
{
    /** Converts a HexPoint to a string. */
    std::string ToString(HexPoint p);

    /** Converts a pair of HexPoints to a string. */
    std::string ToString(const HexPointPair& p);

    /** Returns a space separated output of points in lst. */
    std::string ToString(const PointSequence& lst);

    /** Returns a string representation of the bitset's set bits. */
    std::string ToString(const bitset_t& b);

    /** Returns the HexPoint with the given name; INVALID_POINT otherwise. */
    HexPoint FromString(const std::string& name);

    /** Reads a PointSequence from a string of space separated
        points. */
    void FromString(const std::string& str, PointSequence& pts);

    /** Returns true if this point is a swap move. */
    bool isSwap(HexPoint c);

    /** Returns true if this point is an edge point. */
    bool isEdge(HexPoint c);

    /** Returns true if this point is an interior cell. */
    bool isInteriorCell(HexPoint c);

    /** Returns the edge opposite the given edge. */
    HexPoint oppositeEdge(HexPoint edge);

    /** Returns the edge to the left of the given edge. */
    HexPoint leftEdge(HexPoint edge);

    /** Returns the edge to the right of the given edge. */
    HexPoint rightEdge(HexPoint edge);

    /** Returns a color's first edge.  NORTH for VERTICAL_COLOR
        and EAST for !VERTICAL_COLOR. */ 
    HexPoint colorEdge1(HexColor color);

    /** Returns a color's second edge.  SOUTH for VERTICAL_COLOR
        and WEST for !VERTICAL_COLOR. */
    HexPoint colorEdge2(HexColor color);

    /** Returns true if cell is one of color's edges. */
    bool isColorEdge(HexPoint cell, HexColor color);

    /** Converts cell into its x and y components, where
        @code
        x = (cell-FIRST_CELL) % MAX_WIDTH;
        y = (cell-FIRST_CELL) / MAX_WIDTH;
        @endcode
        
        Does not handle cases where cell is an edge.
        ConstBoard has a method for that.
    */
    void pointToCoords(HexPoint cell, int& x, int& y);

    /** Returns the HexPoint corresponding to the given x and y coords. 
        @code 
        point =  FIRST_CELL + (y*MAX_WIDTH) + x;
        @endcode
    */
    HexPoint coordsToPoint(int x, int y);

    /** Returns the change in the x-coordinate when travelling in the
        given direction. */
    int DeltaX(int dir);

    /** Returns the change in the y-coordinate when travelling in the
        given direction. */
    int DeltaY(int dir);

} // namespace

//----------------------------------------------------------------------------

inline bool HexPointUtil::isSwap(HexPoint c)
{
    return (c==SWAP_PIECES);
}

inline bool HexPointUtil::isEdge(HexPoint c)
{ 
    return (c==NORTH || c==SOUTH || c==EAST || c==WEST); 
}

inline bool HexPointUtil::isInteriorCell(HexPoint c)
{
    return (FIRST_CELL <= c && c < FIRST_INVALID);
}

inline HexPoint HexPointUtil::oppositeEdge(HexPoint edge)
{
    BenzeneAssert(isEdge(edge));
    if (edge == NORTH) return SOUTH;
    if (edge == SOUTH) return NORTH;
    if (edge == EAST)  return WEST;
    BenzeneAssert(edge == WEST);
    return EAST;
}

inline HexPoint HexPointUtil::leftEdge(HexPoint edge)
{
    BenzeneAssert(isEdge(edge));
    if (edge == NORTH) return EAST;
    if (edge == SOUTH) return WEST;
    if (edge == EAST)  return SOUTH;
    BenzeneAssert(edge == WEST);
    return NORTH;
}

inline HexPoint HexPointUtil::rightEdge(HexPoint edge)
{
    BenzeneAssert(isEdge(edge));
    if (edge == NORTH) return WEST;
    if (edge == SOUTH) return EAST;
    if (edge == EAST)  return NORTH;
    BenzeneAssert(edge == WEST);
    return SOUTH;
}

inline HexPoint HexPointUtil::colorEdge1(HexColor color)
{
    BenzeneAssert(HexColorUtil::isBlackWhite(color));
    return (color == VERTICAL_COLOR) ? NORTH : EAST;
}

inline HexPoint HexPointUtil::colorEdge2(HexColor color)
{
    BenzeneAssert(HexColorUtil::isBlackWhite(color));
    return (color == VERTICAL_COLOR) ? SOUTH : WEST;
}

inline bool HexPointUtil::isColorEdge(HexPoint cell, HexColor color)
{
    BenzeneAssert(HexColorUtil::isBlackWhite(color));
    return (cell == colorEdge1(color) || cell == colorEdge2(color));
}

inline void HexPointUtil::pointToCoords(HexPoint cell, int& x, int& y)
{
    BenzeneAssert(FIRST_CELL <= cell && cell < FIRST_INVALID);
    x = (cell-FIRST_CELL) % MAX_WIDTH;
    y = (cell-FIRST_CELL) / MAX_WIDTH;
}

inline HexPoint HexPointUtil::coordsToPoint(int x, int y)
{
    BenzeneAssert(0 <= x && x < MAX_WIDTH);
    BenzeneAssert(0 <= y && y < MAX_HEIGHT);
    return static_cast<HexPoint>(FIRST_CELL + (y*MAX_WIDTH) + x);
}

inline int HexPointUtil::DeltaX(int dir)
{
    BenzeneAssert(DIR_EAST <= dir && dir < NUM_DIRECTIONS);
    static const int dx[] = {1,  1,  0, -1, -1, 0};
    return dx[dir];
}

inline int HexPointUtil::DeltaY(int dir)
{
    BenzeneAssert(DIR_EAST <= dir && dir < NUM_DIRECTIONS);
    static const int dy[] = {0, -1, -1,  0,  1, 1};
    return dy[dir];
}

//----------------------------------------------------------------------------

/** Extends standard output operator to handle HexPoints. */
inline std::ostream& operator<<(std::ostream& os, HexPoint p)
{
    return os << HexPointUtil::ToString(p);
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // HEXPOINT_HPP
