#include <boost/test/auto_unit_test.hpp>

#include "Hex.hpp"

using namespace benzene;

//---------------------------------------------------------------------------

namespace {

BOOST_AUTO_TEST_CASE(HexColor_Iterator)
{
    BWIterator itColor;
    BOOST_CHECK(itColor);
    BOOST_CHECK_EQUAL(*itColor, BLACK);
    ++itColor;
    BOOST_CHECK(itColor);
    BOOST_CHECK_EQUAL(*itColor, WHITE);
    ++itColor;
    BOOST_CHECK(!itColor);
}

BOOST_AUTO_TEST_CASE(HexColor_CheckingValidityAndRange)
{
    BOOST_CHECK(HexColorUtil::isValidColor(BLACK));
    BOOST_CHECK(HexColorUtil::isValidColor(WHITE));
    BOOST_CHECK(HexColorUtil::isValidColor(EMPTY));
    BOOST_CHECK(HexColorUtil::isBlackWhite(BLACK));
    BOOST_CHECK(HexColorUtil::isBlackWhite(WHITE));
    BOOST_CHECK(!HexColorUtil::isBlackWhite(EMPTY));
}

BOOST_AUTO_TEST_CASE(HexColor_StringConversion)
{
    BOOST_CHECK_EQUAL(HexColorUtil::toString(BLACK), "black");
    BOOST_CHECK_EQUAL(HexColorUtil::toString(WHITE), "white");
    BOOST_CHECK_EQUAL(HexColorUtil::toString(EMPTY), "empty");
}

BOOST_AUTO_TEST_CASE(HexColor_GetComplement)
{
    BOOST_CHECK_EQUAL(HexColorUtil::otherColor(BLACK), WHITE);
    BOOST_CHECK_EQUAL(HexColorUtil::otherColor(WHITE), BLACK);
    BOOST_CHECK_EQUAL(HexColorUtil::otherColor(EMPTY), EMPTY);

    BOOST_CHECK_EQUAL(!BLACK, WHITE);
    BOOST_CHECK_EQUAL(!WHITE, BLACK);
    BOOST_CHECK_EQUAL(!EMPTY, EMPTY);
    
    for (ColorIterator color; color; ++color) {
        BOOST_CHECK_EQUAL(!(*color), HexColorUtil::otherColor(*color));
    }
}

BOOST_AUTO_TEST_CASE(HexColorSet_CheckingValidity)
{
    BOOST_CHECK(HexColorSetUtil::isValid(BLACK_ONLY));
    BOOST_CHECK(HexColorSetUtil::isValid(WHITE_ONLY));
    BOOST_CHECK(HexColorSetUtil::isValid(EMPTY_ONLY));
    BOOST_CHECK(HexColorSetUtil::isValid(NOT_BLACK));
    BOOST_CHECK(HexColorSetUtil::isValid(NOT_WHITE));
    BOOST_CHECK(HexColorSetUtil::isValid(NOT_EMPTY));
    BOOST_CHECK(HexColorSetUtil::isValid(ALL_COLORS));
}

BOOST_AUTO_TEST_CASE(HexColorSet_StringConversion)
{
    BOOST_CHECK_EQUAL(HexColorSetUtil::toString(BLACK_ONLY), "black_only");
    BOOST_CHECK_EQUAL(HexColorSetUtil::toString(WHITE_ONLY), "white_only");
    BOOST_CHECK_EQUAL(HexColorSetUtil::toString(EMPTY_ONLY), "empty_only");
    BOOST_CHECK_EQUAL(HexColorSetUtil::toString(NOT_BLACK), "not_black");
    BOOST_CHECK_EQUAL(HexColorSetUtil::toString(NOT_WHITE), "not_white");
    BOOST_CHECK_EQUAL(HexColorSetUtil::toString(NOT_EMPTY), "not_empty");
    BOOST_CHECK_EQUAL(HexColorSetUtil::toString(ALL_COLORS), "all_colors");
    BOOST_CHECK_EQUAL(HexColorSetUtil::fromString("black_only"), BLACK_ONLY);
    BOOST_CHECK_EQUAL(HexColorSetUtil::fromString("white_only"), WHITE_ONLY);
    BOOST_CHECK_EQUAL(HexColorSetUtil::fromString("empty_only"), EMPTY_ONLY);
    BOOST_CHECK_EQUAL(HexColorSetUtil::fromString("not_black"), NOT_BLACK);
    BOOST_CHECK_EQUAL(HexColorSetUtil::fromString("not_white"), NOT_WHITE);
    BOOST_CHECK_EQUAL(HexColorSetUtil::fromString("not_empty"), NOT_EMPTY);
    BOOST_CHECK_EQUAL(HexColorSetUtil::fromString("all_colors"), ALL_COLORS);
}

BOOST_AUTO_TEST_CASE(HexColorSet_CheckingInclusion)
{
    BOOST_CHECK( HexColorSetUtil::InSet(BLACK, BLACK_ONLY));
    BOOST_CHECK(!HexColorSetUtil::InSet(WHITE, BLACK_ONLY));
    BOOST_CHECK(!HexColorSetUtil::InSet(EMPTY, BLACK_ONLY));
    BOOST_CHECK(!HexColorSetUtil::InSet(BLACK, WHITE_ONLY));
    BOOST_CHECK( HexColorSetUtil::InSet(WHITE, WHITE_ONLY));
    BOOST_CHECK(!HexColorSetUtil::InSet(EMPTY, WHITE_ONLY));
    BOOST_CHECK(!HexColorSetUtil::InSet(BLACK, EMPTY_ONLY));
    BOOST_CHECK( HexColorSetUtil::InSet(WHITE, NOT_BLACK));
    BOOST_CHECK(!HexColorSetUtil::InSet(WHITE, NOT_WHITE));
    BOOST_CHECK( HexColorSetUtil::InSet(EMPTY, NOT_WHITE));
    BOOST_CHECK( HexColorSetUtil::InSet(BLACK, NOT_EMPTY));
    BOOST_CHECK( HexColorSetUtil::InSet(WHITE, NOT_EMPTY));
    BOOST_CHECK(!HexColorSetUtil::InSet(EMPTY, NOT_EMPTY));
    BOOST_CHECK( HexColorSetUtil::InSet(BLACK, ALL_COLORS));
    BOOST_CHECK( HexColorSetUtil::InSet(WHITE, ALL_COLORS));
    BOOST_CHECK( HexColorSetUtil::InSet(EMPTY, ALL_COLORS));
}

BOOST_AUTO_TEST_CASE(HexColorSet_ColorToColorSet)
{
    BOOST_CHECK_EQUAL(HexColorSetUtil::Only(BLACK), BLACK_ONLY);
    BOOST_CHECK_EQUAL(HexColorSetUtil::Only(WHITE), WHITE_ONLY);
    BOOST_CHECK_EQUAL(HexColorSetUtil::Only(EMPTY), EMPTY_ONLY);
    BOOST_CHECK_EQUAL(HexColorSetUtil::NotColor(BLACK), NOT_BLACK);
    BOOST_CHECK_EQUAL(HexColorSetUtil::NotColor(WHITE), NOT_WHITE);
    BOOST_CHECK_EQUAL(HexColorSetUtil::NotColor(EMPTY), NOT_EMPTY);
    BOOST_CHECK_EQUAL(HexColorSetUtil::ColorOrEmpty(BLACK), NOT_WHITE);
    BOOST_CHECK_EQUAL(HexColorSetUtil::ColorOrEmpty(WHITE), NOT_BLACK);
    BOOST_CHECK_EQUAL(HexColorSetUtil::ColorOrEmpty(EMPTY), EMPTY_ONLY);
}

}

//---------------------------------------------------------------------------
