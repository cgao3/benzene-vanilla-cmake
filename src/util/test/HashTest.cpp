//---------------------------------------------------------------------------
/** @file
 */
//---------------------------------------------------------------------------

#include <boost/test/auto_unit_test.hpp>

#include "Hash.hpp"

//---------------------------------------------------------------------------

namespace {

BOOST_AUTO_TEST_CASE(Hash_BasicTests)
{
    BOOST_CHECK_EQUAL(HashUtil::toString(0), "0x0");
    BOOST_CHECK_EQUAL(HashUtil::toString(1), "0x1");
    BOOST_CHECK_EQUAL(HashUtil::toString(9), "0x9");
    BOOST_CHECK_EQUAL(HashUtil::toString(10), "0xa");
    BOOST_CHECK_EQUAL(HashUtil::toString(15), "0xf");
    BOOST_CHECK_EQUAL(HashUtil::toString(18), "0x12");
    BOOST_CHECK_EQUAL(HashUtil::toString(4736785), "0x484711");
}

}

//---------------------------------------------------------------------------
