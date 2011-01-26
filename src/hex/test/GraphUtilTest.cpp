//----------------------------------------------------------------------------
/** @file GraphUtilTest.cpp */
//----------------------------------------------------------------------------

#include <boost/test/auto_unit_test.hpp>

#include "SgSystem.h"
#include "GraphUtil.hpp"

using namespace benzene;

//---------------------------------------------------------------------------

namespace {

BOOST_AUTO_TEST_CASE(GraphUtil_ComputeDigraph)
{
    BOOST_REQUIRE(MAX_WIDTH >= 5 && MAX_HEIGHT >= 5);
    StoneBoard gb(5, 5);
    gb.StartNewGame();

    //  a  b  c  d  e  
    // 1\.  .  .  .  .\1
    //  2\W  B  .  .  .\2
    //   3\.  B  .  .  .\3
    //    4\.  .  .  .  .\4
    //     5\.  .  .  .  .\5
    //        a  b  c  d  e  
    HexPoint a1 = HEX_CELL_A1;
    HexPoint b1 = HEX_CELL_B1;
    HexPoint c1 = HEX_CELL_C1;   
    HexPoint a2 = HEX_CELL_A2;
    HexPoint b2 = HEX_CELL_B2;
    HexPoint c2 = HEX_CELL_C2;
    HexPoint a3 = HEX_CELL_A3;
    HexPoint b3 = HEX_CELL_B3;  
    HexPoint c3 = HEX_CELL_C3;
    HexPoint a4 = HEX_CELL_A4;
    HexPoint b4 = HEX_CELL_B4;
    HexPoint a5 = HEX_CELL_A5;    

    bitset_t nbs;    
    gb.PlayMove(BLACK, b2);
    gb.PlayMove(WHITE, a2);
    gb.PlayMove(BLACK, b3);
    Groups groups;
    GroupBuilder::Build(gb, groups);

    PointToBitset dg;
    GraphUtil::ComputeDigraph(groups, BLACK, dg);

    BOOST_CHECK(dg[groups.CaptainOf(b2)] == groups.Nbs(b2, EMPTY));

    BOOST_CHECK_EQUAL(dg[a3].count(), 6u);
    BOOST_CHECK(dg[a3].test(b1));
    BOOST_CHECK(dg[a3].test(c1));
    BOOST_CHECK(dg[a3].test(c2));
    BOOST_CHECK(dg[a3].test(c3));
    BOOST_CHECK(dg[a3].test(a4));
    BOOST_CHECK(dg[a3].test(b4));

    GraphUtil::ComputeDigraph(groups, WHITE, dg);

    BOOST_CHECK(dg[groups.CaptainOf(a2)] == groups.Nbs(a2, EMPTY));

    BOOST_CHECK(dg[groups.CaptainOf(c3)] == groups.Nbs(c3, EMPTY));

    BOOST_CHECK_EQUAL(dg[b1].count(), 5u);
    BOOST_CHECK(dg[b1].test(a1));
    BOOST_CHECK(dg[b1].test(c1));
    BOOST_CHECK(dg[b1].test(a3));
    BOOST_CHECK(dg[b1].test(a4));
    BOOST_CHECK(dg[b1].test(a5));
}

}

//---------------------------------------------------------------------------
