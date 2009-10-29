//---------------------------------------------------------------------------
/** @file

    @todo Write tests for the following methods:

          isVCSuperSetOfAny(), removeSuperSetsOf(), add(VCList other), 
          simple_add(), find(), getGreedyUnion(), operator==(), 
          operator!=(). 

          Maybe do some more thorough testing of the softlimit
          stuff. But this is pretty good already, I think.
*/
//---------------------------------------------------------------------------
#include <boost/test/auto_unit_test.hpp>

#include "VCList.hpp"
#include "ChangeLog.hpp"

using namespace benzene;

//---------------------------------------------------------------------------

namespace {

BOOST_AUTO_TEST_CASE(VCList_Basic)
{
    HexPoint x = HEX_CELL_A1;
    HexPoint y = HEX_CELL_A2;

    VCList vl(x, y, 2);

    // starts out empty
    BOOST_CHECK(vl.empty());
    BOOST_CHECK_EQUAL(vl.size(), 0);

    // any add should succeed here
    bitset_t b1;
    b1.set(FIRST_CELL);
    VC v1(x, y, b1, VC_RULE_BASE);
    BOOST_CHECK_EQUAL(vl.add(v1, NULL), VCList::ADDED_INSIDE_SOFT_LIMIT);
    BOOST_CHECK(!vl.empty());
    BOOST_CHECK_EQUAL(vl.size(), 1);

    BOOST_CHECK_EQUAL(vl.hardIntersection(), v1.carrier());
    BOOST_CHECK_EQUAL(vl.getUnion(), v1.carrier());

    // duplicates are not added
    BOOST_CHECK_EQUAL(vl.add(v1, NULL), VCList::ADD_FAILED);
    BOOST_CHECK(!vl.empty());
    BOOST_CHECK_EQUAL(vl.size(), 1);

    // try adding a superset; should fail
    bitset_t b2;
    b2.set(FIRST_CELL);
    b2.set(FIRST_CELL+1);
    VC v2(x, y, b2, VC_RULE_BASE);
    BOOST_CHECK_EQUAL(vl.add(v2, NULL), VCList::ADD_FAILED);
    BOOST_CHECK(!vl.empty());
    BOOST_CHECK_EQUAL(vl.size(), 1);

    // add a non-superset with three set bits
    bitset_t b3;
    b3.set(FIRST_CELL+1);
    b3.set(FIRST_CELL+2);
    b3.set(FIRST_CELL+3);
    VC v3(x, y, b3, VC_RULE_BASE);
    BOOST_CHECK_EQUAL(vl.add(v3, NULL), VCList::ADDED_INSIDE_SOFT_LIMIT);
    BOOST_CHECK(!vl.empty());
    BOOST_CHECK_EQUAL(vl.size(), 2);

    // ensure v1 appears before v3
    VCList::const_iterator it = vl.begin();
    BOOST_CHECK_EQUAL(*it, v1);
    ++it;
    BOOST_CHECK_EQUAL(*it, v3);
    ++it;
    BOOST_CHECK(it == vl.end());

    BOOST_CHECK_EQUAL(vl.hardIntersection(), v1.carrier() & v3.carrier());
    BOOST_CHECK_EQUAL(vl.getUnion(), v1.carrier() | v3.carrier());

    // add a subset of v3: add should succeed and v3 should be
    // removed.
    bitset_t b4;
    b4.set(FIRST_CELL+1);
    b4.set(FIRST_CELL+2);
    VC v4(x, y, b4, VC_RULE_BASE);
    BOOST_CHECK_EQUAL(vl.add(v4, NULL), VCList::ADDED_INSIDE_SOFT_LIMIT);
    BOOST_CHECK(!vl.empty());
    BOOST_CHECK_EQUAL(vl.size(), 2);

    // list should be [v1, v4].
    it = vl.begin();
    BOOST_CHECK_EQUAL(*it, v1);
    ++it;
    BOOST_CHECK_EQUAL(*it, v4);
    ++it;
    BOOST_CHECK(it == vl.end());

    BOOST_CHECK_EQUAL(vl.hardIntersection(), v1.carrier() & v4.carrier());
    BOOST_CHECK_EQUAL(vl.getUnion(), v1.carrier() | v4.carrier());

    // add another vc to the list; this one is added past the
    // end of the softlimit.
    bitset_t b5;
    b5.set(FIRST_CELL+1);
    b5.set(FIRST_CELL+3);
    b5.set(FIRST_CELL+5);
    VC v5(x, y, b5, VC_RULE_BASE);
    BOOST_CHECK_EQUAL(vl.add(v5, NULL), VCList::ADDED_INSIDE_HARD_LIMIT);
    BOOST_CHECK(!vl.empty());
    BOOST_CHECK_EQUAL(vl.size(), 3);

    // list should be [v1, v4, v5].
    it = vl.begin();
    BOOST_CHECK_EQUAL(*it, v1);
    ++it;
    BOOST_CHECK_EQUAL(*it, v4);
    ++it;
    BOOST_CHECK_EQUAL(*it, v5);
    ++it;
    BOOST_CHECK(it == vl.end());
    
    BOOST_CHECK_EQUAL(vl.softIntersection(), v1.carrier() & v4.carrier());
    BOOST_CHECK_EQUAL(vl.hardIntersection(), 
                      v1.carrier() & v4.carrier() & v5.carrier());
    BOOST_CHECK_EQUAL(vl.getUnion(), v1.carrier()|v4.carrier()|v5.carrier());
    
    // test removingAllContaining()
    bitset_t remove;
    remove.set(FIRST_CELL+2);
    remove.set(FIRST_EDGE);
    std::list<VC> removed;
    BOOST_CHECK_EQUAL(vl.removeAllContaining(remove, removed, NULL), 1);
    BOOST_CHECK_EQUAL(*removed.begin(), v4);
    BOOST_CHECK_EQUAL(vl.size(), 2);

    // list should be [v1, v5].
    it = vl.begin();
    BOOST_CHECK_EQUAL(*it, v1);
    ++it;
    BOOST_CHECK_EQUAL(*it, v5);
    ++it;
    BOOST_CHECK(it == vl.end());

    BOOST_CHECK_EQUAL(vl.hardIntersection(), v1.carrier() & v5.carrier());
    BOOST_CHECK_EQUAL(vl.getUnion(), v1.carrier() | v5.carrier());

    // test remove(VC)
    BOOST_CHECK(!vl.remove(v4, NULL));
    BOOST_CHECK(vl.remove(v1, NULL));
    BOOST_CHECK_EQUAL(vl.size(), 1);
    
    // list should be [v5]
    it = vl.begin();
    BOOST_CHECK_EQUAL(*it, v5);
    ++it;
    BOOST_CHECK(it == vl.end());

    BOOST_CHECK_EQUAL(vl.hardIntersection(), v5.carrier());
    BOOST_CHECK_EQUAL(vl.getUnion(), v5.carrier());
}

}

//---------------------------------------------------------------------------
