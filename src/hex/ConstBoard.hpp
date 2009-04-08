//----------------------------------------------------------------------------
// $Id: ConstBoard.hpp 1786 2008-12-14 01:55:45Z broderic $
//----------------------------------------------------------------------------

#ifndef CONSTBOARD_H
#define CONSTBOARD_H

#include "Hex.hpp"
#include "BoardIterator.hpp"
#include "Pattern.hpp"

//----------------------------------------------------------------------------

/** @page boardrepresentation Board Representation
    The HexPoints on the board are laid out as in the following diagram
    (see @ref hexpoints for information on HexPoints):
  
    @verbatim

                    NORTH
               \--a--b--c-...-\
               1\  0  1  2 ... \ 1
      WEST      2\ 16 17 18 ... \ 2  EAST
                 3\ 32 33 34 ... \ 3
                   \--a--b--c-...-\
                        SOUTH
    @endverbatim
*/

/** @page cellneighbours Cell Neighbours 
    The neighbour lists for the interior cells behave as you would
    expect, e.q. a1 is adjacent to b1, NORTH, WEST, and a2. For edges,
    adjacent edges are added to the neighbour lists for all radius',
    but the closure of this is not computed.  For example, WEST is in
    the radius 1 neighbour list of NORTH, but SOUTH is not in the
    radius 2 neighbour list of NORTH.  Nor is this closure computed
    for interior cells over edges; e.g, a1 is distance 1 from NORTH
    but not distance 2 from EAST (except on a 1x1 board, of course).
*/

/** ConstBoard contains data and methods for dealing with the constant
    aspects of a Hex board. That is, ConstBoard stores a cell's
    neighbours, cell-to-cell distances, etc. It also offers iterators
    to run over the board and the neighbours of a cell.

    Only a single instance of ConstBoard exists for each boardsize.

    See @ref boardrepresentation for how the points are laid out on an
    example board, and @ref cellneighbours for a discussion on how
    neighbours are calculated.

    This class does not track played stones. To keep track of played
    stone information see StoneBoard.
 */
class ConstBoard
{
public:

    /** Creates a square board or returns pre-existing instance of a
        board of that size. */
    static ConstBoard& Get(int size);

    /** Creates a non-square board or returns pre-existing instance of
        a board of that size. */
    static ConstBoard& Get(int width, int height);
    
    //------------------------------------------------------------------------

    /** Returns the width of the board. */
    int width() const;

    /** Returns the height of the board. */
    int height() const;

    /** Returns a bitset with all valid board cells. */
    bitset_t getCells() const;

    /** Returns a bitset with all valid board locations
	(cells and edges). */
    bitset_t getLocations() const;

    /** Returns a bitset of cells comprising all valid moves
	(this includes swap and resign). */
    bitset_t getValid() const;
    
    /** Returns true if cell is a valid cell on this board. */
    bool isCell(HexPoint cell) const;

    /** Returns true if bs encodes a set of valid cells. */
    bool isCell(const bitset_t& bs) const;
    
    /** Returns true if cell is a location on this board. */
    bool isLocation(HexPoint cell) const;

    /** Returns true if bs encodes a set of valid locations. */
    bool isLocation(const bitset_t& bs) const;
    
    /** Returns true if cell is a valid move on this board. */
    bool isValid(HexPoint cell) const;

    /** Returns true if bs encodes a set of valid moves. */
    bool isValid(const bitset_t& bs) const;

    /** Returns true if p1 is adjacent to p2. Iterates over neighbour
        list of p1, so not O(1). */
    bool Adjacent(HexPoint p1, HexPoint p2) const;

    /** Returns the distance between two valid HexPoints. */
    int Distance(HexPoint x, HexPoint y) const;

    //------------------------------------------------------------------------

    /** Returns iterator to the interior board cells. */
    BoardIterator Interior() const;

    /** Returns iterator to the board cells, starting on
        the outer edges. */
    BoardIterator EdgesAndInterior() const;

    /** Returns iterator that runs over all valid moves. */
    BoardIterator AllValid() const;

    /** Returns iterator to the first neighbour of cell. 
        @see @ref cellneighbours
    */
    BoardIterator ConstNbs(HexPoint cell) const;

    /** Returns iterator to the neighbourhood extending outward by
        radius cells of cell. */
    BoardIterator ConstNbs(HexPoint cell, int radius) const;

    //------------------------------------------------------------------------

    /** @name Operators */
    // @{

    bool operator==(const ConstBoard& other) const;

    bool operator!=(const ConstBoard& other) const;

    // @}

private:

    /** Constructs a square board. */
    ConstBoard(int size);

    /** Constructs a rectangular board. */
    ConstBoard(int width, int height);

    /** Destructor. */
    ~ConstBoard();

    void Init();

    void ComputeNeighbours();

    void ComputePointList();

    void CreateIterators();

    void ComputeValid();

    //------------------------------------------------------------------------

    int m_width;
    
    int m_height;

    /** The set of all valid cells/moves. Assumed to be in the
        following order: special moves, edges, interior cells. */
    std::vector<HexPoint> m_points;

    /** Will probably always be zero. 
        @todo remove? */
    int m_all_index;

    /** Index in points where edges start. */
    int m_locations_index;

    /** Index in points where interior cells start. */
    int m_cells_index;

    /** All valid moves/cells. */
    bitset_t m_valid;
        
    /** All valid locations. */
    bitset_t m_locations;

    /** All valid interior cells. */
    bitset_t m_cells;

    /** Neigbour lists for each location and radius. */
    std::vector<HexPoint> m_neighbours[BITSETSIZE][Pattern::MAX_EXTENSION+1];
};

inline int ConstBoard::width() const
{ 
    return m_width; 
}

inline int ConstBoard::height() const
{ 
    return m_height;
}

inline bitset_t ConstBoard::getCells() const
{
    return m_cells;
}

inline bitset_t ConstBoard::getLocations() const
{
    return m_locations;
}

inline bitset_t ConstBoard::getValid() const
{
    return m_valid;
}

inline bool ConstBoard::isCell(HexPoint cell) const
{
    return m_cells.test(cell);
}

inline bool ConstBoard::isCell(const bitset_t& bs) const
{
    return BitsetUtil::IsSubsetOf(bs, m_cells);
}

inline bool ConstBoard::isLocation(HexPoint cell) const
{
    return m_locations.test(cell);
}

inline bool ConstBoard::isLocation(const bitset_t& bs) const
{
    return BitsetUtil::IsSubsetOf(bs, m_locations);
}

inline bool ConstBoard::isValid(HexPoint cell) const
{
    return m_valid.test(cell);
}

inline bool ConstBoard::isValid(const bitset_t& bs) const
{
    return BitsetUtil::IsSubsetOf(bs, m_valid);
}

inline BoardIterator ConstBoard::Interior() const
{
    return BoardIterator(&m_points[m_cells_index]);
}

inline BoardIterator ConstBoard::EdgesAndInterior() const
{ 
    return BoardIterator(&m_points[m_locations_index]);
}

inline BoardIterator ConstBoard::AllValid() const
{ 
    return BoardIterator(&m_points[m_all_index]);
}

inline BoardIterator ConstBoard::ConstNbs(HexPoint cell) const
{
    HexAssert(isLocation(cell));
    return BoardIterator(m_neighbours[cell][1]);
}

inline BoardIterator ConstBoard::ConstNbs(HexPoint cell, int radius) const
{
    HexAssert(isLocation(cell));
    HexAssert(0 <= radius && radius <= Pattern::MAX_EXTENSION);
    return BoardIterator(m_neighbours[cell][radius]);
}

inline bool ConstBoard::operator==(const ConstBoard& other) const
{
    return m_width == other.m_width 
        && m_height == other.m_height;
}

inline bool ConstBoard::operator!=(const ConstBoard& other) const
{
    return !operator==(other);
}

//----------------------------------------------------------------------------

#endif  // CONSTBOARD_HPP
