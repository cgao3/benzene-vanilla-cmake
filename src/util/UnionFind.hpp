//----------------------------------------------------------------------------
/** @file UnionFind.hpp */
//----------------------------------------------------------------------------

#ifndef UNIONFIND_HPP
#define UNIONFIND_HPP

#include <vector>
#include "Benzene.hpp"
#include "BenzeneAssert.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** General union/find implementation. */
template<int S>
class UnionFind
{
public:

    /** Constructor.  All elements are initially isolated. */
    UnionFind();

    /** Sets all elements to be isolated. */
    void Clear();

    /** Returns true if x is the captain of a group. */
    bool IsRoot(int x) const;

    /** Gets the captain of x's group. */
    int GetRoot(int x) const;

    /** Unions two sets of elements. */
    int UnionGroups(int x, int y);

private:
    /** Flag denoting that the element is its own group. */
    static const int ISOLATED = -1;

    mutable std::vector<int> m_sets;
};

template<int S>
inline UnionFind<S>::UnionFind()
    : m_sets(S)
{
    Clear();
}

template<int S>
inline void UnionFind<S>::Clear()
{
    for (int i = 0; i < S; i++) 
        m_sets[i] = ISOLATED;
}

template<int S> 
inline bool UnionFind<S>::IsRoot(int x) const
{
    return m_sets[x] < 0;
}

template<int S>
inline int UnionFind<S>::GetRoot(int x) const
{
    BenzeneAssert(0 <= x && x < S);
    if (m_sets[x] < 0) 
        return x;
    return (m_sets[x] = GetRoot(m_sets[x]));
}

template<int S>
inline int UnionFind<S>::UnionGroups(int a, int b) 
{
    int ra = GetRoot(a);
    int rb = GetRoot(b);
    if (ra == rb) 
        return ra;
    // Force the smaller guy to become captain
    int cap = std::min(ra, rb);
    int non = std::max(ra, rb);
    m_sets[cap] += m_sets[non];
    m_sets[non] = cap; 
    return cap;
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // UNIONFIND_HPP
