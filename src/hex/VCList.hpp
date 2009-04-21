//----------------------------------------------------------------------------
/** @file
 */
//----------------------------------------------------------------------------

#ifndef VCLIST_HPP
#define VCLIST_HPP

#include "ChangeLog.hpp"
#include "VC.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------

/** Sorted list of VCs.
   
    The list is sorted by VC carrier size. When VCs are added, only
    VCs that are not duplicates or supersets of existing VCs in the
    list are considered; any elements currently in the list that are
    supersets of the new vc are removed. This means an add operation
    will run in linear time.

    The soft limit is the number of vcs used in vc calculations.  VCs
    that appear after the soft limit are not considered in vc
    calculations, but may be later on if some vcs are removed.  The
    idea is to keep around extra vcs that may be important later, but
    not at the expense of slower vc calculations now.
*/
class VCList
{
public:

    /** Creates a list with given limits. */
    VCList(HexPoint x, HexPoint y, unsigned int soft);

    HexPoint getX() const;
    
    HexPoint getY() const;

    /** Returns the soft limit. */
    int softlimit() const;

    /** See softlimit() */
    void setSoftLimit(int limit);

    /** Empties the list and the changelog. */
    void clear();

    /** Returns the number of VCs in this list. */
    int size() const;

    /** Returns true if the list is empty. */
    bool empty() const;

    /** Dumps the contents of this list to a string. */
    std::string dump() const;

    //----------------------------------------------------------------------

    /** Returns true if bs is superset of connection in list. */
    bool isSupersetOfAny(const bitset_t& vc) const;

    /** Returns true if vc is subset of connection in list. */
    bool isSubsetOfAny(const bitset_t& vc) const;

    /** Removes any connection whose carrier is a superset of the
        given VC's carrier.  Returns number of connections
        removed. Does not dirty intersections if flag is set to false;
        only use this if you are for sure only removing supersets of a
        member of this list!
    */
    int removeSuperSetsOf(const bitset_t& vc, ChangeLog<VC>* log,
                          bool dirty_intersections = true);
   
    //----------------------------------------------------------------------
    
    /** @name Adding connections */
    // @{

    /** Return type for add() methods. */
    typedef enum 
    { 
        /** Add did not succeed, list is unchanged. */
        ADD_FAILED = 0, 

        /** Added connection within the soft limit. */
        ADDED_INSIDE_SOFT_LIMIT = 1, 
        
        /** Add connection within the hard limit. */
        ADDED_INSIDE_HARD_LIMIT = 2 

    } AddResult;

    /** Adds vc to the list.  
        
        Requires a single pass over the entire list. The add will fail
        if vc is a superset of some vc already in the list.  Supersets
        of this vc that are already in the list are removed.  
        
        Operations will be recorded in the log if log is not 0.

        @return true if the VC was added, false otherwise.
    */
    AddResult add(const VC& vc, ChangeLog<VC>* log);


    /** Adds the elements of other to list. Returns number of vcs
        added. Vcs are added as unprocessed. 
        Operations will be recorded in the log if log is not 0. */
    int add(const VCList& other, ChangeLog<VC>* log);

    /** 
     * Force the addition of this vc.  Used by VCSet::revert():
     *
     *    1) add(vc) can cause a superset to vc to be removed.
     *    2) this superset is then added to the log
     *    3) when reverting the log, we try to add the superset,
     *       but the add fails because vc is still in it.  
     *    4) Ooops!
     *
     *  This method does NO checks.  It simply adds vc into the
     *  proper position according to the ordering on the vcs.
     */
    void simple_add(const VC& vc);

    // @}

    //----------------------------------------------------------------------

    /** @name Iterators */
    // @{

    typedef std::list<VC>::iterator iterator;

    typedef std::list<VC>::const_iterator const_iterator;

    /** Returns an iterator to the given VC, or end() if vc is not in
        the list. Note that these methods are not constant, but as
        long as VCs are immutable (except for their processed flags)
        then the list cannot be altered by these methods. */
    iterator find(const VC& vc);
    iterator find(const VC& vc, const iterator& b, const iterator& e);

    /** Returns a constant iterator to the given VC, or end() if vc is
        not in the list. */
    const_iterator find(const VC& vc) const;
    const_iterator find(const VC& vc, 
                        const const_iterator& b, 
                        const const_iterator& e) const;


    /** Removes the element pointed to by i from the list.
        @return the next element. */
    iterator remove(iterator i, ChangeLog<VC>* log);

    /** Removes the given vc from the list.  Does nothing if vc is not
        actually in the list.  Takes O(n) time. Returns true if vc
        was actually removed, false otherwise. */
    bool remove(const VC& vc, ChangeLog<VC>* log);

    /** Returns an iterator to the start of the list. */
    iterator begin();

    /** Returns an iterator just past the end of the list. */
    iterator end();

    /** Returns a constant iterator to the start of the list. */
    const_iterator begin() const;

    /** Returns a constant iterator just past the end of the list. */
    const_iterator end() const;

    // @}

    //----------------------------------------------------------------------

    /** Returns the union of all carriers in the list. */
    bitset_t getUnion() const;

    /** Returns the union of all carriers in the list.
     
        Only includes carriers of VCs in the union if they shrink the
        intersection at each step (considered in the sorted order).
        
        @note Doing this may result in different sets of connections
        depending upon the relative order of the vcs in the list.  So,
        on an empty board, BLACK can have a slightly different
        connection set that WHITE.  Can also result in different vc
        connections between non-rotated and rotated versions of the
        board.
    */
    bitset_t getGreedyUnion() const;

    /** Returns soft-limit intersection. If list is empty, the bitset
        will have all of its bits set. */
    bitset_t softIntersection() const;

    /** Returns hard-limit intersection. If list is empty, the bitset
        will have all of its bits set. */
    bitset_t hardIntersection() const;

    //----------------------------------------------------------------------

    /** Removes all VCs that intersect with b.  Removed VCs are
        appended to out if out---note that the order of the vcs in out
        is the same as it was in the original list!  Returns number of
        vcs removed. */
    int removeAllContaining(HexPoint cell, std::list<VC>& out,
                            ChangeLog<VC>* log);

    int removeAllContaining(const bitset_t& b, std::list<VC>& out,
                            ChangeLog<VC>* log);

    int removeAllContaining(const bitset_t& b, ChangeLog<VC>* log);
   
    /** Performs list equality. */
    bool operator==(const VCList& other) const;

    /** Performs list inequality. */
    bool operator!=(const VCList& other) const;

protected:

    /** Invalidates the precomputed list unions. */
    void dirty_list_unions();

    /** Invalidates the list intersection. */
    void dirty_list_intersections();

    /** Computes list unions in one pass: normal and greedy. */
    void computeUnions() const;

    /** Computes list intersections in one pass: soft and hard. */
    void computeIntersections() const;

    HexPoint m_x, m_y;

    /** See softlimit() */
    unsigned int m_softlimit;

    std::list<VC> m_vcs;

    mutable bool m_dirty_intersection;
    mutable bitset_t m_soft_intersection;
    mutable bitset_t m_hard_intersection;

    mutable bool m_dirty_union;
    mutable bitset_t m_union;
    mutable bitset_t m_greedy_union;
};

inline HexPoint VCList::getX() const
{
    return m_x;
}

inline HexPoint VCList::getY() const
{
    return m_y;
}

inline int VCList::softlimit() const
{
    return m_softlimit;
}

inline void VCList::setSoftLimit(int limit)
{
    if (limit != (int)m_softlimit)
    {
        m_softlimit = limit;
        m_dirty_intersection = true;
    }
}

// FIXME: size() is might be O(n) for lists:
// keep track of size on our own?
inline int VCList::size() const
{
    return m_vcs.size();
}

inline bool VCList::empty() const
{
    return m_vcs.empty();
}

inline void VCList::dirty_list_unions()
{
    m_dirty_union = true;
}

inline void VCList::dirty_list_intersections()
{
    m_dirty_intersection = true;
}

inline void VCList::clear()
{
    m_vcs.clear();

    dirty_list_unions();
    
    m_dirty_intersection = false;
    m_soft_intersection.set();
    m_hard_intersection.set();
}

inline VCList::iterator VCList::begin()
{
    return m_vcs.begin();
}

inline VCList::iterator VCList::end()
{
    return m_vcs.end();
}

inline VCList::const_iterator VCList::begin() const
{
    return m_vcs.begin();
}

inline VCList::const_iterator VCList::end() const
{
    return m_vcs.end();
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // VCLIST_HPP
