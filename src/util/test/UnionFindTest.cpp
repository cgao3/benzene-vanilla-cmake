//---------------------------------------------------------------------------
/** @file
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
    
    // test that clear method sets all elements to be their own root
    uf.clear();
    BOOST_CHECK_EQUAL(uf.getRoot(0), 0);
    BOOST_CHECK_EQUAL(uf.getRoot(1), 1);
    BOOST_CHECK_EQUAL(uf.getRoot(7), 7);
    BOOST_CHECK_EQUAL(uf.getRoot(13), 13);
    BOOST_CHECK(uf.isRoot(0));
    BOOST_CHECK(uf.isRoot(1));
    BOOST_CHECK(uf.isRoot(7));
    BOOST_CHECK(uf.isRoot(13));
    
    // test unionGroups merges two, no effect on others
    uf.unionGroups(7, 12);
    BOOST_CHECK(uf.isRoot(7) != uf.isRoot(12));
    BOOST_CHECK_EQUAL(uf.getRoot(7), uf.getRoot(12));
    BOOST_CHECK(uf.isRoot(0));
    BOOST_CHECK(uf.isRoot(1));
    BOOST_CHECK(uf.isRoot(13));
    
    uf.unionGroups(0, 1);
    BOOST_CHECK(uf.isRoot(0) != uf.isRoot(1));
    BOOST_CHECK_EQUAL(uf.getRoot(0), uf.getRoot(1));
    BOOST_CHECK_EQUAL(uf.getRoot(7), uf.getRoot(12));
    BOOST_CHECK(uf.isRoot(13));
    
    // test merging with self has no effect
    uf.unionGroups(0, 1);
    BOOST_CHECK(uf.isRoot(0) != uf.isRoot(1));
    BOOST_CHECK_EQUAL(uf.getRoot(0), uf.getRoot(1));
    BOOST_CHECK_EQUAL(uf.getRoot(7), uf.getRoot(12));
    BOOST_CHECK(uf.isRoot(13));
    
    // test merging of groups
    uf.unionGroups(7, 1);
    root = uf.getRoot(0);
    BOOST_CHECK_EQUAL(root, uf.getRoot(1));
    BOOST_CHECK_EQUAL(root, uf.getRoot(7));
    BOOST_CHECK_EQUAL(root, uf.getRoot(12));
    BOOST_CHECK(0==root || 1==root || 7==root || 12==root);
    BOOST_CHECK(uf.isRoot(6));
    BOOST_CHECK(uf.isRoot(8));
    BOOST_CHECK(uf.isRoot(13));
    
    // test that getRoot does not affect group membership
    root = uf.getRoot(12);
    root = uf.getRoot(1);
    root = uf.getRoot(13);
    root = uf.getRoot(0);
    BOOST_CHECK_EQUAL(root, uf.getRoot(1));
    BOOST_CHECK_EQUAL(root, uf.getRoot(7));
    BOOST_CHECK_EQUAL(root, uf.getRoot(12));
    BOOST_CHECK(0==root || 1==root || 7==root || 12==root);
    BOOST_CHECK(uf.isRoot(6));
    BOOST_CHECK(uf.isRoot(8));
    BOOST_CHECK(uf.isRoot(13));
    
    // test clear resets properly to individual sets
    uf.clear();
    BOOST_CHECK(uf.isRoot(0));
    BOOST_CHECK(uf.isRoot(1));
    BOOST_CHECK(uf.isRoot(7));
    BOOST_CHECK(uf.isRoot(13));
    BOOST_CHECK(uf.getRoot(7) != uf.getRoot(12));
}

}

//---------------------------------------------------------------------------
