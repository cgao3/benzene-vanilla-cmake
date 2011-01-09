//----------------------------------------------------------------------------
/** @file Digraph.hpp */
//----------------------------------------------------------------------------

#ifndef DIGRAPH_HPP
#define DIGRAPH_HPP

#include <map>
#include <set>
#include <vector>
#include <iostream>
#include "Benzene.hpp"
#include "BenzeneAssert.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Generic directed graph. */
template <typename T>
class Digraph
{
public:
    
    /** Constructor. */
    Digraph();

    /** Destructor. */
    ~Digraph();

    //------------------------------------------------------------------------

    typedef typename std::map<T, std::set<T> > MapType;

    typedef typename std::set<T>::iterator iterator;
    typedef typename std::set<T>::const_iterator const_iterator;

    //------------------------------------------------------------------------

    /** Returns number of vertices in graph. */
    std::size_t NumVertices() const;

    /** Returns the vertex set. */
    const std::set<T>& Vertices() const;

    /** Returns true if vertex exists in graph. */
    bool VertexExists(const T& vertex) const;

    /** Returns true if vertex has no outgoing or incoming edges. */
    bool IsIsolated(const T& vertex) const;

    /** Returns the vertices with out-degree>0 and in-degree==0. */
    std::set<T> Sources() const;

    /** Returns the vertices with out-degree==0 and in-degree>0. */
    std::set<T> Sinks() const;

    /** Returns the number of edges entering target. */
    std::size_t InDegree(const T& target) const;

    /** Returns the number of edges leaving source. */
    std::size_t OutDegree(const T& source) const;
    
    /** Returns true if there is an edge (x->y). */
    bool IsEdge(const T& x, const T& y) const;

    /** Returns the transpose of this graph (ie, the graph with 
        edge directions reversed). */
    void Transpose(Digraph<T>& out) const;
    
    //------------------------------------------------------------------------

    /** Returns set of nodes pointed to by source. */
    const std::set<T>& OutSet(const T& source) const;

    /** Stores all targets of vertices in source. */
    void OutSet(const std::set<T>& source, std::set<T>& out) const;

    /** Returns nodes pointing to target. */
    const std::set<T>& InSet(const T& target) const;
    
    /** Stores all sources of edges into any member of target. */
    void InSet(const std::set<T>& target, std::set<T>& out) const;
    
    //------------------------------------------------------------------------

    /** Stores all vertices that are on a two cycle in cycles. */
    void FindTwoCycles(std::set<T>& cycles) const;

    /** Stores the strongly connected components in out. */
    void FindStronglyConnectedComponents(std::vector< std::set<T> >& out) const;

    //------------------------------------------------------------------------

    const_iterator out_begin(const T& source) const;
    const_iterator out_end(const T& source) const;

    const_iterator in_begin(const T& target) const;
    const_iterator in_end(const T& target) const;

    //------------------------------------------------------------------------

    /** Clears the graph. */
    void Clear();

    /** Adds edge from source to target. */
    void AddEdge(const T& source, const T& target);

    /** Adds edges from source to each of the targets. */
    void AddEdges(const T& source, const std::set<T>& targets);

    /** Removes edge from source to target. */
    void RemoveEdge(const T& source, const T& target);
    
    /** Removes vertex from the graph. */
    void RemoveVertex(const T& vertex);
    
private:
 
    //------------------------------------------------------------------------

    typedef std::pair<int, T> ScoreNodePair;
    typedef std::set<ScoreNodePair, std::greater<ScoreNodePair> > OrderedChildren;

    void OrderChildren(const std::set<T>& children, 
                       std::map<T, int>& tiebreaker, 
                       std::map<T, int>& finished, 
                       OrderedChildren& out) const;

    void DFS(int& step, 
             const T& vertex, 
             std::map<T, int>& tiebreaker, 
             std::map<T, int>& finished,
             std::set<T>& visited) const;

    //------------------------------------------------------------------------

    mutable MapType m_out;
    mutable MapType m_in;
    std::set<T> m_vertices;
};

//----------------------------------------------------------------------------

template<typename T>
Digraph<T>::Digraph()
{
}

template<typename T>
Digraph<T>::~Digraph()
{
}

//----------------------------------------------------------------------------

template<typename T>
std::size_t Digraph<T>::NumVertices() const
{
    return m_vertices.size();
}

template<typename T>
const std::set<T>& Digraph<T>::Vertices() const
{
    return m_vertices;
}

template<typename T>
std::set<T> Digraph<T>::Sources() const
{
    std::set<T> ret;
    for (typename MapType::const_iterator x=m_out.begin(); x!=m_out.end(); ++x) {
        if (m_in[x->first].empty())
            ret.insert(x->first);
    }
    return ret;
}

template<typename T>
std::set<T> Digraph<T>::Sinks() const
{
    std::set<T> ret;
    for (typename MapType::const_iterator x=m_in.begin(); x!=m_in.end(); ++x) {
        if (m_out[x->first].empty())
            ret.insert(x->first);
    }
    return ret;
}

template<typename T>
std::size_t Digraph<T>::InDegree(const T& target) const
{
    BenzeneAssert(VertexExists(target));
    return m_in[target].size();
}

template<typename T>
bool Digraph<T>::IsIsolated(const T& vertex) const
{
    return (m_in[vertex].empty() && m_out[vertex].empty());
}

template<typename T>
std::size_t Digraph<T>::OutDegree(const T& source) const
{
    BenzeneAssert(VertexExists(source));
    return m_out[source].size();
}

template<typename T>
bool Digraph<T>::IsEdge(const T& source, const T& target) const
{
    return m_out[source].count(target);
}

template<typename T>
const std::set<T>& Digraph<T>::OutSet(const T& source) const
{
    BenzeneAssert(VertexExists(source));
    return m_out[source];
}

template<typename T>
void Digraph<T>::OutSet(const std::set<T>& source, std::set<T>& out) const
{
    out.clear();
    for (const_iterator x=source.begin(); x!=source.end(); ++x) {
        BenzeneAssert(VertexExists(*x));
        out.insert(out_begin(*x), out_end(*x));
    }
}

template<typename T>
const std::set<T>& Digraph<T>::InSet(const T& target) const
{
    BenzeneAssert(VertexExists(target));
    return m_in[target];
}

template<typename T>
void Digraph<T>::InSet(const std::set<T>& target, std::set<T>& out) const
{
    out.clear();
    for (const_iterator x=target.begin(); x!=target.end(); ++x) {
        BenzeneAssert(VertexExists(*x));
        out.insert(in_begin(*x), in_end(*x));
    }
}

template<typename T>
void Digraph<T>::Transpose(Digraph<T>& out) const
{
    out.m_vertices = m_vertices;
    out.m_out = m_in;
    out.m_in = m_out;
}

//----------------------------------------------------------------------------

template<typename T>
void Digraph<T>::FindTwoCycles(std::set<T>& loops) const
{
    loops.clear();
    for (const_iterator x=m_vertices.begin(); x!=m_vertices.end(); ++x) {
        for (const_iterator y=out_begin(*x); y!=out_end(*x); ++y) {
            for (const_iterator z=out_begin(*y); z!=out_end(*y); ++z) {
                if (*z == *x) {
                    loops.insert(*x);
                    loops.insert(*y);
                    break;
                }
            }
        }
    }
}

//----------------------------------------------------------------------------

template<typename T>
typename Digraph<T>::const_iterator Digraph<T>::out_begin(const T& source) const
{
    BenzeneAssert(VertexExists(source));
    return m_out[source].begin();
}

template<typename T>
typename Digraph<T>::const_iterator Digraph<T>::out_end(const T& source) const
{
    BenzeneAssert(VertexExists(source));
    return m_out[source].end();
}

template<typename T>
typename Digraph<T>::const_iterator Digraph<T>::in_begin(const T& target) const
{
    BenzeneAssert(VertexExists(target));
    return m_in[target].begin();
}

template<typename T>
typename Digraph<T>::const_iterator Digraph<T>::in_end(const T& target) const
{
    BenzeneAssert(VertexExists(target));
    return m_in[target].end();
}

//----------------------------------------------------------------------------

template<typename T>
void Digraph<T>::Clear()
{
    m_in.clear();
    m_out.clear();
    m_vertices.clear();
}

template<typename T>
void Digraph<T>::AddEdge(const T& source, const T& target)
{
    m_vertices.insert(source);
    m_vertices.insert(target);

    m_out[source].insert(target);
    m_in[target].insert(source);
}

template<typename T>
void Digraph<T>::AddEdges(const T& source, const std::set<T>& targets)
{
    for (const_iterator x=targets.begin(); x!=targets.end(); ++x) {
        AddEdge(source, *x);
    }
}

template<typename T>
void Digraph<T>::RemoveEdge(const T& source, const T& target)
{
    m_out[source].erase(target);
    m_in[target].erase(source);
}

//----------------------------------------------------------------------------

template<typename T>
void Digraph<T>::RemoveVertex(const T& v)
{
    if (!VertexExists(v))
        return;

    // remove all outgoing edges
    for (iterator t = m_out[v].begin(); t != m_out[v].end(); ++t) {
        m_in[*t].erase(v);
    }
    m_out.erase(v);

    // remove all incoming edges
    for (iterator s = m_in[v].begin(); s != m_in[v].end(); ++s) {
        m_out[*s].erase(v);
    }
    m_in.erase(v);
   
    m_vertices.erase(v);
}

//----------------------------------------------------------------------------

template<typename T>
bool Digraph<T>::VertexExists(const T& v) const
{
    typename std::set<T>::const_iterator it = m_vertices.find(v);
    return (it != m_vertices.end());
}

//----------------------------------------------------------------------------

template<typename T>
void Digraph<T>::OrderChildren(const std::set<T>& children, 
                               std::map<T, int>& tiebreaker, 
                               std::map<T, int>& finished, 
                               OrderedChildren& out) const
{
    // consider vertices that have not been visited/expanded 
    for (const_iterator p = children.begin(); p != children.end(); ++p) {
        if (finished[*p] == 0) {
            out.insert(std::make_pair(tiebreaker[*p], *p));
        }
    }
}

template<typename T>
void Digraph<T>::DFS(int& step, 
                     const T& vertex, 
                     std::map<T, int>& tiebreaker, 
                     std::map<T, int>& finished,
                     std::set<T>& visited) const
{
    finished[vertex] = -1; // mark as expanded, but not finished. 

    // visit children in order determined by tiebreaker
    OrderedChildren children;
    OrderChildren(m_out[vertex], tiebreaker, finished, children);

    typename OrderedChildren::iterator it;
    for (it = children.begin(); it != children.end(); ++it) {
        const T& child = it->second;
        DFS(++step, child, tiebreaker, finished, visited);
    }

    // mark vertex as finished, add it to the set of visited vertices
    finished[vertex] = ++step;
    visited.insert(vertex);
}

template<typename T>
void 
Digraph<T>::FindStronglyConnectedComponents(std::vector< std::set<T> >& out) 
    const
{
    // finishing times of each vertex in first dfs; used as the
    // tiebreaker in the second dfs.
    typename std::map<T, int> finished;

    {
        int step = 1;
        typename std::map<T, int> tiebreaker;
        for (const_iterator p = m_vertices.begin(); p != m_vertices.end(); ++p) {
            if (finished[*p] > 0) continue;
            std::set<T> visited;
            this->DFS(step, *p, tiebreaker, finished, visited);
        }
    }

    Digraph<T> t;
    Transpose(t);
    out.clear();

    {
        int step = 1;
        typename std::map<T, int> finished2;

        // visit children in order determined by tiebreaker
        OrderedChildren children;
        OrderChildren(m_vertices, finished, finished2, children);

        typename OrderedChildren::iterator it;
        for (it = children.begin(); it != children.end(); ++it) {
            const T& child = it->second;
            if (finished2[child] > 0) continue;

            std::set<T> visited;
            t.DFS(step, child, finished, finished2, visited);
            out.push_back(visited);
        }
    }
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // DIGRAPH_HPP
