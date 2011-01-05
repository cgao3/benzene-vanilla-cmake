//----------------------------------------------------------------------------
/** @file Groups.hpp
 */
//----------------------------------------------------------------------------

#ifndef GROUPS_HPP
#define GROUPS_HPP

#include "Hex.hpp"
#include "SafeBool.hpp"
#include "StoneBoard.hpp"
#include <boost/utility.hpp>

_BEGIN_BENZENE_NAMESPACE_

//---------------------------------------------------------------------------

class Groups;

/** A group on the board. 
    
    A group is a maximal set of like-colored stones. Groups of 
    color EMPTY are always singletons.
*/
class Group
{
public:

    /** Creates an empty invalid group. 
        Only GroupBuilder can construct valid groups.
    */
    Group();

    /** Number of stones in group. */
    std::size_t Size() const;

    /** Color of the group. */
    HexColor Color() const;

    /** Point used as the representative of this group. */
    HexPoint Captain() const;

    /** Returns true if point is a member of group, false otherwise. */
    bool IsMember(HexPoint point) const;

    /** Returns the members. */
    const bitset_t& Members() const;

    /** Returns neighbours. */
    const bitset_t& Nbs() const;
    
    /** Returns the captains of neighboring groups whose color belongs
        to colorset. If first time, neighbors in each colorset will be
        computed. */
    const bitset_t& Nbs(HexColorSet colorset) const;

private:

    friend class GroupBuilder;

    HexColor m_color;

    HexPoint m_captain;

    bitset_t m_members;

    bitset_t m_nbs;

    /** Pointer to Groups object which this Group belongs. */
    const Groups* m_groups;

    /** Indices of neighbouring groups in parent Groups's list of
        groups. We don't use pointers because then it would be
        difficult to copy a Groups object. */
    std::vector<std::size_t> m_nbs_index;

    /** True if the colorset neighbours have been computed yet. */
    mutable bool m_colorsets_computed;

    /** Computed colorset neighbours. */
    mutable bitset_t m_nbs_colorset[NUM_COLOR_SETS];

    Group(const Groups* groups, HexColor color, HexPoint captain, 
          const bitset_t& members, const bitset_t& nbs);

    void ComputeColorsetNbs() const;
};

inline Group::Group()
    : m_captain(INVALID_POINT),
      m_groups(0),
      m_colorsets_computed(false)
{
}

/** Used only by GroupBuilder::Build(). */
inline Group::Group(const Groups* groups, HexColor color, HexPoint captain, 
                    const bitset_t& members, const bitset_t& nbs)
    : m_color(color),
      m_captain(captain),
      m_members(members),
      m_nbs(nbs),
      m_groups(groups),
      m_colorsets_computed(false)
{
}

inline std::size_t Group::Size() const
{
    /** @todo Cache group size for speed? */
    return m_members.count();
}

inline HexColor Group::Color() const
{
    return m_color;
}

inline HexPoint Group::Captain() const
{
    return m_captain;
}

inline bool Group::IsMember(HexPoint point) const
{
    return m_members.test(point);
}

inline const bitset_t& Group::Members() const
{
    return m_members;
}

inline const bitset_t& Group::Nbs() const
{
    return m_nbs;
}

//---------------------------------------------------------------------------

/** Collection of groups. 

   @todo If a HexPosition class is ever created, store the 
   HexPosition for which these groups were computed.
*/
class Groups
{
public:

    /** Creates an empty set of groups. */
    Groups();

    /** Returns point's group. */
    const Group& GetGroup(HexPoint point) const;

    /** Returns captain of point's group. */
    HexPoint CaptainOf(HexPoint point) const;

    /** Returns true if point is captain of its group. */
    bool IsCaptain(HexPoint point) const;

    /** @name Group indexing methods. */
    // @{

    /** Returns number of groups. */
    std::size_t NumGroups() const;

    /** Returns number of groups with color belonging to colorset. */
    std::size_t NumGroups(HexColorSet colorset) const;

    /** Returns number of groups of color. */
    std::size_t NumGroups(HexColor color) const;

    /** Returns index of point's group in all groups belonging to
        to colorset. 
        @todo Take this out? Only used in Resistance.
    */
    std::size_t GroupIndex(HexPoint point, HexColorSet colorset) const;

    /** Returns index of point's group in all groups of color. 
        @todo Take this out? Only used in Resistance.
    */
    std::size_t GroupIndex(HexPoint point, HexColor color) const;

    // @}

    /** @name Neighbor convenience methods. */
    // @{

    bitset_t Nbs(HexPoint point) const;

    bitset_t Nbs(HexPoint point, HexColorSet colorset) const;

    bitset_t Nbs(HexPoint point, HexColor color) const;

    bitset_t Nbs(const Group& group) const;

    bitset_t Nbs(const Group& group, HexColorSet colorset) const;

    bitset_t Nbs(const Group& group, HexColor color) const;

    // @}

    /** Returns true if black or white has won. */
    bool IsGameOver() const;

    /** Returns color of winning player, EMPTY if IsGameOver() is
        false. */
    HexColor GetWinner() const;

    /** Returns bitset with only the captains of any set groups. */
    bitset_t CaptainizeBitset(bitset_t locations) const;

    /** Returns reference to board groups were computed on. Does not
        guarantee the board is in the same state it was in when groups
        where computed. */
    StoneBoard& Board();

    /** See Board(). */
    const StoneBoard& Board() const;

private:

    friend class Group;

    friend class GroupBuilder;

    friend class GroupIterator;

    StoneBoard* m_brd;

    std::vector<Group> m_groups;

    std::vector<std::size_t> m_group_index; // HexPoint -> index of m_groups
};

inline Groups::Groups()
    : m_brd(0)
{
}

inline std::size_t Groups::NumGroups() const
{
    return m_groups.size();
}

inline std::size_t Groups::NumGroups(HexColor color) const
{
    return NumGroups(HexColorSetUtil::Only(color));
}

inline std::size_t Groups::GroupIndex(HexPoint point, HexColor color) const
{
    return GroupIndex(point, HexColorSetUtil::Only(color));
}

inline const Group& Groups::GetGroup(HexPoint point) const
{
    return m_groups[m_group_index[point]];
}

inline bitset_t Groups::Nbs(HexPoint point) const
{
    return GetGroup(point).Nbs();
}

inline bitset_t Groups::Nbs(HexPoint point, HexColor color) const
{
    return GetGroup(point).Nbs(HexColorSetUtil::Only(color));
}

inline bitset_t Groups::Nbs(HexPoint point, HexColorSet colorset) const
{
    return GetGroup(point).Nbs(colorset);
}

inline bitset_t Groups::Nbs(const Group& group) const
{
    return GetGroup(group.Captain()).Nbs();
}

inline bitset_t Groups::Nbs(const Group& group, HexColor color) const
{
    return GetGroup(group.Captain()).Nbs(HexColorSetUtil::Only(color));
}

inline bitset_t Groups::Nbs(const Group& group, HexColorSet colorset) const
{
    return GetGroup(group.Captain()).Nbs(colorset);
}

inline HexPoint Groups::CaptainOf(HexPoint point) const
{
    return GetGroup(point).Captain();
}

inline bool Groups::IsCaptain(HexPoint point) const
{
    return GetGroup(point).Captain() == point;
}

inline StoneBoard& Groups::Board()
{
    return *m_brd;
}

inline const StoneBoard& Groups::Board() const
{
    return *m_brd;
}

//---------------------------------------------------------------------------

/** Iterates over an instance of Groups. */
class GroupIterator : public SafeBool<GroupIterator>
{
public:

    /** Creates an iterator over all groups. */
    GroupIterator(const Groups& groups);

    /** Creates an iterator over only those groups in colorset. */
    GroupIterator(const Groups& groups, HexColorSet colorset);

    /** Creates an iterator over only those groups of color. */
    GroupIterator(const Groups& groups, HexColor color);

    /** Returns current group. */
    const Group& operator*() const;

    /** Allows access to current group's methods. */
    const Group* operator->() const;

    /** Moves to the next group. */
    void operator++();

    /** Used by SafeBool. */
    bool boolean_test() const;

private:

    const Groups* m_groups;

    HexColorSet m_colorset;

    std::size_t m_index;

    void FindNextInColorSet();
};

inline GroupIterator::GroupIterator(const Groups& groups)
    : m_groups(&groups),
      m_colorset(ALL_COLORS),
      m_index(0)
{
}

inline GroupIterator::GroupIterator(const Groups& groups, HexColorSet colorset)
    : m_groups(&groups),
      m_colorset(colorset),
      m_index(0)
{
    FindNextInColorSet();
}

inline GroupIterator::GroupIterator(const Groups& groups, HexColor color)
    : m_groups(&groups),
      m_colorset(HexColorSetUtil::Only(color)),
      m_index(0)
{
    FindNextInColorSet();
}

inline const Group& GroupIterator::operator*() const
{
    return m_groups->m_groups[m_index];
}

inline const Group* GroupIterator::operator->() const
{
    return &m_groups->m_groups[m_index];
}

inline void GroupIterator::operator++()
{
    ++m_index;
    FindNextInColorSet();
}

inline void GroupIterator::FindNextInColorSet()
{
    while (m_index < m_groups->m_groups.size() 
           && !HexColorSetUtil::InSet(m_groups->m_groups[m_index].Color(),
                                      m_colorset))
        ++m_index;
}

inline bool GroupIterator::boolean_test() const
{
    return m_index < m_groups->m_groups.size();
}

//---------------------------------------------------------------------------

/** Builds Groups from a StoneBoard. */
class GroupBuilder
{
public:

    /** Computes Groups. */
    static void Build(const StoneBoard& brd, Groups& groups);

private:

};

//---------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // GROUPS_HPP
