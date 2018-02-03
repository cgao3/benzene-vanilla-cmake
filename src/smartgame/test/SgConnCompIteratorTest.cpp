//----------------------------------------------------------------------------
/** @file SgConnCompIteratorTest.cpp
    Unit tests for ConnCompIterator */
//----------------------------------------------------------------------------

#include "SgSystem.h"

#include <sstream>
#include <boost/test/auto_unit_test.hpp>
#include "SgConnCompIterator.h"

using namespace std;
using SgPointUtil::Pt;

//----------------------------------------------------------------------------

namespace {

/** Test ConnCompIterator.
    Assumes that the iterator returns components with lower point values
    first. */
BOOST_AUTO_TEST_CASE(SgPointSetTestConnCompIterator)
{
    SgPointSet a;
    a.Include(Pt(1, 1));
    a.Include(Pt(1, 2));
    a.Include(Pt(2, 1));
    a.Include(Pt(1, SG_MAX_SIZE));
    SgConnCompIterator it(a, SG_MAX_SIZE);
    BOOST_CHECK(it);
    SgPointSet b = *it;
    BOOST_CHECK_EQUAL(b.Size(), 3);
    BOOST_CHECK(a.Contains(Pt(1, 1)));
    BOOST_CHECK(a.Contains(Pt(1, 2)));
    BOOST_CHECK(a.Contains(Pt(2, 1)));
    ++it;
    BOOST_CHECK(it);
    b = *it;
    BOOST_CHECK_EQUAL(b.Size(), 1);
    BOOST_CHECK(a.Contains(Pt(1, SG_MAX_SIZE)));
    ++it;
    BOOST_CHECK(! it);
}

} // namespace

//----------------------------------------------------------------------------

