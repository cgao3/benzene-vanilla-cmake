//----------------------------------------------------------------------------
/** @file Groups.hpp
 */
//----------------------------------------------------------------------------

#ifndef GROUPS_HPP
#define GROUPS_HPP

#include "Hex.hpp"
#include "SafeBool.hpp"
#include "StoneBoard.hpp"

_BEGIN_BENZENE_NAMESPACE_

//---------------------------------------------------------------------------

/** A group on the board. */
class Group
{
public:

    /** Creates an empty invalid group. */
    Group();

    /** Creates a group with the given data. */
    Group(HexColor color, HexPoint captain, 
          const bitset_t& members, const bitset_t& nbs);

    std::size_t Size() const;

    HexColor Color() const;

    HexPoint Captain() const;

    bool IsMember(HexPoint point) const;

    const bitset_t& Members() const;

    const bitset_t& Nbs() const;

private:

    friend class GroupBuilder;

    HexColor m_color;

    HexPoint m_captain;

    bitset_t m_members;

    bitset_t m_nbs;
};

inline Group::Group()
    : m_captain(INVALID_POINT)
{
}

inline Group::Group(HexColor color, HexPoint captain, 
                    const bitset_t& members, const bitset_t& nbs)
    : m_color(color),
      m_captain(captain),
      m_members(members),
      m_nbs(nbs)
{
}

inline std::size_t Group::Size() const
{
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

/** Collection of groups. */
class Groups
{
public:

    Groups();

    std::size_t NumGroups() const;

    std::size_t NumGroups(HexColorSet colorset) const;

    std::size_t NumGroups(HexColor color) const;

    std::size_t GroupIndex(HexPoint point, HexColorSet colorset) const;

    std::size_t GroupIndex(HexPoint point, HexColor color) const;

    const Group& GetGroup(HexPoint point) const;

    HexPoint CaptainOf(HexPoint point) const;

    bool IsCaptain(HexPoint point) const;

    bitset_t Nbs(HexPoint point) const;

    bitset_t Nbs(HexPoint point, HexColorSet colorset) const;

    bitset_t Nbs(HexPoint point, HexColor color) const;

    bitset_t Nbs(const Group& group) const;

    bitset_t Nbs(const Group& group, HexColorSet colorset) const;

    bitset_t Nbs(const Group& group, HexColor color) const;

    /** Returns true black or white has won. */
    bool IsGameOver() const;

    /** Returns color of winning player. */
    HexColor GetWinner() const;

    /** Returns bitset with only the captains of any set groups. */
    bitset_t CaptainizeBitset(bitset_t locations) const;

    StoneBoard& Board();

    const StoneBoard& Board() const;

private:

    friend class GroupBuilder;

    friend class GroupIterator;

    StoneBoard* m_brd;

    std::vector<Group> m_groups;

    std::vector<int> m_group_index; // HexPoint -> index of m_groups
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
    return Nbs(point, HexColorSetUtil::Only(color));
}

inline bitset_t Groups::Nbs(const Group& group) const
{
    return Nbs(group.Captain());
}

inline bitset_t Groups::Nbs(const Group& group, HexColorSet colorset) const
{
    return Nbs(group.Captain(), colorset);
}

inline bitset_t Groups::Nbs(const Group& group, HexColor color) const
{
    return Nbs(group.Captain(), color);
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
    
    GroupIterator(const Groups& groups);

    GroupIterator(const Groups& groups, HexColorSet colorset);

    GroupIterator(const Groups& groups, HexColor color);

    /** Returns current group. */
    const Group& operator*() const;

    /** Allows access to current groups methods. */
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
