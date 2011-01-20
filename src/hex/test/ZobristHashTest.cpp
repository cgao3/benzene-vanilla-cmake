//---------------------------------------------------------------------------
/** @file ZobristHashTest.cpp */
//---------------------------------------------------------------------------

#include <boost/test/auto_unit_test.hpp>

#include "SgSystem.h"
#include "ZobristHash.hpp"

using namespace benzene;

//---------------------------------------------------------------------------

namespace {

BOOST_AUTO_TEST_CASE(ZobristHash_InitializationAndUpdates)
{
    // Note: due to the probabilistic nature of Zobrist
    // hashing, it is possible that some of these tests
    // can fail. However, it should be extremely improbable,
    // to the point of never occuring in practice.
    // This is true only if USE_PREDEFINED_HASHES is false. 
    
    // check that base values do NOT differ
    ZobristHash zh1(5, 5);
    ZobristHash zh2(5, 5);
    BOOST_CHECK(zh1.Hash() == zh2.Hash());
    
    // check that updates change hash value, reset restores
    SgHashCode h1, h2, h3;
    h1 = zh1.Hash();
    zh1.Update(BLACK, FIRST_CELL);
    h2 = zh1.Hash();
    BOOST_CHECK(h1 != h2);
    zh1.Reset();
    BOOST_CHECK_EQUAL(h1, zh1.Hash());
    zh1.Update(WHITE, FIRST_CELL);
    h3 = zh1.Hash();
    BOOST_CHECK(h1 != h3);
    BOOST_CHECK(h2 != h3);
    zh1.Update(WHITE, FIRST_CELL);
    BOOST_CHECK_EQUAL(h1, zh1.Hash());
    zh1.Update(BLACK, FIRST_CELL);
    zh1.Update(WHITE, FIRST_CELL);
    BOOST_CHECK(h1 != zh1.Hash());
    BOOST_CHECK(h2 != zh1.Hash());
    BOOST_CHECK(h3 != zh1.Hash());
    zh1.Update(BLACK, FIRST_CELL);
    BOOST_CHECK_EQUAL(h3, zh1.Hash());
    
    // check that sequence of updates after reset
    // obtains same result as initialization
    bitset_t black, white;
    BOOST_REQUIRE(FIRST_CELL < FIRST_INVALID - 1);
    black.set(FIRST_CELL);
    black.set(FIRST_INVALID - 1);
    white.set(SWAP_PIECES);
    zh1 = ZobristHash(5, 5);
    zh1.Compute(black, white);
    h1 = zh1.Hash();
    zh1.Reset();
    zh1.Update(BLACK, FIRST_CELL);
    BOOST_CHECK(h1 != zh1.Hash());
    zh1.Update(WHITE, SWAP_PIECES);
    BOOST_CHECK(h1 != zh1.Hash());
    zh1.Update(BLACK, static_cast<HexPoint>(FIRST_INVALID - 1));
    BOOST_CHECK_EQUAL(h1, zh1.Hash());
    
    // check that compute gives same result as
    // Update and initialization
    zh1.Reset();
    zh1.Compute(black, white);
    BOOST_CHECK_EQUAL(h1, zh1.Hash());
    zh1.Reset();
    for (int i = 0; i < FIRST_INVALID; ++i) 
    {
        if (black.test(i)) 
            zh1.Update(BLACK, static_cast<HexPoint>(i));
        if (white.test(i))
            zh1.Update(WHITE, static_cast<HexPoint>(i));
    }
    BOOST_CHECK_EQUAL(h1, zh1.Hash());
}

}

//---------------------------------------------------------------------------
