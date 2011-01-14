//----------------------------------------------------------------------------
/** @file VCList.cpp */
//----------------------------------------------------------------------------

#include "VCList.hpp"
#include "ChangeLog.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

VCList::VCList(HexPoint x, HexPoint y, unsigned int soft)
    : m_x(x), m_y(y), 
      m_softlimit(soft),
      m_vcs(),
      m_dirtyIntersection(false),
      m_dirtyUnion(true)
{
    m_softIntersection.set();
    m_hardIntersection.set();
}

//----------------------------------------------------------------------------

std::string VCList::Dump() const
{
    std::size_t i = 0;
    std::ostringstream os;
    const_iterator it;
    for (it = m_vcs.begin(); it != m_vcs.end(); ++it) 
        os << i++ << ": " << *it << '\n';
    return os.str();
}

//----------------------------------------------------------------------------

bool VCList::IsSupersetOfAny(const bitset_t& bs) const
{
    for (const_iterator it = m_vcs.begin(); it != m_vcs.end(); ++it)
        if (BitsetUtil::IsSubsetOf(it->carrier(), bs))
            return true;
    return false;
}

bool VCList::IsSubsetOfAny(const bitset_t& bs) const
{
    for (const_iterator it = m_vcs.begin(); it != m_vcs.end(); ++it) 
        if (BitsetUtil::IsSubsetOf(bs, it->carrier())) 
            return true;
    return false;
}

int VCList::RemoveSuperSetsOf(const bitset_t& bs, ChangeLog<VC>* log,
                              bool dirtyIntersections)
{
    int count = 0;
    for (iterator it = m_vcs.begin(); it != m_vcs.end();)
    {
        if (BitsetUtil::IsSubsetOf(bs, it->carrier())) 
        {
            if (log) 
                log->push(ChangeLog<VC>::REMOVE, *it);
            it = m_vcs.erase(it);
            ++count;
        } 
        else 
            ++it;
    }
    if (count)
    {
        DirtyListUnions();
        if (dirtyIntersections)
            DirtyListIntersections();
    }
    return count;
}

//----------------------------------------------------------------------------

void VCList::SimpleAdd(const VC& vc)
{
    BenzeneAssert((vc.x() == GetX() && vc.y() == GetY()) ||
                  (vc.x() == GetY() && vc.y() == GetX()));
    iterator it;
    unsigned count = 0;
    for (it = m_vcs.begin(); it != m_vcs.end(); ++it, ++count)
        if (*it > vc)
            break;
    it = m_vcs.insert(it, vc);
    DirtyListUnions();
    if (count < m_softlimit)
        m_softIntersection &= vc.carrier();
    m_hardIntersection &= vc.carrier();
}

VCList::AddResult VCList::Add(const VC& vc, ChangeLog<VC>* log)
{
    BenzeneAssert((vc.x() == GetX() && vc.y() == GetY()) ||
                  (vc.x() == GetY() && vc.y() == GetX()));
    unsigned count = 0;
    iterator it = m_vcs.begin();
    for (; it != m_vcs.end(); ++it, ++count) 
    {
        if (*it > vc)
            break;
        if ((*it).isSubsetOf(vc))
            return ADD_FAILED;
    }
    if (log) 
        log->push(ChangeLog<VC>::ADD, vc);
    it = m_vcs.insert(it, vc);

    // update unions/intersections
    DirtyListUnions();
    if (count < m_softlimit) 
        m_softIntersection &= vc.carrier();
    m_hardIntersection &= vc.carrier();

    // remove supersets of vc
    for (++it; it != m_vcs.end();)
    {
        if (vc.isSubsetOf(*it)) 
        {
            if (log) 
                log->push(ChangeLog<VC>::REMOVE, *it);
            it = m_vcs.erase(it);
        } 
        else
            ++it;
    }
    return (count < m_softlimit) 
        ? ADDED_INSIDE_SOFT_LIMIT
        : ADDED_INSIDE_HARD_LIMIT;
}

int VCList::Add(const VCList& other, ChangeLog<VC>* log)
{
    int count = 0;
    for (const_iterator it = other.Begin(); it != other.End(); ++it) 
    {
        VC v(GetX(), GetY(), it->key(), it->carrier(), it->rule());
        v.setProcessed(false);
        if (this->Add(v, log))
            count++;
    }
    return count;
}

//----------------------------------------------------------------------------

VCList::iterator VCList::Remove(iterator it, ChangeLog<VC>* log)
{
    if (log) 
        log->push(ChangeLog<VC>::REMOVE, *it);
    it = m_vcs.erase(it);
    DirtyListUnions();
    DirtyListIntersections();
    return it;
}

bool VCList::Remove(const VC& vc, ChangeLog<VC>* log)
{
    iterator it = Find(vc);
    if (it == End())
        return false;
    Remove(it, log);
    return true;
}

//----------------------------------------------------------------------------

VCList::const_iterator VCList::Find(const VC& vc, const const_iterator& b, 
                                    const const_iterator& e) const
{
    const_iterator it = b;
    for (; it != e; ++it)
        if (vc == *it)      // TODO: Check size for speedup!
            break;
    return it;
}

VCList::const_iterator VCList::Find(const VC& vc) const
{
    return Find(vc, Begin(), End());
}

VCList::iterator VCList::Find(const VC& vc, const iterator& b, 
                              const iterator& e)
{
    iterator it = b;
    for (; it != e; ++it)
        if (vc == *it) 
            break;
    return it;
}

VCList::iterator VCList::Find(const VC& vc)
{
    return Find(vc, Begin(), End());
}

//----------------------------------------------------------------------------

void VCList::ComputeUnions() const
{
    bitset_t inter;
    inter.set();
    m_union.reset();
    m_greedyUnion.reset();
    const_iterator cur = m_vcs.begin();
    for (; cur != m_vcs.end(); ++cur) 
    {
        m_union |= cur->carrier();
        bitset_t c = inter & cur->carrier();
        if (inter != c) 
        {
            m_greedyUnion |= cur->carrier();
            inter = c;
        }
    }
    m_dirtyUnion = false;
}

bitset_t VCList::GetUnion() const
{
    if (m_dirtyUnion)
        ComputeUnions();
    return m_union;
}

bitset_t VCList::GetGreedyUnion() const
{
    if (m_dirtyUnion)
        ComputeUnions();
    return m_greedyUnion;
}

//----------------------------------------------------------------------------

void VCList::ComputeIntersections() const
{
    m_softIntersection.set();
    const_iterator cur = m_vcs.begin();
    const_iterator end = m_vcs.end();
    // TODO: Abort loops early if intersection is empty?
    for (std::size_t count = 0; count < Softlimit() && cur!=end; ++cur, ++count)
        m_softIntersection &= cur->carrier();
    m_hardIntersection = m_softIntersection;
    for (; cur != end; ++cur)
        m_hardIntersection &= cur->carrier();
    m_dirtyIntersection = false;
}

bitset_t VCList::SoftIntersection() const
{
    if (m_dirtyIntersection)
        ComputeIntersections();
    return m_softIntersection;
}

bitset_t VCList::HardIntersection() const
{
    if (m_dirtyIntersection)
        ComputeIntersections();
    return m_hardIntersection;
}

//----------------------------------------------------------------------------

std::size_t VCList::RemoveAllContaining(HexPoint cell, std::list<VC>& out,
                                        ChangeLog<VC>* log)
{
    if (!GetUnion().test(cell))
        return 0;
    int count = 0;
    for (iterator it = m_vcs.begin(); it != m_vcs.end(); ) 
    {
        if (it->carrier().test(cell)) 
        {
            out.push_back(*it);
            if (log) 
                log->push(ChangeLog<VC>::REMOVE, *it);
            it = m_vcs.erase(it);
            ++count;
        } 
        else
            ++it;
    }
    if (count > 0) 
    {
        DirtyListUnions();
        DirtyListIntersections();
    }
    return count;
}

std::size_t VCList::RemoveAllContaining(const bitset_t& b, std::list<VC>& out,
                                        ChangeLog<VC>* log)
{
    if ((GetUnion() & b).none())
        return 0;
    std::size_t count = 0;
    for (iterator it = m_vcs.begin(); it != m_vcs.end(); ) 
    {
        if ((it->carrier() & b).any()) 
        {
            out.push_back(*it);
            if (log)
                log->push(ChangeLog<VC>::REMOVE, *it);
            it = m_vcs.erase(it);
            ++count;
        }
        else
            ++it;
    }
    if (count > 0)
    {
        DirtyListUnions();
        DirtyListIntersections();
    }
    return count;
}

std::size_t VCList::RemoveAllContaining(const bitset_t& b, ChangeLog<VC>* log)
{
    if ((GetUnion() & b).none())
        return 0;
    std::size_t count = 0;
    for (iterator it = m_vcs.begin(); it != m_vcs.end(); ) 
    {
        if ((it->carrier() & b).any()) 
        {
            if (log)
                log->push(ChangeLog<VC>::REMOVE, *it);
            it = m_vcs.erase(it);
            ++count;
        } 
        else
            ++it;
    }
    if (count > 0)
    {
        DirtyListUnions();
        DirtyListIntersections();
    }
    return count;
}

//----------------------------------------------------------------------------

bool VCList::operator==(const VCList& other) const
{
    if (m_softlimit != other.m_softlimit)
        return false;
    if (Size() != other.Size())
        return false;
    const_iterator us = Begin();
    const_iterator them = other.Begin();
    while (us != End()) 
    {
        if (*us != *them) 
            return false;
        if (us->processed() != them->processed()) 
            return false;
        ++us;
        ++them;
    }
    return true;
}

bool VCList::operator!=(const VCList& other) const
{
    if (m_softlimit != other.m_softlimit)
        return true;
    if (Size() != other.Size())
        return true;
    const_iterator us = Begin();
    const_iterator them = other.Begin();
    while (us != End()) 
    {
        if (*us != *them) 
            return true;
        if (us->processed() != them->processed()) 
            return true;
        ++us;
        ++them;
    }
    return false;
}

//----------------------------------------------------------------------------
