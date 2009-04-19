//----------------------------------------------------------------------------
/** @file UnionFind.hpp
 */
//----------------------------------------------------------------------------

#ifndef UNIONFIND_HPP
#define UNIONFIND_HPP

#include <cassert>
#include <vector>
#include "Benzene.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** General union/find implementation. 
    
    @todo instead of storing the group size, store the max depth of any
    element of the group, as per CLRS?
*/
template<int S>
class UnionFind
{
public:

    /** Constructor.  All elements are initially isolated. */
    UnionFind();

    /** Sets all elements to be isolated. */
    void clear();

    /** Returns true if x is the captain of a group. */
    bool isRoot(int x) const;

    /** Gets the captain of x's group. */
    int getRoot(int x) const;

    /** Unions two sets of elements. */
    int unionGroups(int x, int y);

private:
    /** Flag denoting that the element is its own group. */
    static const int ISOLATED = -1;

    mutable std::vector<int> m_sets;
};

template<int S>
inline UnionFind<S>::UnionFind()
    : m_sets(S)
{
    clear();
}

template<int S>
inline void UnionFind<S>::clear()
{
    for (int i=0; i<S; i++) 
        m_sets[i] = ISOLATED;
}

template<int S> 
inline bool UnionFind<S>::isRoot(int x) const
{
    return (m_sets[x] < 0);
}

template<int S>
inline int UnionFind<S>::getRoot(int x) const
{
    assert(0 <= x && x < S);
    if (m_sets[x] < 0) 
        return x;
    return (m_sets[x] = getRoot(m_sets[x]));
}

template<int S>
inline int UnionFind<S>::unionGroups(int a, int b) 
{
    assert(0 <= a && a < S);
    assert(0 <= b && b < S);
    
    int ra = getRoot(a);
    int rb = getRoot(b);

    if (ra == rb) return ra;

    // force the smaller guy to become captain
    int cap = std::min(ra,rb);
    int non = std::max(ra,rb);
    assert(cap != non);

    m_sets[cap] += m_sets[non];
    m_sets[non] = cap; 
    return cap;
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // UNIONFIND_HPP
