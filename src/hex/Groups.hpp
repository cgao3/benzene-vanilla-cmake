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

    const Group& GetGroup(HexPoint point) const;

private:

    friend class GroupBuilder;

    friend class GroupIterator;

    std::vector<Group> m_groups;

    std::vector<int> m_group_index; // HexPoint -> index of m_groups
};

inline Groups::Groups()
{
}

inline std::size_t Groups::NumGroups() const
{
    return m_groups.size();
}

inline const Group& Groups::GetGroup(HexPoint point) const
{
    return m_groups[m_group_index[point]];
}

//---------------------------------------------------------------------------

/** Iterates over an instance of Groups. */
class GroupIterator : public SafeBool<GroupIterator>
{
public:
    
    GroupIterator(const Groups& groups);

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

    std::size_t m_index;
};

inline GroupIterator::GroupIterator(const Groups& groups)
    : m_groups(&groups),
      m_index(0)
{
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
