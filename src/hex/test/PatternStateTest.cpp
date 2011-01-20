//----------------------------------------------------------------------------
/** @file PatternStateTest.cpp

    @todo Test a black pattern.

    @todo Test that the obtuse corner is both black and white.
    
    @todo Test PatternHits::moves2() functionality.

    @todo Test that an arbitrary carrier (more than 1 cell) is correct
          when returned in PatternHits::moves{1/2}(). 

    @todo Test that incremental updates work the same as complete updates.
*/
//----------------------------------------------------------------------------

#include <boost/test/auto_unit_test.hpp>

#include "SgSystem.h"
#include "PatternState.hpp"

using namespace benzene;

//---------------------------------------------------------------------------

namespace {

BOOST_AUTO_TEST_CASE(PatternState_Tests)
{
    //              W !
    //             W * W                         [7/0]
    //
    //             W * W
    //              W !                          [7m/0]
    std::string patstring 
        = "v:1,0,1,0,0;1,0,0,1,0;1,0,1,0,0;1,0,1,0,0;0,0,0,0,0;0,0,0,0,0;";
    Pattern pattern;
    BOOST_CHECK(pattern.Unserialize(patstring));
    PatternSet patterns;
    pattern.SetName("pat");
    patterns.push_back(pattern);
    pattern.Mirror();
    pattern.SetName("mpat");
    patterns.push_back(pattern);
    HashedPatternSet hashpat;
    hashpat.Hash(patterns);
    
    StoneBoard brd(11, 11);
    PatternState pastate(brd);
    
    //       0x5765ad24894d45fc
    //   a  b  c  d  e  f  g  h  i  j  k  
    //  1\.  .  .  .  .  .  .  .  .  .  .\1
    //   2\.  .  .  .  .  .  .  .  .  .  .\2
    //    3\.  .  .  .  .  .  .  W  b  .  .\3
    //     4\f  W  .  .  .  .  W  a  W  .  .\4
    //      5\e  .  .  .  .  .  .  .  .  .  .\5
    //       6\.  .  .  .  .  .  .  .  .  .  .\6  W
    //   W    7\.  .  .  .  .  .  .  .  .  .  .\7
    //         8\.  .  .  .  .  .  W  c  W  .  h\8
    //          9\.  .  .  .  .  .  W  d  .  W  g\9
    //          10\.  .  .  .  .  .  .  .  .  .  .\10
    //           11\.  .  .  .  .  .  .  .  .  .  .\11
    //               a  b  c  d  e  f  g  h  i  j  k  
    brd.PlayMove(WHITE, HEX_CELL_G4);
    brd.PlayMove(WHITE, HEX_CELL_H3);
    brd.PlayMove(WHITE, HEX_CELL_I4);

    brd.PlayMove(WHITE, HEX_CELL_G8);
    brd.PlayMove(WHITE, HEX_CELL_G9);
    brd.PlayMove(WHITE, HEX_CELL_I8);

    brd.PlayMove(WHITE, HEX_CELL_B4);

    brd.PlayMove(WHITE, HEX_CELL_J9);
    pastate.Update();

    std::vector<PatternHits> hits(FIRST_INVALID);
    bitset_t found = pastate.MatchOnBoard(brd.GetEmpty(), hashpat, 
                                          PatternState::MATCH_ALL, hits);

    // Ensure a, d, e, g were found
    BOOST_CHECK_EQUAL(found.count(), 6u);
    BOOST_CHECK(found.test(HEX_CELL_H4));
    BOOST_CHECK(found.test(HEX_CELL_H8));
    BOOST_CHECK(found.test(HEX_CELL_A5));
    BOOST_CHECK(found.test(HEX_CELL_K9));
                      
    // (a->b)
    BOOST_CHECK_EQUAL(hits[HEX_CELL_H4].size(), 1u);
    BOOST_CHECK_EQUAL(hits[HEX_CELL_H4][0].Moves1().size(), 1u);
    BOOST_CHECK_EQUAL(hits[HEX_CELL_H4][0].Moves1()[0], HEX_CELL_I3);

    // (d->c) 
    BOOST_CHECK_EQUAL(hits[HEX_CELL_H8].size(), 1u);
    BOOST_CHECK_EQUAL(hits[HEX_CELL_H8][0].Moves1().size(), 1u);
    BOOST_CHECK_EQUAL(hits[HEX_CELL_H8][0].Moves1()[0], HEX_CELL_H9);

    // (e<->f)
    BOOST_CHECK_EQUAL(hits[HEX_CELL_A5].size(), 1u);
    BOOST_CHECK_EQUAL(hits[HEX_CELL_A5][0].Moves1().size(), 1u);
    BOOST_CHECK_EQUAL(hits[HEX_CELL_A5][0].Moves1()[0], HEX_CELL_A4);

    BOOST_CHECK_EQUAL(hits[HEX_CELL_A4].size(), 1u);
    BOOST_CHECK_EQUAL(hits[HEX_CELL_A4][0].Moves1().size(), 1u);
    BOOST_CHECK_EQUAL(hits[HEX_CELL_A4][0].Moves1()[0], HEX_CELL_A5);

    // (g<->h)
    BOOST_CHECK_EQUAL(hits[HEX_CELL_K9].size(), 1u);
    BOOST_CHECK_EQUAL(hits[HEX_CELL_K9][0].Moves1().size(), 1u);
    BOOST_CHECK_EQUAL(hits[HEX_CELL_K9][0].Moves1()[0], HEX_CELL_K8);

    BOOST_CHECK_EQUAL(hits[HEX_CELL_K8].size(), 1u);
    BOOST_CHECK_EQUAL(hits[HEX_CELL_K8][0].Moves1().size(), 1u);
    BOOST_CHECK_EQUAL(hits[HEX_CELL_K8][0].Moves1()[0], HEX_CELL_K9);
}

}

//---------------------------------------------------------------------------
