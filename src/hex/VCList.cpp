//----------------------------------------------------------------------------
/** @file
 */
//----------------------------------------------------------------------------

#include "VCList.hpp"
#include "ChangeLog.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

VCList::VCList(HexPoint x, HexPoint y, unsigned int soft)
    : m_x(x), m_y(y), 
      m_softlimit(soft),
      m_vcs(),
      m_dirty_intersection(false),
      m_dirty_union(true)
{
    m_soft_intersection.set();
    m_hard_intersection.set();
}

//----------------------------------------------------------------------------

std::string VCList::dump() const
{
    int i=0;
    std::ostringstream os;
    const_iterator it, end = m_vcs.end();
    for (it = m_vcs.begin(); it != end; ++it) {
        os << i++ << ": ";
        os << *it;
        os << std::endl;
    }
    return os.str();
}

//----------------------------------------------------------------------------

bool VCList::isSupersetOfAny(const bitset_t& bs) const
{
    for (const_iterator it = m_vcs.begin(); it != m_vcs.end(); ++it)
        if (BitsetUtil::IsSubsetOf(it->carrier(), bs))
            return true;
    return false;
}

bool VCList::isSubsetOfAny(const bitset_t& bs) const
{
    for (const_iterator it = m_vcs.begin(); it != m_vcs.end(); ++it) 
        if (BitsetUtil::IsSubsetOf(bs, it->carrier())) 
            return true;
    return false;
}

int VCList::removeSuperSetsOf(const bitset_t& bs, ChangeLog<VC>* log,
                              bool dirty_intersections)
{
    int count = 0;
    for (iterator it = m_vcs.begin(); it != m_vcs.end(); ) 
    {
        if (BitsetUtil::IsSubsetOf(bs, it->carrier())) 
        {
            if (log) log->push(ChangeLog<VC>::REMOVE, *it);
            it = m_vcs.erase(it);
            count++;
        } 
        else 
            ++it;
    }
    if (count) 
    {
        dirty_list_unions();
        if (dirty_intersections)
            dirty_list_intersections();
    }
    return count;
}

//----------------------------------------------------------------------------

void VCList::simple_add(const VC& vc)
{
    HexAssert((vc.x() == getX() && vc.y() == getY()) ||
              (vc.x() == getY() && vc.y() == getX()));

    iterator it;
    unsigned count = 0;
    for (it = m_vcs.begin(); it != m_vcs.end(); ++it, ++count) {
        if (*it > vc)
            break;
    }
    it = m_vcs.insert(it, vc);

    // update unions/intersections
    dirty_list_unions();
    if (count < m_softlimit) m_soft_intersection &= vc.carrier();
    m_hard_intersection &= vc.carrier();
}

VCList::AddResult VCList::add(const VC& vc, ChangeLog<VC>* log)
{
    HexAssert((vc.x() == getX() && vc.y() == getY()) ||
              (vc.x() == getY() && vc.y() == getX()));

    unsigned count = 0;
    iterator it = m_vcs.begin();
    for (; it != m_vcs.end(); ++it, ++count) 
    {
        if (*it > vc)
            break;

        if ((*it).isSubsetOf(vc))
            return ADD_FAILED;
    }

    if (log) log->push(ChangeLog<VC>::ADD, vc);
    it = m_vcs.insert(it, vc);

    // update unions/intersections
    dirty_list_unions();
    if (count < m_softlimit) m_soft_intersection &= vc.carrier();
    m_hard_intersection &= vc.carrier();

    // remove supersets of vc
    for (++it; it != m_vcs.end(); ) 
    {
        if (vc.isSubsetOf(*it)) 
        {
            if (log) log->push(ChangeLog<VC>::REMOVE, *it);
            it = m_vcs.erase(it);

        } else {
            ++it;
        }
    }

    return (count < m_softlimit) ? 
        ADDED_INSIDE_SOFT_LIMIT:
        ADDED_INSIDE_HARD_LIMIT;
}

int VCList::add(const VCList& other, ChangeLog<VC>* log)
{
    int count = 0;
    for (const_iterator it = other.begin(); it != other.end(); ++it) 
    {
        VC v(getX(), getY(), it->key(), it->carrier(), 
             it->stones(), it->rule());
        v.setProcessed(false);
        if (this->add(v, log))
            count++;
    }
    return count;
}

//----------------------------------------------------------------------------

VCList::iterator VCList::remove(iterator it, ChangeLog<VC>* log)
{
    if (log) log->push(ChangeLog<VC>::REMOVE, *it);
    it = m_vcs.erase(it);

    dirty_list_unions();
    dirty_list_intersections();
    return it;
}

bool VCList::remove(const VC& vc, ChangeLog<VC>* log)
{
    iterator it = find(vc);
    if (it == end())
        return false;

    remove(it, log);
    return true;
}

//----------------------------------------------------------------------------

VCList::const_iterator VCList::find(const VC& vc, 
                                    const const_iterator& b, 
                                    const const_iterator& e) const
{
    const_iterator it = b;
    for (; it != e; ++it) {
        if (vc == *it)      // @todo check size for speedup!
            break;
    }
    return it;
}

VCList::const_iterator VCList::find(const VC& vc) const
{
    return find(vc, begin(), end());
}

VCList::iterator VCList::find(const VC& vc, 
                              const iterator& b, 
                              const iterator& e)
{
    iterator it = b;
    for (; it != e; ++it) {
        if (vc == *it) 
            break;
    }
    return it;
}

VCList::iterator VCList::find(const VC& vc)
{
    return find(vc, begin(), end());
}

//----------------------------------------------------------------------------

void VCList::computeUnions() const
{
    bitset_t inter;
    inter.set();
    m_union.reset();
    m_greedy_union.reset();

    const_iterator cur = m_vcs.begin();
    const_iterator end = m_vcs.end();
    for (; cur!=end; ++cur) {
        m_union |= cur->carrier();
        bitset_t c = inter & cur->carrier();
        if (inter != c) {
            m_greedy_union |= cur->carrier();
            inter = c;
        }
    }
    m_dirty_union = false;
}

bitset_t VCList::getUnion() const
{
    if (m_dirty_union) {
        computeUnions();
    }
    return m_union;
}

bitset_t VCList::getGreedyUnion() const
{
    if (m_dirty_union) {
        computeUnions();
    }
    return m_greedy_union;
}

//----------------------------------------------------------------------------

void VCList::computeIntersections() const
{
    m_soft_intersection.set();
    const_iterator cur = m_vcs.begin();
    const_iterator end = m_vcs.end();

    // @todo abort loops early if intersection is empty?
    for (int count=0; count<softlimit() && cur!=end; ++cur, ++count) {
        m_soft_intersection &= cur->carrier();
    }
    m_hard_intersection = m_soft_intersection;

    for (; cur != end; ++cur) {
        m_hard_intersection &= cur->carrier();
    }

    m_dirty_intersection = false;
}

bitset_t VCList::softIntersection() const
{
    if (m_dirty_intersection) {
        computeIntersections();
    }
    return m_soft_intersection;
}

bitset_t VCList::hardIntersection() const
{
    if (m_dirty_intersection) {
        computeIntersections();
    }
    return m_hard_intersection;
}

//----------------------------------------------------------------------------

int VCList::removeAllContaining(HexPoint cell, std::list<VC>& out,
                                ChangeLog<VC>* log)
{
    if (!getUnion().test(cell))
        return 0;

    int count = 0;
    for (iterator it = m_vcs.begin(); it != m_vcs.end(); ) {
        if (it->carrier().test(cell)) {
            out.push_back(*it);
            if (log) log->push(ChangeLog<VC>::REMOVE, *it);
            it = m_vcs.erase(it);
            ++count;
        } else {
            ++it;
        }
    }

    if (count) {
        dirty_list_unions();
        dirty_list_intersections();
    }
    return count;
}

int VCList::removeAllContaining(const bitset_t& b, std::list<VC>& out,
                                ChangeLog<VC>* log)
{
    if ((getUnion() & b).none())
        return 0;

    int count = 0;
    for (iterator it = m_vcs.begin(); it != m_vcs.end(); ) {
        if ((it->carrier() & b).any()) {
            out.push_back(*it);
            if (log) log->push(ChangeLog<VC>::REMOVE, *it);
            it = m_vcs.erase(it);
            ++count;
        } else {
            ++it;
        }
    }

    if (count) {
        dirty_list_unions();
        dirty_list_intersections();
    }
    return count;
}

int VCList::removeAllContaining(const bitset_t& b, ChangeLog<VC>* log)
{
    if ((getUnion() & b).none())
        return 0;

    int count = 0;
    for (iterator it = m_vcs.begin(); it != m_vcs.end(); ) {
        if ((it->carrier() & b).any()) {
            if (log) log->push(ChangeLog<VC>::REMOVE, *it);
            it = m_vcs.erase(it);
            ++count;
        } else {
            ++it;
        }
    }

    if (count) {
        dirty_list_unions();
        dirty_list_intersections();
    }
    return count;
}

//----------------------------------------------------------------------------

bool VCList::operator==(const VCList& other) const
{
    if (m_softlimit != other.m_softlimit)
        return false;
    if (size() != other.size())
        return false;

    const_iterator us = begin();
    const_iterator them = other.begin();
    while (us != end()) {
        if (*us != *them) return false;
        if (us->processed() != them->processed()) return false;
        ++us;
        ++them;
    }
    return true;
}

bool VCList::operator!=(const VCList& other) const
{
    if (size() != other.size())
        return true;

    const_iterator us = begin();
    const_iterator them = other.begin();
    while (us != end()) {
        if (*us != *them) return true;
        if (us->processed() != them->processed()) return true;
        ++us;
        ++them;
    }
    return false;
}

//----------------------------------------------------------------------------
