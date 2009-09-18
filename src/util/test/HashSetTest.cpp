//---------------------------------------------------------------------------
/** @file HashSetTest.cpp
 */
//---------------------------------------------------------------------------

#include <boost/test/auto_unit_test.hpp>

#include "HashSet.hpp"

using namespace benzene;

//---------------------------------------------------------------------------

namespace {

BOOST_AUTO_TEST_CASE(HashSet_AllTests)
{
    HashSet hm(5);

    // check size()
    BOOST_CHECK_EQUAL(hm.size(), 32u);
    BOOST_CHECK_EQUAL(hm.count(), 0u);

    // table is empty, so get better fail!
    BOOST_CHECK(!hm.exists(1));

    // check add()/exists()
    hm.add(1);
    BOOST_CHECK(hm.exists(1));
    BOOST_CHECK(!hm.exists(2));
    BOOST_CHECK_EQUAL(hm.count(), 1u);

    // check collisions
    hm.add(33);
    BOOST_CHECK(hm.exists(1));
    BOOST_CHECK(hm.exists(33));
    BOOST_CHECK_EQUAL(hm.count(), 2u);

    HashSet mm(5);
    mm = hm;
    BOOST_CHECK(mm.exists(1));
    BOOST_CHECK(mm.exists(33));
    BOOST_CHECK_EQUAL(mm.count(), 2u);

    HashSet blah(mm);
    BOOST_CHECK(blah.exists(1));
    BOOST_CHECK(blah.exists(33));
    BOOST_CHECK_EQUAL(blah.count(), 2u);

    hm.clear();
    BOOST_CHECK(!hm.exists(1));
    BOOST_CHECK(!hm.exists(33));
    BOOST_CHECK_EQUAL(hm.count(), 0u);
}

}

//---------------------------------------------------------------------------
