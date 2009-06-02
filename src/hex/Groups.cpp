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
