//----------------------------------------------------------------------------
// $Id: GroupBoard.hpp 1821 2008-12-22 04:03:08Z broderic $
//----------------------------------------------------------------------------

#ifndef GROUPBOARD_H
#define GROUPBOARD_H

#include "Hex.hpp"
#include "StoneBoard.hpp"
#include "UnionFind.hpp"

//----------------------------------------------------------------------------

/**
 * Groups cells of same color into a single group. Union-Find is used
 * to group cells of the same color into a single group.  This allows
 * entire groups of connected cells to be treated as a single entity.
 *
 * The group calculation must be stable!!  That is, computing the
 * groups from scratch or incrementally must always result in the same
 * groups and the same captains for each group.  Breaking this
 * contract will cause horrible problems in many different places.
 *
 * NOTE: WE ASSUME AN EDGE IS ALWAYS THE CAPTAIN OF ITS GROUP!
 *
 * GroupBoard does not automatically update the group information,
 * this needs to be done outside the class by calling absorb() or
 * absorb(cell, color).
 *
 * Groups also allow the state of the game to be determined easily. 
 * See the getWinner() and isGameOver() methods.
 *
 */
class GroupBoard : public StoneBoard
{
public:

    /** Constructs a square board. */
    explicit GroupBoard(unsigned size);

    /** Constructs a rectangular board. */
    GroupBoard(unsigned width, unsigned height);

    /** Destructor. */
    virtual ~GroupBoard();

    //-----------------------------------------------------------------------

    /** Returns all the captains on the board for the given colorset. */
    const BoardIterator& Groups(HexColorSet colorset=ALL_COLORS) const;
    const BoardIterator& Groups(HexColor color) const;

    /** Returns number of groups in the colorset. */
    int NumGroups(HexColorSet colorset) const;

    /** Returns the index of the group in the colorsets list of
        groups. If value returned is n, then the BoardIterator
        returned by Groups(colorset) would equal group after n
        increments. */
    int GroupIndex(HexColorSet colorset, HexPoint group) const;

    /** Returns true if p1 and p2 are in the same group. */
    bool inSameGroup(HexPoint p1, HexPoint p2) const;

    /** Returns a bitset of the members of this group. */
    bitset_t GroupMembers(HexPoint cell) const;
    
    /** Returns a bitset of the captains of all locations set in
	this bitset. */
    bitset_t CaptainizeBitset(bitset_t locations) const;

    //-----------------------------------------------------------------------

    /** Returns a bitset containing the captains of the immediate
        neighbouring groups of group with color nb_color. */
    bitset_t Nbs(HexPoint group, HexColor nb_color) const;

    /** Like Nbs(group, HexColor), but instead returns all
        neighbouring groups belong to colorset. */
    bitset_t Nbs(HexPoint group, HexColorSet colorset) const;

    /** Same as Nbs(group, ALL_COLORS). */
    bitset_t Nbs(HexPoint group) const;
    
    //-----------------------------------------------------------------------

    /** Returns the representative of this cell's group. */
    HexPoint getCaptain(HexPoint x) const;

    /** Returns true if this cell is a captain. */
    bool isCaptain(HexPoint x) const;

    /** Returns the color of the winning player and EMPTY if no
        winner. */
    HexColor getWinner() const;
    
    /** Returns true if there is a winner. 
        Uses getCaptain(), see getWinner() for implications of this. */
    bool isGameOver() const;

    //-----------------------------------------------------------------------

    /** Tries to absorb the given cell; uses old absorb information.
        Does nothing if absorb() or absorb(cell) have been called
        already, or if cell is EMPTY. */
    void absorb(HexPoint cell);

    /** Updates absorb info on the given set of cells. All set cells
        in bs should be nonempty and recently added. */
    void absorb(const bitset_t& changed);

    /** Groups adjacent cells of the same color into a single
        group. Old absorb information is cleared beforehand. */
    void absorb();

protected:

    virtual void clear();

    UnionFind<BITSETSIZE> m_groups;

    mutable bool m_captains_computed;
    mutable std::vector<HexPoint> m_captain_list[NUM_COLOR_SETS];
    mutable BoardIterator m_captains[NUM_COLOR_SETS];

    mutable bool m_members_computed;
    mutable PointToBitset m_members;

    /** Computed neighbours.  A map<HexPoint, bitset> is too slow. */
    mutable bool m_nbs_computed;
    mutable bitset_t m_nbs[BLACK_WHITE_EMPTY][BITSETSIZE];

private:

    void init();

    void InvalidateCachedData();

    void internal_absorb(HexPoint cell);

};

inline const BoardIterator& GroupBoard::Groups(HexColor color) const
{
    return Groups(HexColorSetUtil::Only(color));
}

inline HexPoint GroupBoard::getCaptain(HexPoint x) const
{
    return static_cast<HexPoint>(m_groups.getRoot(x));
}

inline bool GroupBoard::inSameGroup(HexPoint p1, HexPoint p2) const
{
    return (getCaptain(p1) == getCaptain(p2));
}

inline bool GroupBoard::isCaptain(HexPoint cell) const
{
    return m_groups.isRoot(cell);
}

inline void GroupBoard::InvalidateCachedData()
{
    m_captains_computed = false;
    m_members_computed = false;
    m_nbs_computed = false;
}

//----------------------------------------------------------------------------

#endif // GROUPBOARD_H
