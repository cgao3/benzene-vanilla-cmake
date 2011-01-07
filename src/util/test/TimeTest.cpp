//---------------------------------------------------------------------------
/** @file TimeTest.cpp */
//---------------------------------------------------------------------------

#include <boost/test/auto_unit_test.hpp>

#include "Time.hpp"

using namespace benzene;

//---------------------------------------------------------------------------

namespace {

BOOST_AUTO_TEST_CASE(Time_FormattedString)
{
    BOOST_CHECK_EQUAL(Time::Formatted(0.0), "0s");
    BOOST_CHECK_EQUAL(Time::Formatted(100.0), "1m40s");
    BOOST_CHECK_EQUAL(Time::Formatted(6300.44287), "1h45m0.4429s");
    BOOST_CHECK_EQUAL(Time::Formatted(86711.130042), "1d5m11.13s");
    // Note: the rounding of seconds is weird... not always up to 4
    // significant digits
    BOOST_CHECK_EQUAL(Time::Formatted(148337.6173), "1d17h12m17.62s");
}

}

//---------------------------------------------------------------------------
