//----------------------------------------------------------------------------
/** @file VCList.cpp */
//----------------------------------------------------------------------------

#include "VCList.hpp"
#include "ChangeLog.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

VCList::VCList(HexPoint x, HexPoint y, std::size_t soft)
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
    for (VCListConstIterator it(*this); it; ++it, ++i)
        os << i++ << ": " << *it << '\n';
    return os.str();
}

//----------------------------------------------------------------------------

bool VCList::IsSupersetOfAny(const bitset_t& bs) const
{
    for (VCListConstIterator it(*this); it; ++it)
        if (BitsetUtil::IsSubsetOf(it->Carrier(), bs))
            return true;
    return false;
}

bool VCList::IsSubsetOfAny(const bitset_t& bs) const
{
    for (VCListConstIterator it(*this); it; ++it) 
        if (BitsetUtil::IsSubsetOf(bs, it->Carrier())) 
            return true;
    return false;
}

std::size_t VCList::RemoveSuperSetsOf(const bitset_t& bs, ChangeLog<VC>* log,
                                      bool dirtyIntersections)
{
    std::size_t count = 0;
    for (std::list<VC>::iterator it = m_vcs.begin(); it != m_vcs.end();)
    {
        if (BitsetUtil::IsSubsetOf(bs, it->Carrier())) 
        {
            if (log) 
                log->Push(ChangeLog<VC>::REMOVE, *it);
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

void VCList::ForcedAdd(const VC& vc)
{
    BenzeneAssert((vc.X() == GetX() && vc.Y() == GetY()) ||
                  (vc.X() == GetY() && vc.Y() == GetX()));
    std::list<VC>::iterator it;
    unsigned count = 0;
    for (it = m_vcs.begin(); it != m_vcs.end(); ++it, ++count)
        if (*it > vc)
            break;
    it = m_vcs.insert(it, vc);
    DirtyListUnions();
    if (count < m_softlimit)
        m_softIntersection &= vc.Carrier();
    m_hardIntersection &= vc.Carrier();
}

VCList::AddResult VCList::Add(const VC& vc, ChangeLog<VC>* log)
{
    BenzeneAssert((vc.X() == GetX() && vc.Y() == GetY()) ||
                  (vc.X() == GetY() && vc.Y() == GetX()));
    unsigned count = 0;
    std::list<VC>::iterator it = m_vcs.begin();
    for (; it != m_vcs.end(); ++it, ++count) 
    {
        if (*it > vc)
            break;
        if ((*it).IsSubsetOf(vc))
            return ADD_FAILED;
    }
    if (log) 
        log->Push(ChangeLog<VC>::ADD, vc);
    it = m_vcs.insert(it, vc);
    DirtyListUnions();
    if (count < m_softlimit) 
        m_softIntersection &= vc.Carrier();
    m_hardIntersection &= vc.Carrier();
    // Remove supersets of vc
    for (++it; it != m_vcs.end();)
    {
        if (vc.IsSubsetOf(*it)) 
        {
            if (log) 
                log->Push(ChangeLog<VC>::REMOVE, *it);
            it = m_vcs.erase(it);
        } 
        else
            ++it;
    }
    return (count < m_softlimit) 
        ? ADDED_INSIDE_SOFT_LIMIT
        : ADDED_INSIDE_HARD_LIMIT;
}

std::size_t VCList::Add(const VCList& other, ChangeLog<VC>* log)
{
    std::size_t count = 0;
    for (VCListConstIterator it(other); it; ++it) 
    {
        VC v(GetX(), GetY(), it->Key(), it->Carrier(), it->Rule());
        v.SetProcessed(false);
        if (this->Add(v, log))
            count++;
    }
    return count;
}

//----------------------------------------------------------------------------

bool VCList::Remove(const VC& vc, ChangeLog<VC>* log)
{
    std::list<VC>::iterator it = m_vcs.begin();
    for (; it != m_vcs.end(); ++it)
        if (vc == *it) 
            break;
    if (it == m_vcs.end())
        return false;
    if (log) 
        log->Push(ChangeLog<VC>::REMOVE, *it);
    it = m_vcs.erase(it);
    DirtyListUnions();
    DirtyListIntersections();
    return true;
}

//----------------------------------------------------------------------------

VC* VCList::FindInList(const VC& vc)
{
    for (VCListIterator it(*this); it; ++it)
        if (*it == vc)
            return &(*it);
    return 0;
}

//----------------------------------------------------------------------------

void VCList::ComputeUnions() const
{
    bitset_t inter;
    inter.set();
    m_union.reset();
    m_greedyUnion.reset();
    for (VCListConstIterator it(*this); it; ++it) 
    {
        m_union |= it->Carrier();
        bitset_t c = inter & it->Carrier();
        if (inter != c) 
        {
            m_greedyUnion |= it->Carrier();
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
    std::list<VC>::const_iterator cur = m_vcs.begin();
    std::list<VC>::const_iterator end = m_vcs.end();
    // TODO: Abort loops early if intersection is empty?
    for (std::size_t count = 0; count < Softlimit() && cur!=end; ++cur, ++count)
        m_softIntersection &= cur->Carrier();
    m_hardIntersection = m_softIntersection;
    for (; cur != end; ++cur)
        m_hardIntersection &= cur->Carrier();
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
    for (std::list<VC>::iterator it = m_vcs.begin(); it != m_vcs.end(); ) 
    {
        if (it->Carrier().test(cell)) 
        {
            out.push_back(*it);
            if (log) 
                log->Push(ChangeLog<VC>::REMOVE, *it);
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
    for (std::list<VC>::iterator it = m_vcs.begin(); it != m_vcs.end(); ) 
    {
        if ((it->Carrier() & b).any()) 
        {
            out.push_back(*it);
            if (log)
                log->Push(ChangeLog<VC>::REMOVE, *it);
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
    for (std::list<VC>::iterator it = m_vcs.begin(); it != m_vcs.end(); ) 
    {
        if ((it->Carrier() & b).any()) 
        {
            if (log)
                log->Push(ChangeLog<VC>::REMOVE, *it);
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
    VCListConstIterator us(*this);
    VCListConstIterator them(other);
    while (us)
    {
        if (*us != *them) 
            return false;
        if (us->Processed() != them->Processed()) 
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
    VCListConstIterator us(*this);
    VCListConstIterator them(other);
    while (us) 
    {
        if (*us != *them) 
            return true;
        if (us->Processed() != them->Processed()) 
            return true;
        ++us;
        ++them;
    }
    return false;
}

//----------------------------------------------------------------------------
