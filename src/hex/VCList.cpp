//----------------------------------------------------------------------------
/** @file VCList.cpp */
//----------------------------------------------------------------------------

#include "VCList.hpp"
#include "ChangeLog.hpp"
#include <algorithm>

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
    for (std::size_t i = 0; i < m_vcs.size(); ++i)
        if (BitsetUtil::IsSubsetOf(m_vcs[i].Carrier(), bs))
        {
            // Move to front
            VC v = m_vcs[i];
            for (std::size_t j = i; j > 0; j--)
                m_vcs[j] = m_vcs[j - 1];
            m_vcs[0] = v;
            return true;
        }
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
    std::size_t j = 0;
    std::size_t i = 0;
    for (; i < m_vcs.size(); ++i)
    {
        if (BitsetUtil::IsSubsetOf(bs, m_vcs[i].Carrier()))
        {
            if (log) 
                log->Push(ChangeLog<VC>::REMOVE, m_vcs[i]);
        } 
        else
            m_vcs[j++] = m_vcs[i];
    }
    m_vcs.resize(j);
    std::size_t count = i - j;
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
    size_t count = m_vcs.size();
    m_vcs.push_back(vc);
    DirtyListUnions();
    if (count < m_softlimit)
        m_softIntersection &= vc.Carrier();
    m_hardIntersection &= vc.Carrier();
}

VCList::AddResult VCList::Add(const VC& vc, ChangeLog<VC>* log)
{
    BenzeneAssert((vc.X() == GetX() && vc.Y() == GetY()) ||
                  (vc.X() == GetY() && vc.Y() == GetX()));
    if (IsSupersetOfAny(vc.Carrier()))
        return ADD_FAILED;
    if (log)
        log->Push(ChangeLog<VC>::ADD, vc);
    RemoveSuperSetsOf(vc.Carrier(), log);
    std::size_t count = m_vcs.size();
    m_vcs.push_back(vc);
    DirtyListUnions();
    if (!m_dirtyIntersection)
    {
        if (count < m_softlimit)
            m_softIntersection &= vc.Carrier();
        m_hardIntersection &= vc.Carrier();
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
        if (this->Add(v, log))
            count++;
    }
    return count;
}

//----------------------------------------------------------------------------

bool VCList::Remove(const VC& vc, ChangeLog<VC>* log)
{
    for (std::size_t i = 0; i < m_vcs.size(); ++i)
        if (vc == m_vcs[i])
        {
            if (log)
                log->Push(ChangeLog<VC>::REMOVE, m_vcs[i]);
            for (std::size_t j = i + 1; j < m_vcs.size(); ++j)
                m_vcs[j - 1] = m_vcs[j];
            m_vcs.pop_back();
            DirtyListUnions();
            DirtyListIntersections();
            return true;
        }
    return false;
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
    std::size_t i = 0;
    for (; i < std::min(Softlimit(), m_vcs.size()); ++i)
    {
        m_softIntersection &= m_vcs[i].Carrier();
        if (m_softIntersection.none())
        {
            m_hardIntersection.reset();
            m_dirtyIntersection = false;
            return;
        }
    }
    m_hardIntersection = m_softIntersection;
    for (; i < m_vcs.size(); ++i)
    {
        m_hardIntersection &= m_vcs[i].Carrier();
        if (m_hardIntersection.none())
            break;
    }
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

std::size_t VCList::RemoveAllContaining(HexPoint cell, std::vector<VC>& out,
                                        ChangeLog<VC>* log)
{
    if (!GetUnion().test(cell))
        return 0;
    std::size_t j = 0;
    std::size_t i = 0;
    for (; i < m_vcs.size(); ++i)
    {
        if (m_vcs[i].Carrier().test(cell))
        {
            out.push_back(m_vcs[i]);
            if (log) 
                log->Push(ChangeLog<VC>::REMOVE, m_vcs[i]);
        } 
        else
            m_vcs[j++] = m_vcs[i];
    }
    m_vcs.resize(j);
    size_t count = i - j;
    if (count > 0) 
    {
        DirtyListUnions();
        DirtyListIntersections();
    }
    return count;
}

std::size_t VCList::RemoveAllContaining(const bitset_t& b, std::vector<VC>& out,
                                        ChangeLog<VC>* log)
{
    if ((GetUnion() & b).none())
        return 0;
    std::size_t j = 0;
    std::size_t i = 0;
    for (; i < m_vcs.size(); ++i)
    {
        if ((m_vcs[i].Carrier() & b).any())
        {
            out.push_back(m_vcs[i]);
            if (log)
                log->Push(ChangeLog<VC>::REMOVE, m_vcs[i]);
        }
        else
            m_vcs[j++] = m_vcs[i];
    }
    m_vcs.resize(j);
    size_t count = i - j;
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
    std::size_t j = 0;
    std::size_t i = 0;
    for (; i < m_vcs.size(); ++i)
    {
        if ((m_vcs[i].Carrier() & b).any())
        {
            if (log)
                log->Push(ChangeLog<VC>::REMOVE, m_vcs[i]);
        }
        else
            m_vcs[j++] = m_vcs[i];
    }
    m_vcs.resize(j);
    size_t count = i - j;
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
    std::vector<VC> us(m_vcs);
    std::sort(us.begin(), us.end());
    std::vector<VC> them(other.m_vcs);
    std::sort(them.begin(), them.end());
    for (std::size_t i = 0; i < us.size(); ++i)
    {
        if (us[i] != them[i])
            return false;
        if (us[i].Processed() != them[i].Processed())
            return false;
    }
    return true;
}

bool VCList::operator!=(const VCList& other) const
{
    if (m_softlimit != other.m_softlimit)
        return true;
    if (Size() != other.Size())
        return true;
    std::vector<VC> us(m_vcs);
    std::sort(us.begin(), us.end());
    std::vector<VC> them(other.m_vcs);
    std::sort(them.begin(), them.end());
    for (std::size_t i = 0; i < us.size(); ++i)
    {
        if (us[i] != them[i])
            return true;
        if (us[i].Processed() != them[i].Processed())
            return true;
    }
    return false;
}

//----------------------------------------------------------------------------
