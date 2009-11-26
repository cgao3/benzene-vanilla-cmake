//----------------------------------------------------------------------------
/** @file GroupsTest.cpp
 */
//----------------------------------------------------------------------------

#include <boost/test/auto_unit_test.hpp>

#include "Groups.hpp"
#include "BoardIterator.hpp"

using namespace benzene;

//---------------------------------------------------------------------------

namespace {

BOOST_AUTO_TEST_CASE(Groups_Captains)
{
    BOOST_REQUIRE(MAX_WIDTH >= 5 && MAX_HEIGHT >= 5);
    StoneBoard brd(5, 5);
    Groups groups;

    // On empty board all edges and cells are captains of themselves.
    GroupBuilder::Build(brd, groups);
    BOOST_CHECK(groups.GetGroup(NORTH).Captain() == NORTH);
    BOOST_CHECK(groups.GetGroup(SOUTH).Captain() == SOUTH);
    BOOST_CHECK(groups.GetGroup(EAST).Captain() == EAST);
    BOOST_CHECK(groups.GetGroup(WEST).Captain() == WEST);
    for (BoardIterator p(brd.Const().Interior()); p; ++p)
        BOOST_CHECK(groups.GetGroup(*p).Captain() == *p);

    // Check that FIRST_CELL is absorbed into the north group;
    // and that NORTH is always the captain of its group. 
    brd.playMove(BLACK, FIRST_CELL);
    GroupBuilder::Build(brd, groups);
    BOOST_CHECK(groups.GetGroup(NORTH).Captain() == NORTH);
    BOOST_CHECK(groups.GetGroup(FIRST_CELL).Captain() == NORTH);
}

BOOST_AUTO_TEST_CASE(Groups_Nbs)
{
    BOOST_REQUIRE(MAX_WIDTH >= 5 && MAX_HEIGHT >= 5);
    StoneBoard brd(5, 5);
    Groups groups;
    bitset_t nbs;

    //  a  b  c  d  e  
    // 1\.  .  .  .  .\1
    //  2\W  B  .  .  .\2
    //   3\.  B  .  .  .\3
    //    4\.  .  .  .  .\4
    //     5\.  .  .  .  .\5
    //        a  b  c  d  e  
    brd.playMove(BLACK, HEX_CELL_B2);
    brd.playMove(WHITE, HEX_CELL_A2);
    brd.playMove(BLACK, HEX_CELL_B3);
    GroupBuilder::Build(brd, groups);

    nbs = groups.GetGroup(HEX_CELL_B2).Nbs();
    BOOST_CHECK_EQUAL(nbs.count(), 8u);
    BOOST_CHECK(nbs.test(HEX_CELL_B1));
    BOOST_CHECK(nbs.test(HEX_CELL_C1));
    BOOST_CHECK(nbs.test(groups.GetGroup(HEX_CELL_A2).Captain()));
    BOOST_CHECK(nbs.test(HEX_CELL_C2));
    BOOST_CHECK(nbs.test(HEX_CELL_A3));
    BOOST_CHECK(nbs.test(HEX_CELL_C3));
    BOOST_CHECK(nbs.test(HEX_CELL_A4));
    BOOST_CHECK(nbs.test(HEX_CELL_B4));
   
    nbs = groups.GetGroup(HEX_CELL_C2).Nbs();
    BOOST_CHECK_EQUAL(nbs.count(), 5u);
    BOOST_CHECK(nbs.test(groups.GetGroup(HEX_CELL_B2).Captain()));

    nbs = groups.GetGroup(HEX_CELL_A2).Nbs();
    BOOST_CHECK_EQUAL(nbs.count(), 8u);
    BOOST_CHECK(nbs.test(NORTH));
    BOOST_CHECK(nbs.test(SOUTH));
    BOOST_CHECK(nbs.test(HEX_CELL_A1));
    BOOST_CHECK(nbs.test(HEX_CELL_B1));
    BOOST_CHECK(nbs.test(HEX_CELL_A3));
    BOOST_CHECK(nbs.test(HEX_CELL_A4));
    BOOST_CHECK(nbs.test(HEX_CELL_A5));
    BOOST_CHECK(nbs.test(groups.GetGroup(HEX_CELL_B2).Captain()));

    nbs = groups.GetGroup(HEX_CELL_A3).Nbs();
    BOOST_CHECK_EQUAL(nbs.count(), 3u);
    BOOST_CHECK(nbs.test(HEX_CELL_A4));
    BOOST_CHECK(nbs.test(groups.GetGroup(HEX_CELL_A2).Captain()));
    BOOST_CHECK(nbs.test(groups.GetGroup(HEX_CELL_B2).Captain()));
}

BOOST_AUTO_TEST_CASE(Groups_Members)
{
    StoneBoard brd(5, 5);
    Groups groups;
    Group grp;

    //  a  b  c  d  e  
    // 1\.  .  W  .  W\1
    //  2\W  .  B  B  .\2
    //   3\B  B  W  B  .\3
    //    4\.  B  B  .  W\4
    //     5\.  .  .  .  .\5
    //        a  b  c  d  e 
    brd.playMove(WHITE, HEX_CELL_C1);
    brd.playMove(WHITE, HEX_CELL_E1);
    brd.playMove(WHITE, HEX_CELL_A2);
    brd.playMove(BLACK, HEX_CELL_C2);
    brd.playMove(BLACK, HEX_CELL_D2);
    brd.playMove(BLACK, HEX_CELL_A3);
    brd.playMove(BLACK, HEX_CELL_B3);
    brd.playMove(WHITE, HEX_CELL_C3);
    brd.playMove(BLACK, HEX_CELL_D3);
    brd.playMove(BLACK, HEX_CELL_B4);
    brd.playMove(BLACK, HEX_CELL_C4);
    brd.playMove(WHITE, HEX_CELL_E4);
    GroupBuilder::Build(brd, groups);
    BOOST_CHECK_EQUAL(groups.NumGroups(), 20u);

    // Check all empties are singletons
    for (BoardIterator p(brd.Const().Interior()); p; ++p)
        if (brd.GetColor(*p) == EMPTY)
        {
            BOOST_CHECK_EQUAL(groups.GetGroup(*p).Size(), 1u);
            BOOST_CHECK(groups.GetGroup(*p).Members().test(*p));
        }            

    grp = groups.GetGroup(NORTH);
    BOOST_CHECK_EQUAL(grp.Size(), 1u);
    BOOST_CHECK(grp.Members().test(NORTH));
    BOOST_CHECK_EQUAL(grp.Captain(), NORTH);

    grp = groups.GetGroup(HEX_CELL_C1);
    BOOST_CHECK_EQUAL(grp.Size(), 1u);
    BOOST_CHECK(grp.Members().test(HEX_CELL_C1));
    BOOST_CHECK_EQUAL(grp.Captain(), HEX_CELL_C1);

    grp = groups.GetGroup(HEX_CELL_E1);
    BOOST_CHECK_EQUAL(grp.Size(), 3u);
    BOOST_CHECK(grp.Members().test(EAST));
    BOOST_CHECK(grp.Members().test(HEX_CELL_E1));
    BOOST_CHECK(grp.Members().test(HEX_CELL_E4));
    BOOST_CHECK_EQUAL(grp.Captain(), EAST);

    grp = groups.GetGroup(HEX_CELL_A2);
    BOOST_CHECK_EQUAL(grp.Size(), 2u);
    BOOST_CHECK(grp.Members().test(HEX_CELL_A2));
    BOOST_CHECK(grp.Members().test(WEST));
    BOOST_CHECK_EQUAL(grp.Captain(), WEST);

    grp = groups.GetGroup(HEX_CELL_C2);
    BOOST_CHECK_EQUAL(grp.Size(), 7u);
    BOOST_CHECK(grp.Members().test(HEX_CELL_C2));
    BOOST_CHECK(grp.Members().test(HEX_CELL_D2));
    BOOST_CHECK(grp.Members().test(HEX_CELL_A3));
    BOOST_CHECK(grp.Members().test(HEX_CELL_B3));
    BOOST_CHECK(grp.Members().test(HEX_CELL_D3));
    BOOST_CHECK(grp.Members().test(HEX_CELL_B4));
    BOOST_CHECK(grp.Members().test(HEX_CELL_C4));
    BOOST_CHECK_EQUAL(grp.Captain(), HEX_CELL_C2);

    grp = groups.GetGroup(HEX_CELL_C3);
    BOOST_CHECK_EQUAL(grp.Size(), 1u);
    BOOST_CHECK(grp.Members().test(HEX_CELL_C3));
    BOOST_CHECK_EQUAL(grp.Captain(), HEX_CELL_C3);
    
    grp = groups.GetGroup(SOUTH);
    BOOST_CHECK_EQUAL(grp.Size(), 1u);
    BOOST_CHECK(grp.Members().test(SOUTH));
    BOOST_CHECK_EQUAL(grp.Captain(), SOUTH);
}

BOOST_AUTO_TEST_CASE(Groups_Iterator)
{
    StoneBoard brd(3, 3);
    Groups groups;
    //  a  b  c   
    // 1\.  .  W\1
    //  2\W  W  B\2
    //   3\B  .  W\3
    //      a  b  c 
    brd.playMove(WHITE, HEX_CELL_C1);
    brd.playMove(WHITE, HEX_CELL_A2);
    brd.playMove(WHITE, HEX_CELL_B2);
    brd.playMove(BLACK, HEX_CELL_C2);
    brd.playMove(BLACK, HEX_CELL_A3);
    brd.playMove(WHITE, HEX_CELL_C3);
    GroupBuilder::Build(brd, groups);

    GroupIterator g(groups);
    BOOST_CHECK(g);
    BOOST_CHECK_EQUAL(g->Captain(), NORTH);
    ++g;
    BOOST_CHECK(g);
    BOOST_CHECK_EQUAL(g->Captain(), EAST);
    ++g;
    BOOST_CHECK(g);
    BOOST_CHECK_EQUAL(g->Captain(), SOUTH);
    ++g;
    BOOST_CHECK(g);
    BOOST_CHECK_EQUAL(g->Captain(), HEX_CELL_A1);
    ++g;
    BOOST_CHECK(g);
    BOOST_CHECK_EQUAL(g->Captain(), HEX_CELL_B1);
    ++g;
    BOOST_CHECK(g);
    BOOST_CHECK_EQUAL(g->Captain(), HEX_CELL_C2);
    ++g;
    BOOST_CHECK(g);
    BOOST_CHECK_EQUAL(g->Captain(), HEX_CELL_B3);
    ++g;
    BOOST_CHECK(!g);
}

//----------------------------------------------------------------------------

} // namespace

//----------------------------------------------------------------------------
