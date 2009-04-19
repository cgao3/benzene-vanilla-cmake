//---------------------------------------------------------------------------
/** @file
 */
//---------------------------------------------------------------------------

#include <boost/test/auto_unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include "HashTable.hpp"

using namespace benzene;

//---------------------------------------------------------------------------

namespace {

BOOST_AUTO_TEST_CASE(HashTable_AllTests)
{
    HashTable<int> hm(5);

    // check size
    BOOST_CHECK_EQUAL(hm.size(), 32u);

    // check read on empty entry is 0.0f
    BOOST_CHECK_EQUAL(hm[13], 0);

    // check write
    hm[13] = 397;
    BOOST_CHECK_EQUAL(hm[13], 397);

    // check overwriting entry
    hm[13] = 315;
    BOOST_CHECK_EQUAL(hm[13], 315);

    // check first and last entries
    BOOST_CHECK_EQUAL(hm.size(), 32u);
    BOOST_CHECK_EQUAL(hm[0], 0);
    BOOST_CHECK_EQUAL(hm[31], 0);

    // check handles access outside of range correctly
    BOOST_CHECK_EQUAL(hm[45], 315);
}

}

//---------------------------------------------------------------------------
