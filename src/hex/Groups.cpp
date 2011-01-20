//----------------------------------------------------------------------------
/** @file Groups.cpp  */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "BitsetIterator.hpp"
#include "Groups.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

namespace {

//----------------------------------------------------------------------------

void Flow(const StoneBoard& brd, HexPoint start, HexColor color, 
          bitset_t& members, bitset_t& nbs)
{
    BenzeneAssert(!members.test(start));
    BenzeneAssert(brd.GetColor(start) == color);
    members.set(start);
    for (BoardIterator p(brd.Const().Nbs(start)); p; ++p)
    {
        if (members.test(*p))
            continue;
        if (color != EMPTY && brd.GetColor(*p) == color)
            Flow(brd, *p, color, members, nbs);
        else
            nbs.set(*p);
    }
    BenzeneAssert((members & nbs).none());
}

//----------------------------------------------------------------------------

} // anonymous namespace

//----------------------------------------------------------------------------

void GroupBuilder::Build(const StoneBoard& brd, Groups& groups)
{
    bitset_t visited;
    groups.m_brd = const_cast<StoneBoard*>(&brd);
    groups.m_groups.clear();
    groups.m_group_index.resize(FIRST_INVALID);
    for (BoardIterator p(brd.Const().EdgesAndInterior()); p; ++p)
    {
        if (visited.test(*p))
            continue;
        bitset_t nbs, members;
        HexColor color = brd.GetColor(*p);
        Flow(brd, *p, color, members, nbs);
        BenzeneAssert((visited & members).none());
        visited |= members;
        for (BitsetIterator m(members); m; ++m)
            groups.m_group_index[*m] = groups.m_groups.size();
        groups.m_groups.push_back(Group(&groups, color, *p, members, nbs));
    }
    for (std::size_t i = 0; i < groups.m_groups.size(); ++i)
    {
        Group& g = groups.m_groups[i];
        g.m_nbs = groups.CaptainizeBitset(g.m_nbs);
        for (BitsetIterator p(g.m_nbs); p; ++p)
            g.m_nbs_index.push_back(groups.m_group_index[*p]);
    }
}

//----------------------------------------------------------------------------

std::size_t Groups::NumGroups(HexColorSet colorset) const
{
    std::size_t num = 0;
    for (GroupIterator g(*this, colorset); g; ++g)
        ++num;
    return num;
}

std::size_t Groups::GroupIndex(HexPoint point, HexColorSet colorset) const
{
    std::size_t count = 0;
    for (GroupIterator g(*this, colorset); g; ++g)
    {
        if (g->IsMember(point))
            break;
        ++count;
    }
    return count;
}

bool Groups::IsGameOver() const
{
    return (GetWinner() != EMPTY);
}

HexColor Groups::GetWinner() const
{
    for (BWIterator c; c; ++c) 
        if (m_group_index[HexPointUtil::colorEdge1(*c)] ==
            m_group_index[HexPointUtil::colorEdge2(*c)])
            return *c;
    return EMPTY;
}

bitset_t Groups::CaptainizeBitset(bitset_t locations) const
{
    BenzeneAssert(m_brd->Const().IsLocation(locations));
    bitset_t captains;
    for (BitsetIterator i(locations); i; ++i)
        captains.set(CaptainOf(*i));
    return captains;
}

//----------------------------------------------------------------------------

const bitset_t& Group::Nbs(HexColorSet colorset) const
{
    if (!m_colorsets_computed)
    {
        ComputeColorsetNbs();
        m_colorsets_computed = true;
    }
    return m_nbs_colorset[colorset];
}

void Group::ComputeColorsetNbs() const
{
    for (int cs = 0; cs < NUM_COLOR_SETS; ++cs) 
    {
        HexColorSet colorset = static_cast<HexColorSet>(cs);
        for (std::size_t i = 0; i < m_nbs_index.size(); ++i)
        {
            const Group* nb = &m_groups->m_groups[m_nbs_index[i]];
            if (HexColorSetUtil::InSet(nb->Color(), colorset))
                m_nbs_colorset[colorset].set(nb->Captain());
        }
    }
}

//----------------------------------------------------------------------------
