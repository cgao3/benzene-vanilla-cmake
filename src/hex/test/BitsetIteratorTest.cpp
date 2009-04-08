//---------------------------------------------------------------------------
// $Id: BitsetIteratorTest.cpp 1264 2008-04-01 00:31:37Z broderic $
//---------------------------------------------------------------------------
#include <boost/test/auto_unit_test.hpp>

#include "BitsetIterator.hpp"

//---------------------------------------------------------------------------

namespace {

BOOST_AUTO_UNIT_TEST(BitsetIterator_Basic)
{
    bitset_t bs1;
    BitsetIterator b1(bs1);
    BOOST_CHECK(!b1);

    bitset_t bs2;
    bs2.set(FIRST_EDGE);
    BitsetIterator b2(bs2);
    BOOST_CHECK(b2);
    BOOST_CHECK_EQUAL(*b2, FIRST_EDGE);
    ++b2;
    BOOST_CHECK(!b2);
    
    bitset_t bs3;
    bs3.set(FIRST_CELL);
    bs3.set(FIRST_CELL+1);
    bs3.set(FIRST_CELL+6);
    BitsetIterator b3(bs3);
    BOOST_CHECK(b3);
    BOOST_CHECK_EQUAL(*b3, FIRST_CELL);
    ++b3;
    BOOST_CHECK(b3);
    BOOST_CHECK_EQUAL(*b3, FIRST_CELL+1);
    ++b3;
    BOOST_CHECK(b3);
    BOOST_CHECK_EQUAL(*b3, FIRST_CELL+6);
    ++b3;
    BOOST_CHECK(!b3);
}

}

//---------------------------------------------------------------------------
