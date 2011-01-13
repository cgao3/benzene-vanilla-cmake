//----------------------------------------------------------------------------
/** @file TestMain.cpp */
//----------------------------------------------------------------------------

#include <cstdlib>
#include <boost/version.hpp>

#include "config.h"
#include "CommonProgram.hpp"

using namespace benzene;

#define BOOST_VERSION_MAJOR (BOOST_VERSION / 100000)
#define BOOST_VERSION_MINOR (BOOST_VERSION / 100 % 1000)

//----------------------------------------------------------------------------

namespace {

/** Initializes hex system. */
void Initialize()
{
    int argc = 1;
    char name[] = "benzene-unittest";
    char* argv = name;
    CommonProgram program;
    program.SetInfo("benzene-unittest", VERSION, __DATE__);
    BenzeneEnvironment::Get().RegisterProgram(program);
    program.Initialize(argc, &argv);
}

} // namespace

//----------------------------------------------------------------------------

#if BOOST_VERSION_MAJOR == 1 && BOOST_VERSION_MINOR == 32

#include <boost/test/auto_unit_test.hpp>
boost::unit_test::test_suite* init_unit_test_suite(int argc, char* argv[])
{
    Initialize();
    return boost::unit_test::ut_detail::auto_unit_test_suite();
}

//----------------------------------------------------------------------------

#elif BOOST_VERSION_MAJOR == 1 && BOOST_VERSION_MINOR == 33

#include <boost/test/auto_unit_test.hpp>
boost::unit_test::test_suite* init_unit_test_suite(int argc, char* argv[])
{
    UNUSED(argc);
    UNUSED(argv);
    Initialize();
    return boost::unit_test::auto_unit_test_suite();
}

//----------------------------------------------------------------------------

#elif BOOST_VERSION_MAJOR == 1 && BOOST_VERSION_MINOR >= 34

// Shamelessly copied from Fuego. :)

// Handling of unit testing framework initialization is messy and not
// documented in the Boost 1.34 documentation. See also:
// http://lists.boost.org/Archives/boost/2006/11/112946.php

#define BOOST_TEST_DYN_LINK // Must be defined before including unit_test.hpp
#include <boost/test/unit_test.hpp>

bool init_unit_test()
{
    Initialize();
    return true;
}

int main(int argc, char** argv)
{
    return boost::unit_test::unit_test_main(&init_unit_test, argc, argv);
}

//----------------------------------------------------------------------------

#else
#error "Unknown Boost version!"
#endif

//----------------------------------------------------------------------------
