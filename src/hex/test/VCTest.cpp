//---------------------------------------------------------------------------
// $Id: VCTest.cpp 571 2007-07-24 20:24:15Z broderic $
//---------------------------------------------------------------------------
#include <boost/test/auto_unit_test.hpp>

#include "VC.hpp"

//---------------------------------------------------------------------------

namespace {

BOOST_AUTO_UNIT_TEST(VC_Construction)
{
    HexPoint x = HexPointUtil::fromString("a1");
    HexPoint y = HexPointUtil::fromString("a2");
    VC a(x,y);
    BOOST_CHECK_EQUAL(a.x(), x);
    BOOST_CHECK_EQUAL(a.y(), y);
}

}

//---------------------------------------------------------------------------
