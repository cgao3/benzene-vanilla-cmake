//----------------------------------------------------------------------------
/** @file VCList.hpp */
//----------------------------------------------------------------------------

#ifndef VCLIST_HPP
#define VCLIST_HPP

#include <limits>
#include "ChangeLog.hpp"
#include "SafeBool.hpp"
#include "VC.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Sorted list of VCs.
    The list is sorted by VC carrier size. When VCs are added, only
    VCs that are not duplicates or supersets of existing VCs in the
    list are considered; any elements currently in the list that are
    supersets of the new vc are removed. This means an add operation
    takes linear time.
    The soft limit is the number of vcs used in vc calculations. VCs
    that appear after the soft limit are not considered in vc
    calculations, but may be later on vcs under the softlimit are
    removed. The idea is to keep around extra vcs that may be
    important later, but not at the expense of slower vc calculations
    now. */
class VCList
{
public:
    /** Creates a list. */
    VCList(HexPoint x, HexPoint y);

    HexPoint GetX() const;
    
    HexPoint GetY() const;

    /** Empties the list. */
    void Clear();

    /** Returns the number of VCs in this list. */
    std::size_t Size() const;

    /** Returns true if the list is empty. */
    bool Empty() const;

    /** Dumps the contents of this list to a string. */
    std::string Dump() const;

    //----------------------------------------------------------------------

    /** Returns true if bs is superset of connection in list. */
    bool IsSupersetOfAny(const bitset_t& vc) const;

    /** Returns true if vc is subset of connection in list. */
    bool IsSubsetOfAny(const bitset_t& vc) const;

    /** Removes any connection whose carrier is a superset of the
        given VC's carrier. Returns number of connections
        removed. Does not dirty intersections if flag is set to false;
        only use this if you are for sure only removing supersets of a
        member of this list! */
    std::size_t RemoveSuperSetsOf(const bitset_t& vc, ChangeLog<VC>* log,
                                  bool dirtyIntersection = true);
   
    //----------------------------------------------------------------------
    
    /** @name Adding connections */
    // @{

    /** Adds vc to the list.  
        Requires a single pass over the entire list. The add will fail
        if vc is a superset of some vc already in the list. Supersets
        of this vc that are already in the list are removed.
        Operations will be recorded in the log if log is not 0.
        @return true if the VC was added, false otherwise. */
    bool Add(const VC& vc, ChangeLog<VC>* log);

    /** Adds the elements of other to list. 
        Returns number of vcs added. VCs are added as unprocessed.
        Operations will be recorded in the log if log is not 0. 
        @return Number of elements added to other list. */
    std::size_t Add(const VCList& other, ChangeLog<VC>* log);

    /** Force the addition of this VC.
        This method does NO checks. It simply adds vc at the proper
        position according to the ordering on the vcs.  Useful for
        adding superset VCs that were previously removed (by the
        changelog, say). */
    void ForcedAdd(const VC& vc);

    // @}

    //----------------------------------------------------------------------

    /** Returns the union of all carriers in the list. */
    bitset_t GetUnion() const;

    /** Returns union of all carriers in the list.
        Only includes carriers of VCs in the union if they shrink the
        intersection at each step (considered in the sorted order).
        @note Doing this may result in different sets of connections
        depending upon the relative order of the VCs in the list. So,
        on an empty board, BLACK can have a slightly different
        connection set that WHITE. Can also result in different VC
        connections between non-rotated and rotated versions of the
        board. */
    bitset_t GetGreedyUnion() const;

    /** Returns the intersection of all carriers in the list.
        If list is empty, the bitset will have all of its bits set. */
    bitset_t GetIntersection() const;

    /** Returns the intersection of all carriers without keys in the list.
     *        If list is empty, the bitset will have all of its bits set. */
    bitset_t GetKeyfreeIntersect() const;

    //----------------------------------------------------------------------

    /** Returns pointer to occurance of VC in the list.
        Returns 0 if VC is not in the list. */
    VC* FindInList(const VC& vc);

    /** Removes the given vc from the list.  Does nothing if vc is not
        actually in the list.  Takes O(n) time. Returns true if vc
        was actually removed, false otherwise. */
    bool Remove(const VC& vc, ChangeLog<VC>* log);

    /** Removes all VCs that intersect with cell. 
        Removed VCs are appended to out---note that the order
        of the vcs in out is the same as it was in the original list.
        Returns number of vcs removed. */
    std::size_t RemoveAllContaining(HexPoint cell, std::vector<VC>& out,
                                    ChangeLog<VC>* log);

    /** Removes all VCs that intersect with b. 
        Removed VCs are stored in out. */
    std::size_t RemoveAllContaining(const bitset_t& b, std::vector<VC>& out,
                                    ChangeLog<VC>* log);

    /** Removes all VCs that intersect with b. */
    std::size_t RemoveAllContaining(const bitset_t& b, ChangeLog<VC>* log);
   
    //----------------------------------------------------------------------

    /** Performs list equality. */
    bool operator==(const VCList& other) const;

    /** Performs list inequality. */
    bool operator!=(const VCList& other) const;

protected:
    HexPoint m_x;
        
    HexPoint m_y;

    mutable std::vector<VC> m_vcs;

    mutable bool m_dirtyIntersection;

    mutable bitset_t m_intersection;

    mutable bitset_t m_keyfree_intersect;

    mutable bool m_dirtyUnion;

    mutable bitset_t m_union;

    mutable bitset_t m_greedyUnion;

    /** Invalidates the precomputed list unions. */
    void DirtyListUnions();

    /** Invalidates the list intersection. */
    void DirtyListIntersection();

    /** Computes list unions in one pass: normal and greedy. */
    void ComputeUnions() const;

    /** Computes list intersection. */
    void ComputeIntersection() const;

    friend class VCListIterator;

    friend class VCListConstIterator;
};

inline HexPoint VCList::GetX() const
{
    return m_x;
}

inline HexPoint VCList::GetY() const
{
    return m_y;
}

inline std::size_t VCList::Size() const
{
    return m_vcs.size();
}

inline bool VCList::Empty() const
{
    return m_vcs.empty();
}

inline void VCList::DirtyListUnions()
{
    m_dirtyUnion = true;
}

inline void VCList::DirtyListIntersection()
{
    m_dirtyIntersection = true;
}

inline void VCList::Clear()
{
    m_vcs.clear();
    DirtyListUnions();
    m_dirtyIntersection = false;
    m_intersection.set();
    m_keyfree_intersect.set();
}

//----------------------------------------------------------------------------

/** Iterates over a VCList. */
class VCListIterator : public SafeBool<VCListIterator>
{
public:
    /** Creates iterator on a VCList. */
    explicit VCListIterator(VCList& lst);

    /** Returns current VC. */
    VC& operator*();

    /** Allows access to current VC's methods. */
    VC* operator->();

    /** Moves to the next VC. */
    void operator++();

    /** Used by SafeBool. */
    bool boolean_test() const;

private:
    std::vector<VC>& m_lst;
    std::size_t m_count;
};

inline VCListIterator::VCListIterator(VCList& lst)
    : m_lst(lst.m_vcs),
      m_count(0)
{
}

inline VC& VCListIterator::operator*()
{
    return m_lst[m_count];
}

inline VC* VCListIterator::operator->()
{
    return &m_lst[m_count];
}

inline void VCListIterator::operator++()
{
    ++m_count;
}

inline bool VCListIterator::boolean_test() const
{
    return m_count < m_lst.size();
}

//----------------------------------------------------------------------------

/** Iterates over a VCList.
    Does not allow VC to be modified.
 */
class VCListConstIterator : public SafeBool<VCListConstIterator>
{
public:
    /** Creates a constnat iterator on a VCList. */
    VCListConstIterator(const VCList& lst);

    /** Returns current VC. */
    const VC& operator*() const;

    /** Allows access to current VC's methods. */
    const VC* operator->() const;

    /** Moves to the next VC. */
    void operator++();

    /** Used by SafeBool. */
    bool boolean_test() const;

private:
    const std::vector<VC>& m_lst;
    std::size_t m_count;
};

inline VCListConstIterator::VCListConstIterator(const VCList& lst)
    : m_lst(lst.m_vcs),
      m_count(0)
{
}

inline const VC& VCListConstIterator::operator*() const
{
    return m_lst[m_count];
}

inline const VC* VCListConstIterator::operator->() const
{
    return &m_lst[m_count];
}

inline void VCListConstIterator::operator++()
{
    ++m_count;
}

inline bool VCListConstIterator::boolean_test() const
{
    return m_count < m_lst.size();
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // VCLIST_HPP
