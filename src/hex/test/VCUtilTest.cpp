//---------------------------------------------------------------------------
/** @file VCUtilTest.cpp */
//---------------------------------------------------------------------------
#include <boost/test/auto_unit_test.hpp>

#include "VCUtil.hpp"

using namespace benzene;

//---------------------------------------------------------------------------

namespace {

BOOST_AUTO_TEST_CASE(VCUtil_ValidEdgeBridge)
{
    StoneBoard brd(4, 4);
    brd.StartNewGame();

    HexPoint a1 = HEX_CELL_A1;
    HexPoint a2 = HEX_CELL_A2;
    HexPoint a3 = HEX_CELL_A3;
    HexPoint b1 = HEX_CELL_B1;
    HexPoint b2 = HEX_CELL_B2;
    HexPoint b3 = HEX_CELL_B3;

    HexPoint e,p;
    bitset_t carrier;
    carrier.set(a1);
    carrier.set(a2);

    BOOST_CHECK(VCUtil::ValidEdgeBridge(brd, carrier, p, e));
    BOOST_CHECK_EQUAL(e, WEST);
    BOOST_CHECK_EQUAL(p, b1);

    carrier.reset();
    carrier.set(a1);
    carrier.set(b1);
    BOOST_CHECK(VCUtil::ValidEdgeBridge(brd, carrier, p, e));
    BOOST_CHECK_EQUAL(e, NORTH);
    BOOST_CHECK_EQUAL(p, a2);
    
    carrier.reset();
    carrier.set(b1);
    carrier.set(b2);
    BOOST_CHECK(!VCUtil::ValidEdgeBridge(brd, carrier, p, e));
    
    carrier.reset();
    carrier.set(a1);
    carrier.set(b3);
    BOOST_CHECK(!VCUtil::ValidEdgeBridge(brd, carrier, p, e));

    brd.PlayMove(BLACK, a2);
    carrier.reset();
    carrier.set(a2);
    carrier.set(a3);
    BOOST_CHECK(!VCUtil::ValidEdgeBridge(brd, carrier, p, e));
    brd.UndoMove(a2);
}

}

//---------------------------------------------------------------------------
