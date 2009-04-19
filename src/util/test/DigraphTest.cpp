//----------------------------------------------------------------------------
/** @file
 */
//----------------------------------------------------------------------------

#include <boost/test/auto_unit_test.hpp>

#include "Digraph.hpp"

using namespace benzene;

//---------------------------------------------------------------------------

namespace {

BOOST_AUTO_TEST_CASE(Digraph_AllTests)
{
    Digraph<int> g;

    g.AddEdge(1, 2);
    BOOST_CHECK_EQUAL(g.OutDegree(1), 1);
    BOOST_CHECK_EQUAL(g.OutDegree(2), 0);
    BOOST_CHECK_EQUAL(g.InDegree(1), 0);
    BOOST_CHECK_EQUAL(g.InDegree(2), 1);
    BOOST_CHECK(g.IsEdge(1, 2));
    BOOST_CHECK(!g.IsEdge(2, 1));
    BOOST_CHECK(g.VertexExists(1));
    BOOST_CHECK(g.VertexExists(2));

    g.AddEdge(2, 1);
    BOOST_CHECK_EQUAL(g.OutDegree(1), 1);
    BOOST_CHECK_EQUAL(g.OutDegree(2), 1);
    BOOST_CHECK_EQUAL(g.InDegree(1), 1);
    BOOST_CHECK_EQUAL(g.InDegree(2), 1);
    BOOST_CHECK(g.IsEdge(1, 2));
    BOOST_CHECK(g.IsEdge(2, 1));

    std::set<int> s;
    g.AddEdge(4, 5);
    g.AddEdge(1, 3);
    g.AddEdge(5, 7);
    g.AddEdge(3, 1);
    g.AddEdge(8, 1);
    g.AddEdge(9, 3);
    g.FindTwoCycles(s);
    BOOST_CHECK_EQUAL(s.size(), 3u);
    BOOST_CHECK_EQUAL(s.count(1), 1u);
    BOOST_CHECK_EQUAL(s.count(2), 1u);
    BOOST_CHECK_EQUAL(s.count(3), 1u);
    BOOST_CHECK_EQUAL(s.count(5), 0u);

    s = g.OutSet(1);
    BOOST_CHECK_EQUAL(s.size(), 2u);
    BOOST_CHECK_EQUAL(s.count(2), 1u);
    BOOST_CHECK_EQUAL(s.count(3), 1u);

    s = g.InSet(1);
    BOOST_CHECK_EQUAL(s.size(), 3u);
    BOOST_CHECK_EQUAL(s.count(2), 1u);
    BOOST_CHECK_EQUAL(s.count(3), 1u);
    BOOST_CHECK_EQUAL(s.count(8), 1u);

    std::set<int> t;
    t.insert(1);
    t.insert(3);
    g.InSet(t, s);
    BOOST_CHECK_EQUAL(s.size(), 5u);
    BOOST_CHECK_EQUAL(s.count(1), 1u);
    BOOST_CHECK_EQUAL(s.count(2), 1u);
    BOOST_CHECK_EQUAL(s.count(3), 1u);
    BOOST_CHECK_EQUAL(s.count(8), 1u);
    BOOST_CHECK_EQUAL(s.count(9), 1u);    

    // add a loop
    g.AddEdge(1, 1);
    s = g.OutSet(1);
    BOOST_CHECK_EQUAL(s.size(), 3u);
    BOOST_CHECK_EQUAL(s.count(1), 1u);

    // check the transpose
    Digraph<int> tr;
    g.Transpose(tr);
    BOOST_CHECK(tr.IsEdge(1, 2));
    BOOST_CHECK(tr.IsEdge(2, 1));
    BOOST_CHECK(tr.IsEdge(5, 4));    
    BOOST_CHECK(tr.IsEdge(3, 1));
    BOOST_CHECK(tr.IsEdge(7, 5));
    BOOST_CHECK(tr.IsEdge(1, 3));
    BOOST_CHECK(tr.IsEdge(1, 8));
    BOOST_CHECK(tr.IsEdge(3, 9));
    BOOST_CHECK(tr.IsEdge(1, 1));

    // check sources and sinks
    t = g.Sources();
    BOOST_CHECK_EQUAL(t.size(), 3u);
    BOOST_CHECK_EQUAL(t.count(4), 1u);
    BOOST_CHECK_EQUAL(t.count(8), 1u);
    BOOST_CHECK_EQUAL(t.count(9), 1u);
    t = g.Sinks();
    BOOST_CHECK_EQUAL(t.size(), 1u);
    BOOST_CHECK_EQUAL(t.count(7), 1u);
    
    // check removing edges and removing vertices
    g.Clear();
    BOOST_CHECK_EQUAL(g.NumVertices(), 0);

    g.AddEdge(1, 2);
    g.AddEdge(2, 3);
    g.RemoveEdge(1, 2);
    BOOST_CHECK(!g.IsEdge(1, 2));
    BOOST_CHECK(g.VertexExists(1));
    BOOST_CHECK(g.VertexExists(2));
    BOOST_CHECK(g.VertexExists(3));
    
    g.AddEdge(1, 5);
    g.AddEdge(2, 5);
    g.RemoveVertex(5);
    BOOST_CHECK(!g.VertexExists(5));
    BOOST_CHECK(g.VertexExists(1));
    BOOST_CHECK(g.VertexExists(2));
    BOOST_CHECK(!g.IsEdge(1, 5));
    BOOST_CHECK(!g.IsEdge(2, 5));

}

BOOST_AUTO_TEST_CASE(Digraph_StronglyConnectedComponents)
{
    Digraph<int> g1;
    std::vector<std::set<int> > comp;

    //
    //  1 -> 2 -> 3 -> 4    8 <-> 9  10 -> 11
    //  ^         | 
    //  +---------+
    //
    //  Components are (1,2,3), (4), (8, 9), (10), (11)
    //
    g1.AddEdge(1, 2);
    g1.AddEdge(2, 3);
    g1.AddEdge(3, 1);
    g1.AddEdge(3, 4);

    g1.AddEdge(8, 9);
    g1.AddEdge(9, 8);
    
    g1.AddEdge(10, 11);

    g1.FindStronglyConnectedComponents(comp);
    BOOST_CHECK_EQUAL(comp.size(), 5u);

    //
    //  1 -> 2 -> 3 -> 4 -> 9 -> 10 -> 12 -> 13  14 -+
    //  ^         v         ^    v      ^----+    ^__|
    //  7 <- 6 <- 5 <- 8    +----11
    //
    //  Components are:
    //    (1,2,3,5,6,7), (4), (8), (9,10,11), (12,13), (14)
    //
    Digraph<int> g2;
    g2.AddEdge(1, 2);
    g2.AddEdge(2, 3);
    g2.AddEdge(3, 5);
    g2.AddEdge(5, 6);
    g2.AddEdge(6, 7);
    g2.AddEdge(7, 1);

    g2.AddEdge(8, 5);
    
    g2.AddEdge(4, 9);

    g2.AddEdge(9, 10);
    g2.AddEdge(10, 11);
    g2.AddEdge(11, 9);

    g2.AddEdge(10, 12);
    
    g2.AddEdge(12, 13);
    g2.AddEdge(13, 12);
    
    g2.AddEdge(14, 14);

    g2.FindStronglyConnectedComponents(comp);
    BOOST_CHECK_EQUAL(comp.size(), 6u);
    
    //  3
    //  ^
    //  1 > 2
    Digraph<int> g3;
    g3.AddEdge(1, 2);
    g3.AddEdge(1, 3);
    g3.FindStronglyConnectedComponents(comp);
    BOOST_CHECK_EQUAL(comp.size(), 3u);
    
    //  3
    //  v
    //  1 < 2
    Digraph<int> g4;
    g4.AddEdge(2, 1);
    g4.AddEdge(3, 1);
    g4.FindStronglyConnectedComponents(comp);
    BOOST_CHECK_EQUAL(comp.size(), 3u);
}

}

//---------------------------------------------------------------------------
