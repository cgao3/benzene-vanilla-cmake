//---------------------------------------------------------------------------
/** @file
 */
//---------------------------------------------------------------------------

#include <boost/test/auto_unit_test.hpp>

#include "Hash.hpp"

using namespace benzene;

//---------------------------------------------------------------------------

namespace {

BOOST_AUTO_TEST_CASE(Hash_BasicTests)
{
    BOOST_CHECK_EQUAL(HashUtil::toString(0),       "0x0000000000000000");
    BOOST_CHECK_EQUAL(HashUtil::toString(1),       "0x0000000000000001");
    BOOST_CHECK_EQUAL(HashUtil::toString(9),       "0x0000000000000009");
    BOOST_CHECK_EQUAL(HashUtil::toString(10),      "0x000000000000000a");
    BOOST_CHECK_EQUAL(HashUtil::toString(15),      "0x000000000000000f");
    BOOST_CHECK_EQUAL(HashUtil::toString(18),      "0x0000000000000012");
    BOOST_CHECK_EQUAL(HashUtil::toString(4736785), "0x0000000000484711");
}

}

//---------------------------------------------------------------------------
