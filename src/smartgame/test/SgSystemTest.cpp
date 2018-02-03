//----------------------------------------------------------------------------
/** @file SgSystemTest.cpp
    Unit tests for SgSystem. */
//----------------------------------------------------------------------------

#include "SgSystem.h"

#include <bitset>
#include <boost/test/auto_unit_test.hpp>

using namespace std;

//----------------------------------------------------------------------------

namespace {

/** Check for a bug in std::bitset.
    This bug occurs on Mac OS X 10.3.8 (gcc version 3.3 20030304) in
    the debug version of bitset.h (used if _GLIBCXX_DEBUG is defined).
    The implementation of bitset::operator|= in
    /usr/include/gcc/darwin/3.3/c++/debug/dbg_bitset.h contains a typo
    (!= instead of |=). */
BOOST_AUTO_TEST_CASE(SgSystemTestGccBitSetBug)
{
    bitset<1> set1;
    bitset<1> set2;
    set2.set(0);
    set1 |= set2;
    BOOST_CHECK(set1.test(0));
}

} // namespace

//----------------------------------------------------------------------------

