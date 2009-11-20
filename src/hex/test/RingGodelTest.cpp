//---------------------------------------------------------------------------
/** @file RingGodelTest.cpp
 */
//---------------------------------------------------------------------------
#include <boost/test/auto_unit_test.hpp>

#include "RingGodel.hpp"

using namespace benzene;

//---------------------------------------------------------------------------

namespace {

BOOST_AUTO_TEST_CASE(RingGodel_All)
{
    RingGodel brd;
    PatternRingGodel pat;
    
    brd.SetEmpty();
    pat.SetEmpty();

    // empty mask matches empty brd
    BOOST_CHECK(pat.MatchesGodel(brd));

    // color of a slice not in mask does not affect matching.
    for (ColorIterator c1; c1; ++c1) {
        pat.SetSliceToColor(0, *c1);
        for (ColorIterator c2; c2; ++c2) {
            brd.SetSliceToColor(0, *c2);
            BOOST_CHECK(pat.MatchesGodel(brd));
        }
    }

    // color of slices in mask does affect matching.
    brd.SetEmpty();
    pat.SetEmpty();
    pat.AddSliceToMask(0);
    for (ColorIterator c1; c1; ++c1) {
        pat.SetSliceToColor(0, *c1);
        for (ColorIterator c2; c2; ++c2) {
            brd.SetSliceToColor(0, *c2);
            BOOST_CHECK_EQUAL(pat.MatchesGodel(brd), *c1 == *c2);
        }
    }

    // check that a B,W,or BW slice matches a BW slice, and that E
    // does not match BW.
    brd.SetEmpty();
    pat.SetEmpty();
    pat.AddSliceToMask(0);
    
    brd.AddColorToSlice(0, BLACK);
    brd.AddColorToSlice(0, WHITE);
    brd.RemoveColorFromSlice(0, EMPTY);

    pat.SetSliceToColor(0, EMPTY);
    BOOST_CHECK(!pat.MatchesGodel(brd));
    pat.SetSliceToColor(0, BLACK);
    BOOST_CHECK(pat.MatchesGodel(brd));
    pat.SetSliceToColor(0, WHITE);
    BOOST_CHECK(pat.MatchesGodel(brd));
    pat.SetSliceToColor(0, WHITE);
    pat.AddColorToSlice(0, BLACK);
    BOOST_CHECK(pat.MatchesGodel(brd));
    
    // check that a BW slice matches only BW.
    brd.SetEmpty();
    pat.SetEmpty();
    pat.AddSliceToMask(0);
    
    pat.AddColorToSlice(0, BLACK);
    pat.AddColorToSlice(0, WHITE);
    pat.RemoveColorFromSlice(0, EMPTY);

    brd.SetSliceToColor(0, EMPTY);
    BOOST_CHECK(!pat.MatchesGodel(brd));
    brd.SetSliceToColor(0, BLACK);
    BOOST_CHECK(!pat.MatchesGodel(brd));
    brd.SetSliceToColor(0, WHITE);
    BOOST_CHECK(!pat.MatchesGodel(brd));
    brd.SetSliceToColor(0, WHITE);
    brd.AddColorToSlice(0, BLACK);
    BOOST_CHECK(pat.MatchesGodel(brd));
}

}

//---------------------------------------------------------------------------
