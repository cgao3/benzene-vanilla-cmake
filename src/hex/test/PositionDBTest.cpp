//---------------------------------------------------------------------------
/** @file PositionDBTest.cpp
 */
//---------------------------------------------------------------------------
#include <boost/test/auto_unit_test.hpp>

#include "PositionDB.hpp"

using namespace benzene;

//---------------------------------------------------------------------------

namespace {

BOOST_AUTO_TEST_CASE(PositionDB_PositionSet)
{
    StoneBoard b1(3, 3, "Bbw"
                        ".Ww"
                        "..W");
    StoneBoard rb1(b1);
    rb1.RotateBoard();

    PositionSet set;
    BOOST_CHECK(!set.Exists(b1));

    set.Insert(b1);
    BOOST_CHECK(set.Exists(b1));
    BOOST_CHECK(set.Exists(rb1));
    
    StoneBoard b2(3, 3);
    BOOST_CHECK(!set.Exists(b2));
}

BOOST_AUTO_TEST_CASE(PositionDB_PositionMap)
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

    PositionMap<int> map;
    BOOST_CHECK(!map.Exists(b1));
    
    map[b1] = 5;
    BOOST_CHECK(map.Exists(b1));
    BOOST_CHECK(map.Exists(rb1));
    BOOST_CHECK(!map.Exists(b2));
    BOOST_CHECK(!map.Exists(rb2));    
    BOOST_CHECK_EQUAL(map[b1], 5);
    BOOST_CHECK_EQUAL(map[rb1], 5);

    map[rb2] = 1;
    BOOST_CHECK(map.Exists(b2));
    BOOST_CHECK(map.Exists(rb2));
    BOOST_CHECK_EQUAL(map[b1], 5);
    BOOST_CHECK_EQUAL(map[rb1], 5);
    BOOST_CHECK_EQUAL(map[b2], 1);
    BOOST_CHECK_EQUAL(map[rb2], 1);
}

}

//---------------------------------------------------------------------------
