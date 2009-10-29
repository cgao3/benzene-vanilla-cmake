//---------------------------------------------------------------------------
/** @file
 */
//---------------------------------------------------------------------------
#include <boost/test/auto_unit_test.hpp>

#include "VC.hpp"

using namespace benzene;

//---------------------------------------------------------------------------

namespace {

BOOST_AUTO_TEST_CASE(VC_Construction)
{
    HexPoint x = HEX_CELL_A1;
    HexPoint y = HEX_CELL_A2;
    VC a(x,y);
    BOOST_CHECK_EQUAL(a.x(), x);
    BOOST_CHECK_EQUAL(a.y(), y);
}

}

//---------------------------------------------------------------------------
