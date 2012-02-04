//---------------------------------------------------------------------------
/** @file CarrierListTest.cpp
*/
//---------------------------------------------------------------------------
#include <boost/test/auto_unit_test.hpp>

#include "VCS.cpp"

using namespace benzene;

//---------------------------------------------------------------------------

namespace {

class List : public CarrierList
{
public:
    void Add(bitset_t carrier) { AddNew(carrier); }
    using CarrierList::TrySetOld;
    using CarrierList::RemoveSupersetsOfCheckAnyRemoved;
};

BOOST_AUTO_TEST_CASE(CarrierList_Iterators)
{
    List vl;
    {
        List::Iterator it(vl);
        BOOST_CHECK(!it);
    }
    bitset_t b1;
    b1.set(HEX_CELL_C1);
    vl.Add(b1);
    bitset_t b2;
    b2.set(HEX_CELL_C2);
    vl.Add(b2);
    bitset_t b3;
    b3.set(HEX_CELL_C3);
    vl.Add(b3);
    BOOST_CHECK(vl.TrySetOld(b2));
    {   // Check iterating over entire list
        List::Iterator it(vl);
        BOOST_CHECK(it);
        BOOST_CHECK(it.Carrier() == b1);
        ++it;
        BOOST_CHECK(it);
        BOOST_CHECK(it.Carrier() == b2);
        ++it;
        BOOST_CHECK(it);
        BOOST_CHECK(it.Carrier() == b3);
        ++it;
        BOOST_CHECK(!it);
    }
}

BOOST_AUTO_TEST_CASE(CarrierList_Basic)
{
    List vl;

    // starts out Empty
    BOOST_CHECK(vl.IsEmpty());
    BOOST_CHECK_EQUAL(vl.Count(), 0);

    // any add should succeed here
    bitset_t b1;
    b1.set(FIRST_CELL);
    vl.Add(b1);
    BOOST_CHECK(!vl.IsEmpty());
    BOOST_CHECK_EQUAL(vl.Count(), 1);

    BOOST_CHECK_EQUAL(vl.GetAllIntersection(), b1);
    BOOST_CHECK_EQUAL(vl.GetGreedyUnion(), b1);


    // supersets
    BOOST_CHECK(vl.SupersetOfAny(b1));
    bitset_t b2;
    b2.set(FIRST_CELL);
    b2.set(FIRST_CELL + 1);
    BOOST_CHECK(vl.SupersetOfAny(b2));

    // add a non-superset with three set bits
    bitset_t b3;
    b3.set(FIRST_CELL + 1);
    b3.set(FIRST_CELL + 2);
    b3.set(FIRST_CELL + 3);
    vl.Add(b3);
    BOOST_CHECK(!vl.IsEmpty());
    BOOST_CHECK_EQUAL(vl.Count(), 2);

    // ensure b1 appears before b3
    {
        List::Iterator it(vl);
        BOOST_CHECK_EQUAL(it.Carrier(), b1);
        ++it;
        BOOST_CHECK_EQUAL(it.Carrier(), b3);
        ++it;
        BOOST_CHECK(!it);
    }
    BOOST_CHECK_EQUAL(vl.GetAllIntersection(), b1 & b3);
    BOOST_CHECK_EQUAL(vl.GetGreedyUnion(), b1 | b3);

    // add a subset of b3: add should succeed and b3 should be
    // removed.
    bitset_t b4;
    b4.set(FIRST_CELL + 1);
    b4.set(FIRST_CELL + 2);
    BOOST_CHECK(vl.RemoveSupersetsOfCheckAnyRemoved(b4));
    vl.Add(b4);
    BOOST_CHECK(!vl.IsEmpty());
    BOOST_CHECK_EQUAL(vl.Count(), 2);

    // list should be [b1, b4].
    {
        List::Iterator it(vl);
        BOOST_CHECK_EQUAL(it.Carrier(), b1);
        ++it;
        BOOST_CHECK_EQUAL(it.Carrier(), b4);
        ++it;
        BOOST_CHECK(!it);
    }
    BOOST_CHECK_EQUAL(vl.GetAllIntersection(), b1 & b4);
    BOOST_CHECK_EQUAL(vl.GetGreedyUnion(), b1 | b4);

    // add another carrier to the list
    bitset_t b5;
    b5.set(FIRST_CELL + 1);
    b5.set(FIRST_CELL + 3);
    b5.set(FIRST_CELL + 5);
    vl.Add(b5);
    BOOST_CHECK(!vl.IsEmpty());
    BOOST_CHECK_EQUAL(vl.Count(), 3);

    // list should be [b1, b4, b5].
    {
        List::Iterator it(vl);
        BOOST_CHECK_EQUAL(it.Carrier(), b1);
        ++it;
        BOOST_CHECK_EQUAL(it.Carrier(), b4);
        ++it;
        BOOST_CHECK_EQUAL(it.Carrier(), b5);
        ++it;
        BOOST_CHECK(!it);
    }

    // test RemovingAllContaining()
    bitset_t remove;
    remove.set(FIRST_CELL+2);
    remove.set(FIRST_EDGE);
    std::vector<bitset_t> removed;
    BOOST_CHECK_EQUAL(vl.RemoveAllContaining(remove, removed), 1);
    BOOST_CHECK_EQUAL(*removed.begin(), b4);
    BOOST_CHECK_EQUAL(vl.Count(), 2);

    // list should be [b1, b5].
    {
        List::Iterator it(vl);
        BOOST_CHECK_EQUAL(it.Carrier(), b1);
        ++it;
        BOOST_CHECK_EQUAL(it.Carrier(), b5);
        ++it;
        BOOST_CHECK(!it);
    }
    BOOST_CHECK_EQUAL(vl.GetAllIntersection(), b1 & b5);
    BOOST_CHECK_EQUAL(vl.GetGreedyUnion(), b1 | b5);

}

}

//---------------------------------------------------------------------------
