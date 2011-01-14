//---------------------------------------------------------------------------
/** @file VCSetTest.cpp */
//---------------------------------------------------------------------------

#include <boost/test/auto_unit_test.hpp>

#include "BitsetIterator.hpp"
#include "VCBuilder.hpp"
#include "VCSet.hpp"
#include "ChangeLog.hpp"

using namespace benzene;

//---------------------------------------------------------------------------

namespace {

BOOST_AUTO_TEST_CASE(VCSet_CheckCopy)
{
    StoneBoard bd(11, 11);
    VCSet con1(bd.Const(), BLACK);
    con1.Add(VC(NORTH, SOUTH), 0);
    VCSet con2(con1);
    BOOST_CHECK(con1 == con2);

    con1.Add(VC(NORTH, HEX_CELL_A1), 0);
    BOOST_CHECK(con1 != con2);

    con2 = con1;
    BOOST_CHECK(con1 == con2);

    con1.Add(VC(NORTH, HEX_CELL_C1), 0);
    BOOST_CHECK(con1 != con2);
}

BOOST_AUTO_TEST_CASE(VCSet_CheckRevert)
{
    //   a  b  c  d  e  f  g  h  i  
    //  1\.  .  .  .  .  .  .  .  .\1
    //   2\.  .  .  *  .  .  *  .  .\2
    //    3\.  .  B  .  .  .  B  .  .\3
    //     4\.  .  .  .  .  *  *  .  W\4   W
    //      5\.  .  W  .  B  W  B  .  .\5
    //  W    6\.  .  .  .  *  B  .  .  .\6  
    //        7\.  .  .  W  .  .  W  .  .\7
    //         8\.  .  .  .  .  .  .  .  .\8
    //          9\.  .  .  .  .  .  .  .  .\9
    //             a  b  c  d  e  f  g  h  i  
    StoneBoard bd(9, 9);
    bd.PlayMove(BLACK, HEX_CELL_E5);
    bd.PlayMove(WHITE, HEX_CELL_D7);
    bd.PlayMove(BLACK, HEX_CELL_F6);
    bd.PlayMove(WHITE, HEX_CELL_F5);
    bd.PlayMove(BLACK, HEX_CELL_C3);
    bd.PlayMove(WHITE, HEX_CELL_C5);
    bd.PlayMove(BLACK, HEX_CELL_G3);
    bd.PlayMove(WHITE, HEX_CELL_G7);
    bd.PlayMove(BLACK, HEX_CELL_G5);
    bd.PlayMove(WHITE, HEX_CELL_I4);

    std::vector<HexPoint> movesToCheck;
    movesToCheck.push_back(HEX_CELL_D2);
    movesToCheck.push_back(HEX_CELL_G2);
    movesToCheck.push_back(HEX_CELL_F4);
    movesToCheck.push_back(HEX_CELL_G4);
    movesToCheck.push_back(HEX_CELL_E6);

    Groups groups;
    GroupBuilder::Build(bd, groups);
    PatternState patterns(bd);
    patterns.Update();

    ChangeLog<VC> cl;
    VCSet con1(bd.Const(), BLACK);
    con1.SetSoftLimit(VC::FULL, 10);
    con1.SetSoftLimit(VC::SEMI, 25);
    VCSet con2(con1);

    VCBuilderParam param;
    param.max_ors = 4;
    param.and_over_edge = true;
    param.use_greedy_union = true;
    
    VCBuilder builder(param);
    builder.Build(con1, groups, patterns);
    builder.Build(con2, groups, patterns);
    BOOST_CHECK(con1 == con2);

    for (std::size_t i = 0; i < movesToCheck.size(); ++i) 
    {
        HexPoint p = movesToCheck[i];
        bitset_t added[BLACK_AND_WHITE];
        added[BLACK].set(p);
        bd.PlayMove(BLACK, p);
        Groups newGroups;
        GroupBuilder::Build(bd, newGroups);
        builder.Build(con2, groups, newGroups, patterns, added, &cl);
        con2.Revert(cl);
        bd.UndoMove(p);
        BOOST_CHECK(cl.Empty());
        BOOST_CHECK(con1 == con2);
    }
}

}

//---------------------------------------------------------------------------
