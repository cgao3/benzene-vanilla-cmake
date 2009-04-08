//---------------------------------------------------------------------------
// $Id: VCUtilsTest.cpp 1536 2008-07-09 22:47:27Z broderic $
//---------------------------------------------------------------------------
#include <boost/test/auto_unit_test.hpp>

#include "VCUtils.hpp"

//---------------------------------------------------------------------------

namespace {

BOOST_AUTO_UNIT_TEST(VCUtils_ValidEdgeBridge)
{
    StoneBoard brd(4, 4);
    brd.startNewGame();

    HexPoint a1 = HexPointUtil::fromString("a1");
    HexPoint a2 = HexPointUtil::fromString("a2");
    HexPoint a3 = HexPointUtil::fromString("a3");
    HexPoint a4 = HexPointUtil::fromString("a4");
    HexPoint b1 = HexPointUtil::fromString("b1");
    HexPoint b2 = HexPointUtil::fromString("b2");
    HexPoint b3 = HexPointUtil::fromString("b3");
    HexPoint b4 = HexPointUtil::fromString("b4");

    HexPoint e,p;
    bitset_t carrier;
    carrier.set(a1);
    carrier.set(a2);

    BOOST_CHECK(VCUtils::ValidEdgeBridge(brd, carrier, p, e));
    BOOST_CHECK_EQUAL(e, WEST);
    BOOST_CHECK_EQUAL(p, b1);

    carrier.reset();
    carrier.set(a1);
    carrier.set(b1);
    BOOST_CHECK(VCUtils::ValidEdgeBridge(brd, carrier, p, e));
    BOOST_CHECK_EQUAL(e, NORTH);
    BOOST_CHECK_EQUAL(p, a2);
    
    carrier.reset();
    carrier.set(b1);
    carrier.set(b2);
    BOOST_CHECK(!VCUtils::ValidEdgeBridge(brd, carrier, p, e));
    
    carrier.reset();
    carrier.set(a1);
    carrier.set(b3);
    BOOST_CHECK(!VCUtils::ValidEdgeBridge(brd, carrier, p, e));

    brd.playMove(BLACK, a2);
    carrier.reset();
    carrier.set(a2);
    carrier.set(a3);
    BOOST_CHECK(!VCUtils::ValidEdgeBridge(brd, carrier, p, e));
    brd.undoMove(a2);
    
}

}

//---------------------------------------------------------------------------
