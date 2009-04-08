//---------------------------------------------------------------------------
// $Id: ZobristHashTest.cpp 1657 2008-09-15 23:32:09Z broderic $
//---------------------------------------------------------------------------

#include <boost/test/auto_unit_test.hpp>

#include "ZobristHash.hpp"

//---------------------------------------------------------------------------

namespace {

BOOST_AUTO_UNIT_TEST(ZobristHash_InitializationAndUpdates)
{
    bitset_t black, white;
    hash_t h1, h2, h3;
    ZobristHash zh1, zh2;
    
    // Note: due to the probabilistic nature of Zobrist
    // hashing, it is possible that some of these tests
    // can fail. However, it should be extremely improbable,
    // to the point of never occuring in practice
    
    // check that base values do NOT  differ
    zh1 = ZobristHash();
    zh2 = ZobristHash();
    BOOST_CHECK(zh1.hash() == zh2.hash());
    
    // check that updates change hash value, reset restores
    h1 = zh1.hash();
    zh1.update(BLACK, FIRST_CELL);
    h2 = zh1.hash();
    BOOST_CHECK(h1 != h2);
    zh1.reset();
    BOOST_CHECK_EQUAL(h1, zh1.hash());
    zh1.update(WHITE, FIRST_CELL);
    h3 = zh1.hash();
    BOOST_CHECK(h1 != h3);
    BOOST_CHECK(h2 != h3);
    zh1.update(WHITE, FIRST_CELL);
    BOOST_CHECK_EQUAL(h1, zh1.hash());
    zh1.update(BLACK, FIRST_CELL);
    zh1.update(WHITE, FIRST_CELL);
    BOOST_CHECK(h1 != zh1.hash());
    BOOST_CHECK(h2 != zh1.hash());
    BOOST_CHECK(h3 != zh1.hash());
    zh1.update(BLACK, FIRST_CELL);
    BOOST_CHECK_EQUAL(h3, zh1.hash());
    
    // check that sequence of updates after reset
    // obtains same result as initialization
    BOOST_REQUIRE(FIRST_CELL < FIRST_INVALID - 1);
    black.set(FIRST_CELL);
    black.set(FIRST_INVALID - 1);
    white.set(SWAP_PIECES);
    zh1 = ZobristHash(black, white);
    h1 = zh1.hash();
    zh1.reset();
    zh1.update(BLACK, FIRST_CELL);
    BOOST_CHECK(h1 != zh1.hash());
    zh1.update(WHITE, SWAP_PIECES);
    BOOST_CHECK(h1 != zh1.hash());
    zh1.update(BLACK, static_cast<HexPoint>(FIRST_INVALID - 1));
    BOOST_CHECK_EQUAL(h1, zh1.hash());
    
    // check that compute gives same result as
    // update and initialization
    zh1.reset();
    zh1.compute(black, white);
    BOOST_CHECK_EQUAL(h1, zh1.hash());
    zh1.reset();
    for (int i=0; i<FIRST_INVALID; ++i) {
        if (black.test(i)) 
            zh1.update(BLACK, static_cast<HexPoint>(i));
        if (white.test(i))
            zh1.update(WHITE, static_cast<HexPoint>(i));
    }
    BOOST_CHECK_EQUAL(h1, zh1.hash());
}

}

//---------------------------------------------------------------------------
