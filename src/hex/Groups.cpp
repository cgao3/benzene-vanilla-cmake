//----------------------------------------------------------------------------
/** @file Groups.cpp 
 */
//----------------------------------------------------------------------------

#include "BitsetIterator.hpp"
#include "Groups.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

namespace {

//----------------------------------------------------------------------------

void Flow(const StoneBoard& brd, HexPoint start, HexColor color, 
          bitset_t& members, bitset_t& nbs)
{
    HexAssert(!members.test(start));
    HexAssert(brd.getColor(start) == color);
    members.set(start);
    for (BoardIterator p(brd.Const().Nbs(start)); p; ++p)
    {
        if (members.test(*p))
            continue;
        if (color != EMPTY && brd.getColor(*p) == color)
            Flow(brd, *p, color, members, nbs);
        else
            nbs.set(*p);
    }
    HexAssert((members & nbs).none());
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
    for (BoardIterator p(brd.EdgesAndInterior()); p; ++p)
    {
        if (visited.test(*p))
            continue;
        bitset_t nbs, members;
        HexColor color = brd.getColor(*p);
        Flow(brd, *p, color, members, nbs);
        HexAssert((visited & members).none());
        visited |= members;
        for (BitsetIterator m(members); m; ++m)
            groups.m_group_index[*m] = groups.m_groups.size();
        groups.m_groups.push_back(Group(color, *p, members, nbs));
    }
    for (std::size_t i = 0; i < groups.m_groups.size(); ++i)
    {
        Group& g = groups.m_groups[i];
        for (BitsetIterator p(g.Nbs()); p; ++p)
        {
            const Group& nb = groups.GetGroup(*p);
            if (nb.Captain() != *p)
            {
                g.m_nbs.reset(*p);
                g.m_nbs.set(nb.Captain());
            }
        }
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

bitset_t Groups::Nbs(HexPoint point, HexColorSet colorset) const
{
    bitset_t ret;
    for (BitsetIterator p(Nbs(point)); p; ++p)
        if (HexColorSetUtil::InSet(GetGroup(*p).Color(), colorset))
            ret.set(*p);
    return ret;
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
    HexAssert(m_brd->Const().isLocation(locations));
    bitset_t captains;
    for (BitsetIterator i(locations); i; ++i)
        captains.set(CaptainOf(*i));
    return captains;
}

//----------------------------------------------------------------------------
