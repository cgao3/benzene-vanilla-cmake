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

    // table is empty, so get better fail!
    BOOST_CHECK(!hm.get(1, data));

    // check put()/get()
    hm.put(1, 5);
    BOOST_CHECK(hm.get(1, data));
    BOOST_CHECK_EQUAL(data, 5);
    BOOST_CHECK(!hm.get(2, data));

    // check collision will not clobber values
    hm.put(33, 11);
    BOOST_CHECK(hm.get(1, data));
    BOOST_CHECK_EQUAL(data, 5);
    BOOST_CHECK(hm.get(33, data));
    BOOST_CHECK_EQUAL(data, 11);
}

}

//---------------------------------------------------------------------------
