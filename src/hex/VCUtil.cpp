//----------------------------------------------------------------------------
/** @file VCUtil.cpp */
//----------------------------------------------------------------------------

#include "VCUtil.hpp"
#include "VCS.hpp"
#include "HexBoard.hpp"
#include "BoardUtil.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

bitset_t VCUtil::GetMustplay(const HexBoard& brd, HexColor color)
{
    HexColor other = !color;
    if (brd.Cons(other).FullExists())
        return EMPTY_BITSET;
    else
        return brd.GetPosition().GetEmpty() & brd.Cons(other).SemiIntersection();
}

//----------------------------------------------------------------------------

bool VCUtil::ValidEdgeBridge(const StoneBoard& brd, const bitset_t& carrier, 
                             HexPoint& endpoint, HexPoint& edge)
{
    if (carrier.count() != 2) 
        return false;
    if ((brd.GetOccupied() & carrier).any()) 
        return false;
    std::vector<HexPoint> miai;
    BitsetUtil::BitsetToVector(carrier, miai);
    if (!brd.Const().Adjacent(miai[0], miai[1])) 
        return false;
    // Find the two cells adjacent to both
    std::vector<HexPoint> adj;
    for (BoardIterator n1(brd.Const().Nbs(miai[0])); n1; ++n1)
        for (BoardIterator n2(brd.Const().Nbs(miai[1])); n2; ++n2)
            if (*n1 == *n2) 
                adj.push_back(*n1);
    BenzeneAssert(adj.size() == 2);
    // Pick the edge and endpoint
    for (std::size_t i = 0; i < 2; ++i) 
    {
        if (HexPointUtil::isEdge(adj[i])) 
        {
            edge = adj[i];
            endpoint = adj[i^1];
            return true;
        }
    }
    // Did not touch an edge, so return false
    return false;
}

//----------------------------------------------------------------------------

namespace {

/** Tries to find a flaring move around probe.
    For all full nbs z of x: find a semi between z and y
    that does not hit probe and does not touch sxy or fxz. */
void FlareUsingSemi(const VCS& vcs, const HexPoint x, const HexPoint y,
                    const HexPoint probe, const bitset_t sxy,
                    bitset_t& flares)
{
    for (BitsetIterator z(vcs.GetFullNbs(x)); z; ++z)
    {
        if (*z == probe || *z == y || flares.test(*z))
            continue;
        // FIXME: too lazy?
        // Should we look through list for fxz that don't touch sxy?
        const bitset_t fxz = vcs.FullGreedyUnion(x, *z);
        if (fxz.test(probe) || ((fxz & sxy).any()))
            continue;
        
        for (CarrierList::Iterator s2(vcs.GetSemiCarriers(*z, y)); s2; ++s2)
        {
            bitset_t szy = s2.Carrier();
            if (!szy.test(probe) 
                && ((szy & fxz).none())
                && ((szy & sxy).none()))
            {
                // found a flare!!!
                flares.set(*z);
#if 0
                LogInfo() << "FLARE xzy " 
                          << " x=" << x << " y=" << y
                          << " p=" << probe << " z=" << *z 
                          << '\n';
#endif
                break;
            }
        }
    }
}

} // anonymous namespace

void VCUtil::RespondToProbe(const HexBoard& vcbrd, const HexColor toPlay, 
                            const HexPoint probe, bitset_t& responses)
{
    const HexColor opp = !toPlay;
    const VCS& vcs = vcbrd.Cons(opp);
    const Groups& groups = vcbrd.GetGroups();
    for (GroupIterator xg(groups, opp); xg; ++xg)
    {
        const HexPoint x = xg->Captain();
        for (GroupIterator yg(groups, opp); 
             yg->Captain() != xg->Captain(); ++yg)
        {
            const HexPoint y = yg->Captain();
            if (!vcs.FullExists(x, y))
                continue;
            if (!vcs.FullIntersection(x, y).test(probe))
                continue;
            for (CarrierList::Iterator s(vcs.GetSemiCarriers(x, y)); s; ++s)
            {
                const bitset_t sxy = s.Carrier();
                if (sxy.test(probe))
                    continue;
                // Semi's key is a direct response                    
                const HexPoint key = vcs.SemiKey(sxy, x, y);
                if (key != INVALID_POINT)
                    responses.set(key);
                // Try to restore the connection by finding disjoint
                // semis to either side of the probe
                FlareUsingSemi(vcs, x, y, probe, sxy, responses);
                FlareUsingSemi(vcs, y, x, probe, sxy, responses);
            }
        }
    }
}

//----------------------------------------------------------------------------
