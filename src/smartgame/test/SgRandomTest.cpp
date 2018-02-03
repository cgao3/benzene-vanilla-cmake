//----------------------------------------------------------------------------
/** @file SgRandomTest.cpp
    Unit tests for SgRandom. */
//----------------------------------------------------------------------------

#include "SgSystem.h"

#include <boost/test/auto_unit_test.hpp>
#include "SgRandom.h"

//----------------------------------------------------------------------------

namespace {

BOOST_AUTO_TEST_CASE(SgRandomTestFloat_01)
{
    SgRandom r;
    float float01;
	for (int i=0; i < 1000; ++i)
    {
        float01 = r.Float_01();
        BOOST_CHECK_GE(float01, 0.);
        BOOST_CHECK_LT(float01, 1.);
    }
}

BOOST_AUTO_TEST_CASE(SgRandomTestFloat)
{
    SgRandom r;
    float f;
	for (int i=0; i < 1000; ++i)
    {
        int bound = r.Int(1000) + 1;
        f = r.Float(bound);
        BOOST_CHECK_GE(f, 0.);
        BOOST_CHECK_LT(f, bound);
    }
}

} // namespace

//----------------------------------------------------------------------------

