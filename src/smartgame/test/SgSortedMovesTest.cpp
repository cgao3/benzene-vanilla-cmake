//----------------------------------------------------------------------------
/** @file SgSortedMovesTest.cpp
    Unit tests for SgSortedMoves. */
//----------------------------------------------------------------------------

#include "SgSystem.h"

#include <limits>
#include <boost/test/auto_unit_test.hpp>
#include "SgSortedMoves.h"

using std::numeric_limits;
//----------------------------------------------------------------------------

namespace {

BOOST_AUTO_TEST_CASE(SgSortedMovesTest_1)
{
    SgSortedMoves<int,int,2> moves(2);
    BOOST_CHECK_EQUAL(moves.NuMoves(), 0);
    BOOST_CHECK_EQUAL(moves.MaxNuMoves(), 2);
    BOOST_CHECK_EQUAL(moves.InitLowerBound(), 
    				  boost::numeric::bounds<int>::lowest());
    BOOST_CHECK_EQUAL(moves.LowerBound(), 
    				  boost::numeric::bounds<int>::lowest());
    moves.Insert(20,3);
    BOOST_CHECK_EQUAL(moves.NuMoves(), 1);
    BOOST_CHECK_EQUAL(moves.MaxNuMoves(), 2);
    BOOST_CHECK_EQUAL(moves.LowerBound(),
    				  boost::numeric::bounds<int>::lowest());
    moves.Insert(10,4);
    BOOST_CHECK_EQUAL(moves.LowerBound(), 3);
    moves.Insert(3,6);
    BOOST_CHECK_EQUAL(moves.LowerBound(), 4);
}

BOOST_AUTO_TEST_CASE(SgSortedMovesTest_2)
{
    SgSortedMoves<int,double,3> moves(2);
    BOOST_CHECK_EQUAL(moves.InitLowerBound(), 
    				  boost::numeric::bounds<double>::lowest());
    BOOST_CHECK_EQUAL(moves.LowerBound(), 
    				  boost::numeric::bounds<double>::lowest());
}

} // namespace
