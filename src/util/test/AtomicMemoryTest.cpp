//---------------------------------------------------------------------------
/** @file AtomicMemoryTest.cpp */
//---------------------------------------------------------------------------

#include <boost/test/auto_unit_test.hpp>

#include "AtomicMemory.hpp"

using namespace benzene;

//---------------------------------------------------------------------------

namespace 
{

BOOST_AUTO_TEST_CASE(AtomicMemory_Tests)
{
    volatile int a = 5;
    BOOST_CHECK_EQUAL(FetchAndAdd(&a, 1), 5);
    BOOST_CHECK_EQUAL(FetchAndAdd(&a, 2), 6);
    BOOST_CHECK_EQUAL(FetchAndAdd(&a, 3), 8);
    BOOST_CHECK_EQUAL(FetchAndAdd(&a, -1), 11);
    BOOST_CHECK_EQUAL(a, 10);

    a = 5;
    BOOST_CHECK(!CompareAndSwap(&a, 7, 5));
    BOOST_CHECK(CompareAndSwap(&a, 5, 7));
    BOOST_CHECK_EQUAL(a, 7);
}

}

//---------------------------------------------------------------------------
