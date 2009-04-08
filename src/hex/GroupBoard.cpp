//----------------------------------------------------------------------------
// $Id: GroupBoard.cpp 1994 2009-04-06 00:57:12Z broderic $
//----------------------------------------------------------------------------

#include "BitsetIterator.hpp"
#include "GraphUtils.hpp"
#include "GroupBoard.hpp"

//----------------------------------------------------------------------------

GroupBoard::GroupBoard( unsigned size )
    : StoneBoard(size)
{
    init();
}

GroupBoard::GroupBoard( unsigned width, unsigned height )
    : StoneBoard(width, height)
{
    init();
}

void GroupBoard::init()
{
    LogFine() << "--- GroupBoard" << '\n'
	      << "sizeof(GroupBoard) = " << sizeof(GroupBoard) << '\n';
    m_groups.clear();
    InvalidateCachedData();
}

GroupBoard::~GroupBoard()
{
}

//----------------------------------------------------------------------------

HexColor GroupBoard::getWinner() const
{
    for (BWIterator it; it; ++it) {
    if (getCaptain(HexPointUtil::colorEdge1(*it)) == 
        getCaptain(HexPointUtil::colorEdge2(*it)))
        return *it;
    }
    return EMPTY;
}

bool GroupBoard::isGameOver() const
{
    return (getWinner() != EMPTY);
}

//----------------------------------------------------------------------------

const BoardIterator& GroupBoard::Groups(HexColorSet colorset) const
{
    if (!m_captains_computed) {

        for (int i=0; i<NUM_COLOR_SETS; ++i) 
            m_captain_list[i].clear();
    
        for (BoardIterator p(EdgesAndInterior()); p; ++p) {
            if (!isCaptain(*p)) continue;

            HexColor color = getColor(*p);
            for (int i=0; i<NUM_COLOR_SETS; ++i) {
                if (HexColorSetUtil::InSet(color, (HexColorSet)i))
                    m_captain_list[i].push_back(*p);
            }
        }

        for (int i=0; i<NUM_COLOR_SETS; ++i) {
            m_captain_list[i].push_back(INVALID_POINT);
            m_captains[i] = BoardIterator(m_captain_list[i]);
        }

        m_captains_computed = true;
    }

    return m_captains[colorset];
}

int GroupBoard::NumGroups(HexColorSet colorset) const
{
    // force computation of groups if not computed already 
    if (!m_captains_computed) {
        Groups(colorset);  
    }

    // subtract 1 because of the INVALID_POINT appended at the end
    return m_captain_list[colorset].size()-1;
}

int GroupBoard::GroupIndex(HexColorSet colorset, HexPoint group) const
{
    // force computation of groups if not computed already 
    if (!m_captains_computed) {
        Groups(colorset);  
    }

    /** find group in the list
        @todo cache this? */
    int n = m_captain_list[colorset].size()-1;
    for (int i=0; i<n; ++i) {
        if (m_captain_list[colorset][i] == group)
            return i;
    }
    HexAssert(false);
    return 0;
}

bitset_t GroupBoard::GroupMembers(HexPoint cell) const
{
    if (!m_members_computed) {
        m_members.clear();

        for (BoardIterator p(EdgesAndInterior()); p; ++p)
            m_members[getCaptain(*p)].set(*p);
        
        m_members_computed = true;
    }
    return m_members[getCaptain(cell)];
}

bitset_t GroupBoard::CaptainizeBitset(bitset_t locations) const
{
    HexAssert(isLocation(locations));
    bitset_t captains;
    for (BitsetIterator i(locations); i; ++i) {
        captains.set(getCaptain(*i));
    }
    return captains;
}

//----------------------------------------------------------------------------

bitset_t GroupBoard::Nbs(HexPoint group, HexColor nb_color) const
{
    if (!m_nbs_computed) {
        
        memset(m_nbs, 0, sizeof(m_nbs));
        for (BoardIterator p(EdgesAndInterior()); p; ++p) {
            HexPoint pcap = getCaptain(*p);
            HexColor pcolor = getColor(*p);
            for (BoardIterator nb(ConstNbs(*p)); nb; ++nb) {
                HexPoint ncap = getCaptain(*nb);
                HexColor ncolor = getColor(*nb);
                if (ncap != pcap) {
                    m_nbs[ncolor][pcap].set(ncap);
                    m_nbs[pcolor][ncap].set(pcap);
                }
            }
        }

        m_nbs_computed = true;
    }

    return m_nbs[nb_color][getCaptain(group)];
}

bitset_t GroupBoard::Nbs(HexPoint group, HexColorSet colorset) const
{
    bitset_t ret;
    for (ColorIterator color; color; ++color) 
        if (HexColorSetUtil::InSet(*color, colorset))
            ret |= Nbs(group, *color);
    return ret;
}

bitset_t GroupBoard::Nbs(HexPoint group) const
{
    return Nbs(group, ALL_COLORS);
}

//----------------------------------------------------------------------------
// Modifying methods
//
// All methods need to ensure that InvalidateCachedData() is called
// before they return control to caller.

void GroupBoard::clear()
{
    StoneBoard::clear();
    
    m_groups.clear();
    InvalidateCachedData();
}

/** Does the actual absorb; does not invalidate the cached
    data--caller should do so after calling this for each cell that
    needs to be absorbed. */
void GroupBoard::internal_absorb(HexPoint cell)
{
    HexColor color = getColor(cell);
    /// @todo leave early if absorbing empty cell?
    HexAssert(color != EMPTY);            
    for (BoardIterator i(ConstNbs(cell)); i; ++i)
        if (getColor(*i) == color)
            m_groups.unionGroups(cell, *i);
}

void GroupBoard::absorb(HexPoint cell)
{
    internal_absorb(cell);
    InvalidateCachedData();    
}

void GroupBoard::absorb(const bitset_t& changed)
{
    for (BitsetIterator p(changed); p; ++p)
        internal_absorb(*p);
    InvalidateCachedData();
}

void GroupBoard::absorb()
{
    m_groups.clear();
    for (BitsetIterator p(getBlack() | getWhite()); p; ++p)
        internal_absorb(*p);
    InvalidateCachedData();
}

//----------------------------------------------------------------------------
