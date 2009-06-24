//---------------------------------------------------------------------------
/** @file HashMapTest.cpp
 */
//---------------------------------------------------------------------------

#include <boost/test/auto_unit_test.hpp>

#include "HashMap.hpp"

using namespace benzene;

//---------------------------------------------------------------------------

namespace {

BOOST_AUTO_TEST_CASE(HashMap_AllTests)
{
    int data = 0;
    HashMap<int> hm(5);

    // check size()
    BOOST_CHECK_EQUAL(hm.size(), 32u);
    BOOST_CHECK_EQUAL(hm.count(), 0u);

    // table is empty, so get better fail!
    BOOST_CHECK(!hm.get(1, data));

    // check put()/get()
    hm.put(1, 5);
    BOOST_CHECK(hm.get(1, data));
    BOOST_CHECK_EQUAL(data, 5);
    BOOST_CHECK(!hm.get(2, data));
    BOOST_CHECK_EQUAL(hm.count(), 1u);

    // check collision will not clobber values (33 = 1 mod 32)
    hm.put(33, 11);
    BOOST_CHECK(hm.get(1, data));
    BOOST_CHECK_EQUAL(data, 5);
    BOOST_CHECK(hm.get(33, data));
    BOOST_CHECK_EQUAL(data, 11);
    BOOST_CHECK_EQUAL(hm.count(), 2u);

    HashMap<int> mm(5);
    mm = hm;
    BOOST_CHECK(mm.get(1, data));
    BOOST_CHECK_EQUAL(data, 5);
    BOOST_CHECK(mm.get(33, data));
    BOOST_CHECK_EQUAL(data, 11);
    BOOST_CHECK_EQUAL(mm.count(), 2u);

    HashMap<int> blah(mm);
    BOOST_CHECK(blah.get(1, data));
    BOOST_CHECK_EQUAL(data, 5);
    BOOST_CHECK(blah.get(33, data));
    BOOST_CHECK_EQUAL(data, 11);
    BOOST_CHECK_EQUAL(blah.count(), 2u);
    
}

}

//---------------------------------------------------------------------------
