//---------------------------------------------------------------------------
/** @file HexPointTest.cpp */
//---------------------------------------------------------------------------

#include <boost/test/auto_unit_test.hpp>

#include "Hex.hpp"

using namespace benzene;

//---------------------------------------------------------------------------

namespace {

BOOST_AUTO_TEST_CASE(HexPoint_NecessaryPointOrdering)
{
    BOOST_CHECK(!INVALID_POINT);
    BOOST_CHECK(FIRST_SPECIAL < FIRST_EDGE);
    BOOST_CHECK(RESIGN < FIRST_EDGE);
    BOOST_CHECK(SWAP_PIECES < FIRST_EDGE);
    BOOST_CHECK(FIRST_EDGE < FIRST_CELL);
    BOOST_CHECK(NORTH < FIRST_CELL);
    BOOST_CHECK(SOUTH < FIRST_CELL);
    BOOST_CHECK(EAST < FIRST_CELL);
    BOOST_CHECK(WEST < FIRST_CELL);
    BOOST_CHECK(FIRST_CELL < FIRST_INVALID);
    BOOST_CHECK(FIRST_INVALID <= BITSETSIZE);
}

BOOST_AUTO_TEST_CASE(HexPoint_StringConversion)
{
    BOOST_CHECK_EQUAL(HexPointUtil::ToString(INVALID_POINT), "invalid");
    BOOST_CHECK_EQUAL(HexPointUtil::ToString(RESIGN), "resign");
    BOOST_CHECK_EQUAL(HexPointUtil::ToString(SWAP_PIECES), "swap-pieces");
    BOOST_CHECK_EQUAL(HexPointUtil::ToString(NORTH), "north");
    BOOST_CHECK_EQUAL(HexPointUtil::ToString(SOUTH), "south");
    BOOST_CHECK_EQUAL(HexPointUtil::ToString(EAST), "east");
    BOOST_CHECK_EQUAL(HexPointUtil::ToString(WEST), "west");
    BOOST_CHECK_EQUAL(HexPointUtil::ToString(FIRST_CELL), "a1");

    // a smattering to ensure the constants a sync'd with the
    // strings in HexPointInit().
    BOOST_CHECK_EQUAL(HexPointUtil::ToString(HEX_CELL_A1),  "a1");
    BOOST_CHECK_EQUAL(HexPointUtil::ToString(HEX_CELL_A2),  "a2");
    BOOST_CHECK_EQUAL(HexPointUtil::ToString(HEX_CELL_A3),  "a3");
    BOOST_CHECK_EQUAL(HexPointUtil::ToString(HEX_CELL_A4),  "a4");
    BOOST_CHECK_EQUAL(HexPointUtil::ToString(HEX_CELL_A5),  "a5");
    BOOST_CHECK_EQUAL(HexPointUtil::ToString(HEX_CELL_A6),  "a6");
    BOOST_CHECK_EQUAL(HexPointUtil::ToString(HEX_CELL_A7),  "a7");
    BOOST_CHECK_EQUAL(HexPointUtil::ToString(HEX_CELL_A8),  "a8");
    BOOST_CHECK_EQUAL(HexPointUtil::ToString(HEX_CELL_A9),  "a9");
    BOOST_CHECK_EQUAL(HexPointUtil::ToString(HEX_CELL_A10), "a10");
    BOOST_CHECK_EQUAL(HexPointUtil::ToString(HEX_CELL_A11), "a11");
    BOOST_CHECK_EQUAL(HexPointUtil::ToString(HEX_CELL_B1),  "b1");
    BOOST_CHECK_EQUAL(HexPointUtil::ToString(HEX_CELL_C1),  "c1");
    BOOST_CHECK_EQUAL(HexPointUtil::ToString(HEX_CELL_C5),  "c5");
    BOOST_CHECK_EQUAL(HexPointUtil::ToString(HEX_CELL_D1),  "d1");
    BOOST_CHECK_EQUAL(HexPointUtil::ToString(HEX_CELL_F7),  "f7");
    BOOST_CHECK_EQUAL(HexPointUtil::ToString(HEX_CELL_I1),  "i1");
    BOOST_CHECK_EQUAL(HexPointUtil::ToString(HEX_CELL_K1),  "k1");
    BOOST_CHECK_EQUAL(HexPointUtil::ToString(HEX_CELL_K11), "k11");
    
    
    BOOST_CHECK_EQUAL(HexPointUtil::FromString("invalid"), INVALID_POINT);
    BOOST_CHECK_EQUAL(HexPointUtil::FromString("resign"), RESIGN);
    BOOST_CHECK_EQUAL(HexPointUtil::FromString("swap-pieces"), SWAP_PIECES);
    BOOST_CHECK_EQUAL(HexPointUtil::FromString("north"), NORTH);
    BOOST_CHECK_EQUAL(HexPointUtil::FromString("south"), SOUTH);
    BOOST_CHECK_EQUAL(HexPointUtil::FromString("east"), EAST);
    BOOST_CHECK_EQUAL(HexPointUtil::FromString("west"), WEST);
    BOOST_CHECK_EQUAL(HexPointUtil::FromString("a1"), FIRST_CELL);
    
    BOOST_CHECK_EQUAL(HexPointUtil::FromString
                      (HexPointUtil::ToString(HEX_CELL_E7)), 
                      HEX_CELL_E7);
}

BOOST_AUTO_TEST_CASE(HexPoint_StringConversionSequence)
{
    std::string blah = "  a1  a2 a3 a4   a5\ta6 a7\n";
    PointSequence pts;
    HexPointUtil::FromString(blah, pts);
    BOOST_CHECK_EQUAL(pts.size(), 7u);
    BOOST_CHECK_EQUAL(pts[0], HEX_CELL_A1);
    BOOST_CHECK_EQUAL(pts[1], HEX_CELL_A2);
    BOOST_CHECK_EQUAL(pts[2], HEX_CELL_A3);
    BOOST_CHECK_EQUAL(pts[3], HEX_CELL_A4);
    BOOST_CHECK_EQUAL(pts[4], HEX_CELL_A5);
    BOOST_CHECK_EQUAL(pts[5], HEX_CELL_A6);
    BOOST_CHECK_EQUAL(pts[6], HEX_CELL_A7);
}

BOOST_AUTO_TEST_CASE(HexPoint_Swap)
{
    BOOST_CHECK(!HexPointUtil::isSwap(INVALID_POINT));
    BOOST_CHECK(!HexPointUtil::isSwap(RESIGN));
    BOOST_CHECK( HexPointUtil::isSwap(SWAP_PIECES));
    BOOST_CHECK(!HexPointUtil::isSwap(FIRST_EDGE));
    BOOST_CHECK(!HexPointUtil::isSwap(FIRST_INVALID));
}

BOOST_AUTO_TEST_CASE(HexPoint_Edges)
{
    BOOST_CHECK(!HexPointUtil::isEdge(INVALID_POINT));
    BOOST_CHECK(!HexPointUtil::isEdge(RESIGN));
    BOOST_CHECK(!HexPointUtil::isEdge(SWAP_PIECES));
    BOOST_CHECK( HexPointUtil::isEdge(FIRST_EDGE));
    BOOST_CHECK(!HexPointUtil::isEdge(FIRST_INVALID));
    BOOST_CHECK( HexPointUtil::isEdge(NORTH));
    BOOST_CHECK( HexPointUtil::isEdge(SOUTH));
    BOOST_CHECK( HexPointUtil::isEdge(WEST));
    BOOST_CHECK( HexPointUtil::isEdge(EAST));
    
    BOOST_CHECK_EQUAL(HexPointUtil::oppositeEdge(EAST), WEST);
    BOOST_CHECK_EQUAL(HexPointUtil::oppositeEdge(WEST), EAST);
    BOOST_CHECK_EQUAL(HexPointUtil::oppositeEdge(NORTH), SOUTH);
    BOOST_CHECK_EQUAL(HexPointUtil::oppositeEdge(SOUTH), NORTH);
    
    BOOST_CHECK_EQUAL(HexPointUtil::leftEdge(EAST), HexPointUtil::rightEdge(WEST));
    BOOST_CHECK_EQUAL(HexPointUtil::rightEdge(EAST), HexPointUtil::leftEdge(WEST));
    BOOST_CHECK_EQUAL(HexPointUtil::leftEdge(HexPointUtil::leftEdge(EAST)), WEST);
    BOOST_CHECK_EQUAL(HexPointUtil::leftEdge(SOUTH), WEST);
    BOOST_CHECK_EQUAL(HexPointUtil::rightEdge(SOUTH), EAST);
    BOOST_CHECK_EQUAL(HexPointUtil::rightEdge(NORTH), WEST);
    
    BOOST_CHECK_EQUAL(HexPointUtil::oppositeEdge(HexPointUtil::colorEdge1(BLACK)),
		      HexPointUtil::colorEdge2(BLACK));
    BOOST_CHECK_EQUAL(HexPointUtil::oppositeEdge(HexPointUtil::colorEdge1(WHITE)),
		      HexPointUtil::colorEdge2(WHITE));
    
    BOOST_CHECK(HexPointUtil::isColorEdge(NORTH, BLACK));
    BOOST_CHECK(HexPointUtil::isColorEdge(EAST, WHITE));
    BOOST_CHECK(HexPointUtil::isColorEdge(HexPointUtil::rightEdge
					  (HexPointUtil::colorEdge2(BLACK)), WHITE));
    BOOST_CHECK(HexPointUtil::isColorEdge(HexPointUtil::colorEdge1(BLACK), BLACK));
    BOOST_CHECK(HexPointUtil::isColorEdge(HexPointUtil::colorEdge2(BLACK), BLACK));
    BOOST_CHECK(HexPointUtil::isColorEdge(HexPointUtil::colorEdge1(WHITE), WHITE));
    BOOST_CHECK(HexPointUtil::isColorEdge(HexPointUtil::colorEdge2(WHITE), WHITE));
}

BOOST_AUTO_TEST_CASE(HexPoint_CoordsConversion)
{
    int x, y;
    HexPointUtil::pointToCoords(FIRST_CELL, x, y);
    BOOST_CHECK_EQUAL(x, 0);
    BOOST_CHECK_EQUAL(y, 0);
    BOOST_CHECK_EQUAL(HexPointUtil::coordsToPoint(x, y), FIRST_CELL);

    HexPointUtil::pointToCoords(LAST_CELL, x, y);
    BOOST_CHECK_EQUAL(x, MAX_WIDTH - 1);
    BOOST_CHECK_EQUAL(y, MAX_HEIGHT - 1);
    BOOST_CHECK_EQUAL(HexPointUtil::coordsToPoint(x, y), LAST_CELL);
    
    BOOST_REQUIRE(MAX_WIDTH >= 2 && MAX_HEIGHT >= 2);
    BOOST_CHECK_EQUAL(HexPointUtil::coordsToPoint(0, 1), HEX_CELL_A2);
    BOOST_CHECK_EQUAL(HexPointUtil::coordsToPoint(1, 0), HEX_CELL_B1);
    BOOST_CHECK_EQUAL(HexPointUtil::coordsToPoint(1, 1), HEX_CELL_B2);
}

BOOST_AUTO_TEST_CASE(HexPoint_DirectionalDeltaXY)
{
    BOOST_CHECK_EQUAL(HexPointUtil::DeltaX(DIR_EAST), 1);
    BOOST_CHECK_EQUAL(HexPointUtil::DeltaY(DIR_EAST), 0);
    BOOST_CHECK_EQUAL(HexPointUtil::DeltaX(DIR_NORTH_EAST), 1);
    BOOST_CHECK_EQUAL(HexPointUtil::DeltaY(DIR_NORTH_EAST), -1);
    BOOST_CHECK_EQUAL(HexPointUtil::DeltaX(DIR_NORTH), 0);
    BOOST_CHECK_EQUAL(HexPointUtil::DeltaY(DIR_NORTH), -1);
    BOOST_CHECK_EQUAL(HexPointUtil::DeltaX(DIR_WEST), -1);
    BOOST_CHECK_EQUAL(HexPointUtil::DeltaY(DIR_WEST), 0);
    BOOST_CHECK_EQUAL(HexPointUtil::DeltaX(DIR_SOUTH_WEST), -1);
    BOOST_CHECK_EQUAL(HexPointUtil::DeltaY(DIR_SOUTH_WEST), 1);
    BOOST_CHECK_EQUAL(HexPointUtil::DeltaX(DIR_SOUTH), 0);
    BOOST_CHECK_EQUAL(HexPointUtil::DeltaY(DIR_SOUTH), 1);
}

}

//---------------------------------------------------------------------------
