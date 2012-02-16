//----------------------------------------------------------------------------
/** @file NeighborTracker.cpp */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "NeighborTracker.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

void NeighborTracker::Init(const Groups& groups)
{
    for (GroupIterator g(groups); g; ++g) 
        for (BitsetIterator m(g->Members()); m; ++m)
            m_groups.UnionGroups(*m, g->Captain());
    for (GroupIterator g(groups); g; ++g)
        m_empty_nbs[m_groups.GetRoot(g->Captain())] = g->Nbs(EMPTY_ONLY);
}

void NeighborTracker::Play(const HexColor color, const HexPoint x, 
                           const StoneBoard& brd)
{
    for (BoardIterator n(brd.Const().Nbs(x)); n; ++n) 
    {
        m_empty_nbs[m_groups.GetRoot(*n)].reset(x);
        if (brd.GetColor(*n) == color) 
        {
            HexPoint cn = static_cast<HexPoint>(m_groups.GetRoot(*n));
            HexPoint captain = static_cast<HexPoint>
                (m_groups.UnionGroups(x, cn));
            HexPoint other = (captain == x) ? cn : x;
            m_empty_nbs[captain] |= m_empty_nbs[other];
        }
    }
}

bitset_t NeighborTracker::Threats(const HexColor color) const
{
    HexPoint e1 = static_cast<HexPoint>
        (m_groups.GetRoot(HexPointUtil::colorEdge1(color)));
    HexPoint e2 = static_cast<HexPoint>
        (m_groups.GetRoot(HexPointUtil::colorEdge2(color)));
    return m_empty_nbs[e1] & m_empty_nbs[e2];
}

//----------------------------------------------------------------------------
