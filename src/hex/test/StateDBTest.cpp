//---------------------------------------------------------------------------
/** @file StateDBTest.cpp */
//---------------------------------------------------------------------------
#include <boost/test/auto_unit_test.hpp>

#include "SgSystem.h"
#include "StateDB.hpp"

using namespace benzene;

//---------------------------------------------------------------------------

namespace {

BOOST_AUTO_TEST_CASE(StateDB_StateSet)
{
    StoneBoard b1(3, 3, "Bbw"
                        ".Ww"
                        "..W");
    StoneBoard rb1(b1);
    rb1.RotateBoard();

    HexState sb1(b1, BLACK);
    HexState srb1(rb1, BLACK);

    StateSet set;
    BOOST_CHECK(!set.Exists(sb1));

    set.Insert(sb1);
    BOOST_CHECK(set.Exists(sb1));
    BOOST_CHECK(set.Exists(srb1));
    
    StoneBoard b2(3, 3);
    HexState sb2(b2, BLACK);
    BOOST_CHECK(!set.Exists(sb2));
}

BOOST_AUTO_TEST_CASE(StateDB_StateMap)
{
    StoneBoard b1(3, 3, "Bbw"
                        ".Ww"
                        "..W");
    StoneBoard rb1(b1);
    rb1.RotateBoard();
    StoneBoard b2(3, 5, "Bbw"
                        ".Ww"
                        "..W"
                        "..."
                        "...");
    StoneBoard rb2(b2);
    rb2.RotateBoard();

    HexState sb1(b1, BLACK);
    HexState sb2(b2, BLACK);
    HexState srb1(rb1, BLACK);
    HexState srb2(rb2, BLACK);

    StateMap<int> map;
    BOOST_CHECK(!map.Exists(sb1));
    
    map[sb1] = 5;
    BOOST_CHECK(map.Exists(sb1));
    BOOST_CHECK(map.Exists(srb1));
    BOOST_CHECK(!map.Exists(sb2));
    BOOST_CHECK(!map.Exists(srb2));    
    BOOST_CHECK_EQUAL(map[sb1], 5);
    BOOST_CHECK_EQUAL(map[srb1], 5);

    map[srb2] = 1;
    BOOST_CHECK(map.Exists(sb2));
    BOOST_CHECK(map.Exists(srb2));
    BOOST_CHECK_EQUAL(map[sb1], 5);
    BOOST_CHECK_EQUAL(map[srb1], 5);
    BOOST_CHECK_EQUAL(map[sb2], 1);
    BOOST_CHECK_EQUAL(map[srb2], 1);
}

}

//---------------------------------------------------------------------------
