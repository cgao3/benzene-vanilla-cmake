//---------------------------------------------------------------------------
/** @file UnionFindTest.cpp
 */
//---------------------------------------------------------------------------

#include <boost/test/auto_unit_test.hpp>

#include "Hex.hpp"
#include "UnionFind.hpp"

using namespace benzene;

//---------------------------------------------------------------------------

namespace {

BOOST_AUTO_TEST_CASE(UnionFind_AllTests)
{
    UnionFind<14> uf;
    int root;
    
    // test that Clear method sets all elements to be their own root
    uf.Clear();
    BOOST_CHECK_EQUAL(uf.GetRoot(0), 0);
    BOOST_CHECK_EQUAL(uf.GetRoot(1), 1);
    BOOST_CHECK_EQUAL(uf.GetRoot(7), 7);
    BOOST_CHECK_EQUAL(uf.GetRoot(13), 13);
    BOOST_CHECK(uf.IsRoot(0));
    BOOST_CHECK(uf.IsRoot(1));
    BOOST_CHECK(uf.IsRoot(7));
    BOOST_CHECK(uf.IsRoot(13));
    
    // test UnionGroups merges two, no effect on others
    uf.UnionGroups(7, 12);
    BOOST_CHECK(uf.IsRoot(7) != uf.IsRoot(12));
    BOOST_CHECK_EQUAL(uf.GetRoot(7), uf.GetRoot(12));
    BOOST_CHECK(uf.IsRoot(0));
    BOOST_CHECK(uf.IsRoot(1));
    BOOST_CHECK(uf.IsRoot(13));
    
    uf.UnionGroups(0, 1);
    BOOST_CHECK(uf.IsRoot(0) != uf.IsRoot(1));
    BOOST_CHECK_EQUAL(uf.GetRoot(0), uf.GetRoot(1));
    BOOST_CHECK_EQUAL(uf.GetRoot(7), uf.GetRoot(12));
    BOOST_CHECK(uf.IsRoot(13));
    
    // test merging with self has no effect
    uf.UnionGroups(0, 1);
    BOOST_CHECK(uf.IsRoot(0) != uf.IsRoot(1));
    BOOST_CHECK_EQUAL(uf.GetRoot(0), uf.GetRoot(1));
    BOOST_CHECK_EQUAL(uf.GetRoot(7), uf.GetRoot(12));
    BOOST_CHECK(uf.IsRoot(13));
    
    // test merging of groups
    uf.UnionGroups(7, 1);
    root = uf.GetRoot(0);
    BOOST_CHECK_EQUAL(root, uf.GetRoot(1));
    BOOST_CHECK_EQUAL(root, uf.GetRoot(7));
    BOOST_CHECK_EQUAL(root, uf.GetRoot(12));
    BOOST_CHECK(0==root || 1==root || 7==root || 12==root);
    BOOST_CHECK(uf.IsRoot(6));
    BOOST_CHECK(uf.IsRoot(8));
    BOOST_CHECK(uf.IsRoot(13));
    
    // test that GetRoot does not affect group membership
    root = uf.GetRoot(12);
    root = uf.GetRoot(1);
    root = uf.GetRoot(13);
    root = uf.GetRoot(0);
    BOOST_CHECK_EQUAL(root, uf.GetRoot(1));
    BOOST_CHECK_EQUAL(root, uf.GetRoot(7));
    BOOST_CHECK_EQUAL(root, uf.GetRoot(12));
    BOOST_CHECK(0==root || 1==root || 7==root || 12==root);
    BOOST_CHECK(uf.IsRoot(6));
    BOOST_CHECK(uf.IsRoot(8));
    BOOST_CHECK(uf.IsRoot(13));
    
    // test Clear resets properly to individual sets
    uf.Clear();
    BOOST_CHECK(uf.IsRoot(0));
    BOOST_CHECK(uf.IsRoot(1));
    BOOST_CHECK(uf.IsRoot(7));
    BOOST_CHECK(uf.IsRoot(13));
    BOOST_CHECK(uf.GetRoot(7) != uf.GetRoot(12));
}

}

//---------------------------------------------------------------------------
