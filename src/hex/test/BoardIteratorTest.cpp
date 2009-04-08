//---------------------------------------------------------------------------
// $Id: BoardIteratorTest.cpp 1264 2008-04-01 00:31:37Z broderic $
//---------------------------------------------------------------------------
#include <boost/test/auto_unit_test.hpp>

#include "BoardIterator.hpp"

//---------------------------------------------------------------------------

namespace {

BOOST_AUTO_UNIT_TEST(BoardIterator_Basic)
{
    std::vector<HexPoint> empty, simple;
    
    empty.push_back(INVALID_POINT);
    simple.push_back(FIRST_EDGE);
    simple.push_back(FIRST_CELL);
    simple.push_back(INVALID_POINT);

    {
        BoardIterator b(&empty[0]);
        BOOST_CHECK(!b);
    }

    { 
        BoardIterator b(simple);
        BOOST_CHECK(b);
        BOOST_CHECK_EQUAL(*b, FIRST_EDGE);
        ++b;
        BOOST_CHECK(b);
        BOOST_CHECK_EQUAL(*b, FIRST_CELL);
        ++b;
        BOOST_CHECK(!b);
    }
}

}

//---------------------------------------------------------------------------
