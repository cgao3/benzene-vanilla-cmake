//---------------------------------------------------------------------------
/** @file VCListTest.cpp

    @todo Write tests for the following methods:

          IsVCSuperSetOfAny(), RemoveSuperSetsOf(), Add(VCList other), 
          SimpleAdd(), Find(), GetGreedyUnion(), operator==(), 
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

BOOST_AUTO_TEST_CASE(VCList_Iterators)
{
    HexPoint x = HEX_CELL_A1;
    HexPoint y = HEX_CELL_A2;
    VCList vl(x, y, 10);
    {
        VCListIterator it(vl);
        BOOST_CHECK(!it);
        VCListConstIterator cit(vl);
        BOOST_CHECK(!cit);
    }
    bitset_t b1;
    b1.set(HEX_CELL_C1);
    VC v1(x, y, b1, VC_RULE_BASE);
    BOOST_CHECK_EQUAL(vl.Add(v1, NULL), VCList::ADDED_INSIDE_SOFT_LIMIT);
    bitset_t b2;
    b2.set(HEX_CELL_C2);
    VC v2(x, y, b2, VC_RULE_BASE);
    BOOST_CHECK_EQUAL(vl.Add(v2, NULL), VCList::ADDED_INSIDE_SOFT_LIMIT);
    bitset_t b3;
    b3.set(HEX_CELL_C3);
    VC v3(x, y, b3, VC_RULE_BASE);
    BOOST_CHECK_EQUAL(vl.Add(v3, NULL), VCList::ADDED_INSIDE_SOFT_LIMIT);
    {   // Check FindInList();
        VC* v = vl.FindInList(v2);
        BOOST_CHECK_EQUAL(*v, v2);
        BOOST_CHECK(!v->Processed());
        v->SetProcessed(true);
        VC* w = vl.FindInList(v2);
        BOOST_CHECK(w->Processed());
    }
    {   // Check iterating over entire list
        VCListIterator it(vl);
        BOOST_CHECK(it);
        BOOST_CHECK(*it == v1);
        BOOST_CHECK(it->Carrier() == v1.Carrier());
        ++it;
        BOOST_CHECK(it);
        BOOST_CHECK(*it == v2);
        ++it;
        BOOST_CHECK(it);
        BOOST_CHECK(*it == v3);
        ++it;
        BOOST_CHECK(!it);
    }
    {   // Check iterating over only the first N elements
        VCListIterator it(vl, 2);
        BOOST_CHECK(it);
        BOOST_CHECK(*it == v1);
        BOOST_CHECK(it->Carrier() == v1.Carrier());
        ++it;
        BOOST_CHECK(it);
        BOOST_CHECK(*it == v2);
        ++it;
        BOOST_CHECK(!it);
    }
    {   // Check iterating over entire list
        VCListConstIterator it(vl);
        BOOST_CHECK(it);
        BOOST_CHECK(*it == v1);
        BOOST_CHECK(it->Carrier() == v1.Carrier());
        ++it;
        BOOST_CHECK(it);
        BOOST_CHECK(*it == v2);
        ++it;
        BOOST_CHECK(it);
        BOOST_CHECK(*it == v3);
        ++it;
        BOOST_CHECK(!it);
    }
    {   // Check iterating over only the first N elements
        VCListConstIterator it(vl, 2);
        BOOST_CHECK(it);
        BOOST_CHECK(*it == v1);
        BOOST_CHECK(it->Carrier() == v1.Carrier());
        ++it;
        BOOST_CHECK(it);
        BOOST_CHECK(*it == v2);
        ++it;
        BOOST_CHECK(!it);
    }
}

BOOST_AUTO_TEST_CASE(VCList_Basic)
{
    HexPoint x = HEX_CELL_A1;
    HexPoint y = HEX_CELL_A2;
    VCList vl(x, y, 2);

    // starts out Empty
    BOOST_CHECK(vl.Empty());
    BOOST_CHECK_EQUAL(vl.Size(), 0);

    // any add should succeed here
    bitset_t b1;
    b1.set(FIRST_CELL);
    VC v1(x, y, b1, VC_RULE_BASE);
    BOOST_CHECK_EQUAL(vl.Add(v1, NULL), VCList::ADDED_INSIDE_SOFT_LIMIT);
    BOOST_CHECK(!vl.Empty());
    BOOST_CHECK_EQUAL(vl.Size(), 1);

    BOOST_CHECK_EQUAL(vl.HardIntersection(), v1.Carrier());
    BOOST_CHECK_EQUAL(vl.GetUnion(), v1.Carrier());

    // duplicates are not added
    BOOST_CHECK_EQUAL(vl.Add(v1, NULL), VCList::ADD_FAILED);
    BOOST_CHECK(!vl.Empty());
    BOOST_CHECK_EQUAL(vl.Size(), 1);

    // try adding a superset; should fail
    bitset_t b2;
    b2.set(FIRST_CELL);
    b2.set(FIRST_CELL + 1);
    VC v2(x, y, b2, VC_RULE_BASE);
    BOOST_CHECK_EQUAL(vl.Add(v2, NULL), VCList::ADD_FAILED);
    BOOST_CHECK(!vl.Empty());
    BOOST_CHECK_EQUAL(vl.Size(), 1);

    // add a non-superset with three set bits
    bitset_t b3;
    b3.set(FIRST_CELL + 1);
    b3.set(FIRST_CELL + 2);
    b3.set(FIRST_CELL + 3);
    VC v3(x, y, b3, VC_RULE_BASE);
    BOOST_CHECK_EQUAL(vl.Add(v3, NULL), VCList::ADDED_INSIDE_SOFT_LIMIT);
    BOOST_CHECK(!vl.Empty());
    BOOST_CHECK_EQUAL(vl.Size(), 2);

    // ensure v1 appears before v3
    {
        VCListConstIterator it(vl);
        BOOST_CHECK_EQUAL(*it, v1);
        ++it;
        BOOST_CHECK_EQUAL(*it, v3);
        ++it;
        BOOST_CHECK(!it);
    }
    BOOST_CHECK_EQUAL(vl.HardIntersection(), v1.Carrier() & v3.Carrier());
    BOOST_CHECK_EQUAL(vl.GetUnion(), v1.Carrier() | v3.Carrier());

    // add a subset of v3: add should succeed and v3 should be
    // removed.
    bitset_t b4;
    b4.set(FIRST_CELL + 1);
    b4.set(FIRST_CELL + 2);
    VC v4(x, y, b4, VC_RULE_BASE);
    BOOST_CHECK_EQUAL(vl.Add(v4, NULL), VCList::ADDED_INSIDE_SOFT_LIMIT);
    BOOST_CHECK(!vl.Empty());
    BOOST_CHECK_EQUAL(vl.Size(), 2);

    // list should be [v1, v4].
    {
        VCListConstIterator it(vl);
        BOOST_CHECK_EQUAL(*it, v1);
        ++it;
        BOOST_CHECK_EQUAL(*it, v4);
        ++it;
        BOOST_CHECK(!it);
    }
    BOOST_CHECK_EQUAL(vl.HardIntersection(), v1.Carrier() & v4.Carrier());
    BOOST_CHECK_EQUAL(vl.GetUnion(), v1.Carrier() | v4.Carrier());

    // add another vc to the list; this one is added past the
    // end of the softlimit.
    bitset_t b5;
    b5.set(FIRST_CELL + 1);
    b5.set(FIRST_CELL + 3);
    b5.set(FIRST_CELL + 5);
    VC v5(x, y, b5, VC_RULE_BASE);
    BOOST_CHECK_EQUAL(vl.Add(v5, NULL), VCList::ADDED_INSIDE_HARD_LIMIT);
    BOOST_CHECK(!vl.Empty());
    BOOST_CHECK_EQUAL(vl.Size(), 3);

    // list should be [v1, v4, v5].
    {
        VCListConstIterator it(vl);
        BOOST_CHECK_EQUAL(*it, v1);
        ++it;
        BOOST_CHECK_EQUAL(*it, v4);
        ++it;
        BOOST_CHECK_EQUAL(*it, v5);
        ++it;
        BOOST_CHECK(!it);
    }    
    BOOST_CHECK_EQUAL(vl.SoftIntersection(), v1.Carrier() & v4.Carrier());
    BOOST_CHECK_EQUAL(vl.HardIntersection(), 
                      v1.Carrier() & v4.Carrier() & v5.Carrier());
    BOOST_CHECK_EQUAL(vl.GetUnion(), v1.Carrier()|v4.Carrier()|v5.Carrier());
    
    // test RemovingAllContaining()
    bitset_t remove;
    remove.set(FIRST_CELL+2);
    remove.set(FIRST_EDGE);
    std::list<VC> removed;
    BOOST_CHECK_EQUAL(vl.RemoveAllContaining(remove, removed, NULL), 1);
    BOOST_CHECK_EQUAL(*removed.begin(), v4);
    BOOST_CHECK_EQUAL(vl.Size(), 2);

    // list should be [v1, v5].
    {
        VCListConstIterator it(vl);
        BOOST_CHECK_EQUAL(*it, v1);
        ++it;
        BOOST_CHECK_EQUAL(*it, v5);
        ++it;
        BOOST_CHECK(!it);
    }
    BOOST_CHECK_EQUAL(vl.HardIntersection(), v1.Carrier() & v5.Carrier());
    BOOST_CHECK_EQUAL(vl.GetUnion(), v1.Carrier() | v5.Carrier());

    // test remove(VC)
    BOOST_CHECK(!vl.Remove(v4, NULL));
    BOOST_CHECK(vl.Remove(v1, NULL));
    BOOST_CHECK_EQUAL(vl.Size(), 1);
    
    // list should be [v5]
    {
        VCListConstIterator it(vl);
        BOOST_CHECK_EQUAL(*it, v5);
        ++it;
        BOOST_CHECK(!it);
    }
    BOOST_CHECK_EQUAL(vl.HardIntersection(), v5.Carrier());
    BOOST_CHECK_EQUAL(vl.GetUnion(), v5.Carrier());
}

}

//---------------------------------------------------------------------------
