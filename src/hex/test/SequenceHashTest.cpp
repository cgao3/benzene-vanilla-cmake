//----------------------------------------------------------------------------
/** @file SequenceHashTest.cpp */
//----------------------------------------------------------------------------

#include <boost/test/auto_unit_test.hpp>

#include "SgSystem.h"
#include "SequenceHash.hpp"

using namespace benzene;

//---------------------------------------------------------------------------

namespace {

BOOST_AUTO_TEST_CASE(SequenceHash_PointSequence)
{
    PointSequence a, b;

    BOOST_CHECK_EQUAL(SequenceHash::Hash(a), 0u);
    
    a.push_back(HEX_CELL_A1);
    b.push_back(HEX_CELL_A1);
    BOOST_CHECK(SequenceHash::Hash(a) == SequenceHash::Hash(b));
    
    a.push_back(HEX_CELL_A2);
    b.push_back(HEX_CELL_A3);
    BOOST_CHECK(SequenceHash::Hash(a) != SequenceHash::Hash(b));

    // a = {1,2,3}, b = {1,3,2}
    a.push_back(HEX_CELL_A3);
    b.push_back(HEX_CELL_A2);
    BOOST_CHECK(SequenceHash::Hash(a) != SequenceHash::Hash(b));
    
    // a = {1,2,3}, b = {3,2,1}
    b.clear();
    b.push_back(HEX_CELL_A3);
    b.push_back(HEX_CELL_A2);
    b.push_back(HEX_CELL_A1);
    BOOST_CHECK(SequenceHash::Hash(a) != SequenceHash::Hash(b));

    // a = {1,2,3}, b = {}
    b.clear();
    BOOST_CHECK(SequenceHash::Hash(a) != SequenceHash::Hash(b));

    // a = {1,2,3}, b = {1}
    b.push_back(HEX_CELL_A1);
    BOOST_CHECK(SequenceHash::Hash(a) != SequenceHash::Hash(b));

    // a = {1,2,3}, b = {1,2}
    b.push_back(HEX_CELL_A2);
    BOOST_CHECK(SequenceHash::Hash(a) != SequenceHash::Hash(b));

    // a = {1,2,3}, b = {1,2,3}
    b.push_back(HEX_CELL_A3);
    BOOST_CHECK(SequenceHash::Hash(a) == SequenceHash::Hash(b));
}

BOOST_AUTO_TEST_CASE(SequenceHash_MoveSequence)
{
    MoveSequence a, b, c;
    BOOST_CHECK(SequenceHash::Hash(a) == SequenceHash::Hash(b));
    a.push_back(Move(BLACK, HEX_CELL_A1));
    BOOST_CHECK(SequenceHash::Hash(a) != SequenceHash::Hash(b));
    b.push_back(Move(BLACK, HEX_CELL_A1));
    BOOST_CHECK(SequenceHash::Hash(a) == SequenceHash::Hash(b));
    c.push_back(Move(BLACK, HEX_CELL_A3));
    BOOST_CHECK(SequenceHash::Hash(a) != SequenceHash::Hash(c));    
    a.push_back(Move(BLACK, HEX_CELL_A2));
    BOOST_CHECK(SequenceHash::Hash(a) != SequenceHash::Hash(b));
    b.push_back(Move(WHITE, HEX_CELL_A2));
    BOOST_CHECK(SequenceHash::Hash(a) != SequenceHash::Hash(b));
}

}

//---------------------------------------------------------------------------
