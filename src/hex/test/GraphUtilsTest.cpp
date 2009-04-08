//----------------------------------------------------------------------------
// $Id: GraphUtilsTest.cpp 1821 2008-12-22 04:03:08Z broderic $
//----------------------------------------------------------------------------

#include <boost/test/auto_unit_test.hpp>

#include "GraphUtils.hpp"

//---------------------------------------------------------------------------

namespace {

BOOST_AUTO_UNIT_TEST(GraphUtils_ComputeDigraph)
{
    BOOST_REQUIRE(MAX_WIDTH >= 5 && MAX_HEIGHT >= 5);
    GroupBoard gb(5, 5);
    gb.startNewGame();

    //  a  b  c  d  e  
    // 1\.  .  .  .  .\1
    //  2\W  B  .  .  .\2
    //   3\.  B  .  .  .\3
    //    4\.  .  .  .  .\4
    //     5\.  .  .  .  .\5
    //        a  b  c  d  e  
    HexPoint a1 = HexPointUtil::fromString("a1");
    HexPoint b1 = HexPointUtil::fromString("b1");
    HexPoint c1 = HexPointUtil::fromString("c1");   
    HexPoint a2 = HexPointUtil::fromString("a2");
    HexPoint b2 = HexPointUtil::fromString("b2");
    HexPoint c2 = HexPointUtil::fromString("c2");
    HexPoint a3 = HexPointUtil::fromString("a3");
    HexPoint b3 = HexPointUtil::fromString("b3");  
    HexPoint c3 = HexPointUtil::fromString("c3");
    HexPoint a4 = HexPointUtil::fromString("a4");
    HexPoint b4 = HexPointUtil::fromString("b4");
    HexPoint a5 = HexPointUtil::fromString("a5");    

    bitset_t nbs;    
    gb.playMove(BLACK, b2);
    gb.playMove(WHITE, a2);
    gb.playMove(BLACK, b3);
    gb.absorb();

    PointToBitset dg;
    GraphUtils::ComputeDigraph(gb, BLACK, dg);

    BOOST_CHECK(dg[gb.getCaptain(b2)] == 
                gb.Nbs(gb.getCaptain(b2), EMPTY));

    BOOST_CHECK_EQUAL(dg[a3].count(), 6);
    BOOST_CHECK(dg[a3].test(b1));
    BOOST_CHECK(dg[a3].test(c1));
    BOOST_CHECK(dg[a3].test(c2));
    BOOST_CHECK(dg[a3].test(c3));
    BOOST_CHECK(dg[a3].test(a4));
    BOOST_CHECK(dg[a3].test(b4));

    GraphUtils::ComputeDigraph(gb, WHITE, dg);

    BOOST_CHECK(dg[gb.getCaptain(a2)] == 
                gb.Nbs(gb.getCaptain(a2), EMPTY));

    BOOST_CHECK(dg[gb.getCaptain(c3)] == 
                gb.Nbs(gb.getCaptain(c3), EMPTY));

    BOOST_CHECK_EQUAL(dg[b1].count(), 5);
    BOOST_CHECK(dg[b1].test(a1));
    BOOST_CHECK(dg[b1].test(c1));
    BOOST_CHECK(dg[b1].test(a3));
    BOOST_CHECK(dg[b1].test(a4));
    BOOST_CHECK(dg[b1].test(a5));
}

}
