//----------------------------------------------------------------------------
/** @file ConstBoard.cpp */
//----------------------------------------------------------------------------

#include "ConstBoard.hpp"
#include "BoardUtil.hpp"
#include "BitsetIterator.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

namespace {

//----------------------------------------------------------------------------

int DistanceToEdge(const ConstBoard& brd, HexPoint from, HexPoint edge)
{
    BenzeneAssert(HexPointUtil::isEdge(edge));
    
    if (HexPointUtil::isEdge(from)) 
    {
	if (from == edge)                             return 0;
	if (HexPointUtil::oppositeEdge(from) != edge) return 1;
	if (edge == NORTH || edge == SOUTH)           return brd.Height();
	return brd.Width();
    }
    
    int r, c;
    HexPointUtil::pointToCoords(from, c, r);
    switch(edge) 
    {
    case NORTH: return r + 1;
    case SOUTH: return brd.Height() - r;
    case EAST:  return brd.Width() - c;
    default:    return c + 1;
    }
}

//----------------------------------------------------------------------------

} // namespace

//----------------------------------------------------------------------------

ConstBoard& ConstBoard::Get(int size)
{
    return Get(size, size);
}

ConstBoard& ConstBoard::Get(int width, int height)
{
    // Must use a vector of pointers since a vector of ConstBoards could
    // resize itself and invalidate all pointers into it.  
    // FIXME: how to ensure memory is freed nicely?
    static std::vector<ConstBoard*> s_brds;
    for (std::size_t i=0; i<s_brds.size(); ++i) {
        if (s_brds[i]->Width() == width && s_brds[i]->Height() == height)
            return *s_brds[i];
    }
    s_brds.push_back(new ConstBoard(width, height));
    return *s_brds.back();
}

//----------------------------------------------------------------------------

ConstBoard::ConstBoard(int size) 
    : m_width(size),
      m_height(size)
{
    BenzeneAssert(1 <= m_width && m_width <= MAX_WIDTH);
    BenzeneAssert(1 <= m_height && m_height <= MAX_HEIGHT);
    Init();
}

ConstBoard::ConstBoard(int width, int height)
    : m_width(width),
      m_height(height)
{
    BenzeneAssert(1 <= m_width && m_width <= MAX_WIDTH);
    BenzeneAssert(1 <= m_height && m_height <= MAX_HEIGHT);
    Init();
}

ConstBoard::~ConstBoard()
{
}

//----------------------------------------------------------------------

bool ConstBoard::Adjacent(HexPoint p1, HexPoint p2) const
{
    for (BoardIterator it(Nbs(p1)); it; ++it)
        if (*it == p2) 
            return true;
    return false;
}

int ConstBoard::Distance(HexPoint x, HexPoint y) const
{
    BenzeneAssert(IsValid(x));
    BenzeneAssert(IsValid(y));
    
    if (HexPointUtil::isEdge(y)) 
        return DistanceToEdge(*this, x, y);
    else if (HexPointUtil::isEdge(x)) 
        return DistanceToEdge(*this, y, x);
    
    int r1, c1, r2, c2;
    HexPointUtil::pointToCoords(x, r1, c1);
    HexPointUtil::pointToCoords(y, r2, c2);
    int dr = r1 - r2;
    int dc = c1 - c2;

    return (dr*dc >= 0) 
        ? std::abs(dr) + std::abs(dc) 
        : std::max(std::abs(dr), std::abs(dc));
}

//----------------------------------------------------------------------

void ConstBoard::Init()
{
    LogFine() << "--- ConstBoard"
              << " (" << Width() << " x " << Height() << ")" << '\n';
    ComputePointList();
    CreateIterators();
    ComputeValid();
    ComputeNeighbours();
}

void ConstBoard::ComputePointList()
{
    /** @note There are several pieces of code that rely on the fact
        points are visited in the order (a1,b1,...,a2,b2,...,etc)
        (StoneBoard::GetBoardID() is one).  Do not change this order
        unless you know what you are doing!!.
    */
    for (int p = FIRST_SPECIAL; p < FIRST_CELL; ++p) 
        m_points.push_back(static_cast<HexPoint>(p));

    for (int y = 0; y < Height(); y++)
        for (int x = 0; x < Width(); x++)
            m_points.push_back(HexPointUtil::coordsToPoint(x, y));

    m_points.push_back(INVALID_POINT);
}

void ConstBoard::CreateIterators()
{
    int p = 0;
    while (m_points[p] != FIRST_SPECIAL) p++;
    m_all_index = p;

    while (m_points[p] != FIRST_EDGE) p++;
    m_locations_index = p;

    while (m_points[p] != FIRST_CELL) p++;
    m_cells_index = p;
}

void ConstBoard::ComputeValid()
{
    m_valid.reset();
    for (BoardIterator i(AllValid()); i; ++i)
        m_valid.set(*i);

    m_locations.reset();
    for (BoardIterator i(EdgesAndInterior()); i; ++i)
        m_locations.set(*i);

    m_cells.reset();
    for (BoardIterator i(Interior()); i; ++i)
        m_cells.set(*i);
}

void ConstBoard::ComputeNeighbours()
{
    // Try all directions for interior cells
    for (BoardIterator i(Interior()); i; ++i) 
    {
        int x, y;
        HexPoint cur = *i;
        HexPointUtil::pointToCoords(cur, x, y);
        for (int a = 0; a < NUM_DIRECTIONS; a++) 
        {
            int fwd = a;
            int lft = (a + 2) % NUM_DIRECTIONS;
            int x1 = x + HexPointUtil::DeltaX(fwd);
            int y1 = y + HexPointUtil::DeltaY(fwd);
            for (int r = 1; r <= Pattern::MAX_EXTENSION; r++) 
            {
                int x2 = x1;
                int y2 = y1;
                for (int t = 0; t < r; t++) 
                {
                    HexPoint p = BoardUtil::CoordsToPoint(*this, x2, y2);
                    if (p != INVALID_POINT) 
                    {
                        // Add p to cur's list and cur to p's list
                        // for each radius in [r, MAX_EXTENSION]. 
                        for (int v=r; v <= Pattern::MAX_EXTENSION; v++) 
                        {
                            std::vector<HexPoint>::iterator result;
                           
                            result = find(m_neighbours[cur][v].begin(), 
                                          m_neighbours[cur][v].end(), p);
                            if (result == m_neighbours[cur][v].end())
                                m_neighbours[cur][v].push_back(p);
                            
                            result = find(m_neighbours[p][v].begin(), 
                                          m_neighbours[p][v].end(), cur);
                            if (result == m_neighbours[p][v].end())
                                m_neighbours[p][v].push_back(cur);
                        }
                    }
                    x2 += HexPointUtil::DeltaX(lft);
                    y2 += HexPointUtil::DeltaY(lft);
                }
                x1 += HexPointUtil::DeltaX(fwd);
                y1 += HexPointUtil::DeltaY(fwd);
            }
        }
    }
    
    /** Add edges to neighbour lists of neighbouring edges.
        @bug NORTH is now distance 2 from SOUTH, but won't appear in
        the neighbour lists for r >= 2; likewise for EAST/WEST. 
    */
    for (BoardIterator i(EdgesAndInterior()); i; ++i) 
    {
	if (!HexPointUtil::isEdge(*i)) 
            continue;
        for (int r = 1; r <= Pattern::MAX_EXTENSION; r++) 
        {
	    // Edges sharing a corner are distance one apart
            m_neighbours[*i][r].push_back(HexPointUtil::leftEdge(*i));
            m_neighbours[*i][r].push_back(HexPointUtil::rightEdge(*i));
        }
    }
    // Null terminate the lists.
    for (BoardIterator i(EdgesAndInterior()); i; ++i) 
        for (int r = 1; r <= Pattern::MAX_EXTENSION; r++) 
            m_neighbours[*i][r].push_back(INVALID_POINT);
}

//----------------------------------------------------------------------------
