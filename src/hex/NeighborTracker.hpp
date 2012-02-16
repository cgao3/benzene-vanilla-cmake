//----------------------------------------------------------------------------
/** @file MoHexThreadState.hpp */
//----------------------------------------------------------------------------

#ifndef NEIGHBORTRACKER_HPP
#define NEIGHBORTRACKER_HPP

#include "BitsetIterator.hpp"
#include "HexColor.hpp"
#include "Groups.hpp"
#include "UnionFind.hpp"

_BEGIN_BENZENE_NAMESPACE_

class NeighborTracker
{
public:
    void Init(const Groups& groups);

    bool GameOver(const HexColor toPlay) const;

    void Play(const HexColor color, const HexPoint x, 
              const StoneBoard& brd);
    
    bitset_t Threats(const HexColor color) const;

private:
    bitset_t m_empty_nbs[BITSETSIZE];
    UnionFind<BITSETSIZE> m_groups;
};

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // NEIGHBORTRACKER_HPP
