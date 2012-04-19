//----------------------------------------------------------------------------
/** @file NeighborTracker.cpp */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "NeighborTracker.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

void NeighborTracker::Init(const Groups& groups)
{
    m_groups.Clear();
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
            HexPoint cx = static_cast<HexPoint>(m_groups.GetRoot(x));
            HexPoint captain = static_cast<HexPoint>
                (m_groups.UnionGroups(cx, cn));
            HexPoint other = (captain == cx) ? cn : cx;
            m_empty_nbs[captain] |= m_empty_nbs[other];
        }
    }
}

HexColor NeighborTracker::GetWinner() const
{
    if (m_groups.GetRoot(HexPointUtil::colorEdge1(BLACK)) == 
        m_groups.GetRoot(HexPointUtil::colorEdge2(BLACK)))
        return BLACK;
    if (m_groups.GetRoot(HexPointUtil::colorEdge1(WHITE)) == 
        m_groups.GetRoot(HexPointUtil::colorEdge2(WHITE)))
        return WHITE;
    return EMPTY;
}

bool NeighborTracker::GameOver() const
{
    return GetWinner() != EMPTY;
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
