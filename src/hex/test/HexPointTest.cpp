//---------------------------------------------------------------------------
// $Id: HexPointTest.cpp 1657 2008-09-15 23:32:09Z broderic $
//---------------------------------------------------------------------------

#include <boost/test/auto_unit_test.hpp>

#include "Hex.hpp"

//---------------------------------------------------------------------------

namespace {

BOOST_AUTO_UNIT_TEST(HexPoint_NecessaryPointOrdering)
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

BOOST_AUTO_UNIT_TEST(HexPoint_StringConversion)
{
    BOOST_CHECK_EQUAL(HexPointUtil::toString(INVALID_POINT), "invalid");
    BOOST_CHECK_EQUAL(HexPointUtil::toString(RESIGN), "resign");
    BOOST_CHECK_EQUAL(HexPointUtil::toString(SWAP_PIECES), "swap-pieces");
    BOOST_CHECK_EQUAL(HexPointUtil::toString(NORTH), "north");
    BOOST_CHECK_EQUAL(HexPointUtil::toString(SOUTH), "south");
    BOOST_CHECK_EQUAL(HexPointUtil::toString(EAST), "east");
    BOOST_CHECK_EQUAL(HexPointUtil::toString(WEST), "west");
    BOOST_CHECK_EQUAL(HexPointUtil::toString(FIRST_CELL), "a1");

    // a smattering to ensure the constants a sync'd with the
    // strings in HexPointInit().
    BOOST_CHECK_EQUAL(HexPointUtil::toString(HEX_CELL_A1),  "a1");
    BOOST_CHECK_EQUAL(HexPointUtil::toString(HEX_CELL_A2),  "a2");
    BOOST_CHECK_EQUAL(HexPointUtil::toString(HEX_CELL_A3),  "a3");
    BOOST_CHECK_EQUAL(HexPointUtil::toString(HEX_CELL_A4),  "a4");
    BOOST_CHECK_EQUAL(HexPointUtil::toString(HEX_CELL_A5),  "a5");
    BOOST_CHECK_EQUAL(HexPointUtil::toString(HEX_CELL_A6),  "a6");
    BOOST_CHECK_EQUAL(HexPointUtil::toString(HEX_CELL_A7),  "a7");
    BOOST_CHECK_EQUAL(HexPointUtil::toString(HEX_CELL_A8),  "a8");
    BOOST_CHECK_EQUAL(HexPointUtil::toString(HEX_CELL_A9),  "a9");
    BOOST_CHECK_EQUAL(HexPointUtil::toString(HEX_CELL_A10), "a10");
    BOOST_CHECK_EQUAL(HexPointUtil::toString(HEX_CELL_A11), "a11");
    BOOST_CHECK_EQUAL(HexPointUtil::toString(HEX_CELL_B1),  "b1");
    BOOST_CHECK_EQUAL(HexPointUtil::toString(HEX_CELL_C1),  "c1");
    BOOST_CHECK_EQUAL(HexPointUtil::toString(HEX_CELL_C5),  "c5");
    BOOST_CHECK_EQUAL(HexPointUtil::toString(HEX_CELL_D1),  "d1");
    BOOST_CHECK_EQUAL(HexPointUtil::toString(HEX_CELL_F7),  "f7");
    BOOST_CHECK_EQUAL(HexPointUtil::toString(HEX_CELL_I1),  "i1");
    BOOST_CHECK_EQUAL(HexPointUtil::toString(HEX_CELL_K1),  "k1");
    BOOST_CHECK_EQUAL(HexPointUtil::toString(HEX_CELL_K11), "k11");
    
    
    BOOST_CHECK_EQUAL(HexPointUtil::fromString("invalid"), INVALID_POINT);
    BOOST_CHECK_EQUAL(HexPointUtil::fromString("resign"), RESIGN);
    BOOST_CHECK_EQUAL(HexPointUtil::fromString("swap-pieces"), SWAP_PIECES);
    BOOST_CHECK_EQUAL(HexPointUtil::fromString("north"), NORTH);
    BOOST_CHECK_EQUAL(HexPointUtil::fromString("south"), SOUTH);
    BOOST_CHECK_EQUAL(HexPointUtil::fromString("east"), EAST);
    BOOST_CHECK_EQUAL(HexPointUtil::fromString("west"), WEST);
    BOOST_CHECK_EQUAL(HexPointUtil::fromString("a1"), FIRST_CELL);
    
    BOOST_CHECK_EQUAL(HexPointUtil::fromString
                      (HexPointUtil::toString(HEX_CELL_E7)), 
                      HEX_CELL_E7);
}

BOOST_AUTO_UNIT_TEST(HexPoint_Swap)
{
    BOOST_CHECK(!HexPointUtil::isSwap(INVALID_POINT));
    BOOST_CHECK(!HexPointUtil::isSwap(RESIGN));
    BOOST_CHECK( HexPointUtil::isSwap(SWAP_PIECES));
    BOOST_CHECK(!HexPointUtil::isSwap(FIRST_EDGE));
    BOOST_CHECK(!HexPointUtil::isSwap(FIRST_INVALID));
}

BOOST_AUTO_UNIT_TEST(HexPoint_Edges)
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

BOOST_AUTO_UNIT_TEST(HexPoint_CoordsConversion)
{
    int x, y;
    HexPointUtil::pointToCoords(FIRST_CELL, x, y);
    BOOST_CHECK_EQUAL(x, 0);
    BOOST_CHECK_EQUAL(y, 0);
    BOOST_CHECK_EQUAL(HexPointUtil::coordsToPoint(x, y), FIRST_CELL);

    HexPointUtil::pointToCoords(HEX_CELL_K11, x, y);
    BOOST_CHECK_EQUAL(x, MAX_WIDTH - 1);
    BOOST_CHECK_EQUAL(y, MAX_HEIGHT - 1);
    BOOST_CHECK_EQUAL(HexPointUtil::coordsToPoint(x, y), HEX_CELL_K11);
    
    BOOST_REQUIRE(MAX_WIDTH >= 2 && MAX_HEIGHT >= 2);
    BOOST_CHECK_EQUAL(HexPointUtil::coordsToPoint(0, 1), HEX_CELL_A2);
    BOOST_CHECK_EQUAL(HexPointUtil::coordsToPoint(1, 0), HEX_CELL_B1);
    BOOST_CHECK_EQUAL(HexPointUtil::coordsToPoint(1, 1), HEX_CELL_B2);
}

BOOST_AUTO_UNIT_TEST(HexPoint_DirectionalDeltaXY)
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
