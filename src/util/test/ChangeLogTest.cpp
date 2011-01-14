//----------------------------------------------------------------------------
/** @file ChangeLogTest.cpp */
//----------------------------------------------------------------------------

#include <boost/test/auto_unit_test.hpp>

#include "ChangeLog.hpp"

using namespace benzene;

//---------------------------------------------------------------------------

namespace {

BOOST_AUTO_TEST_CASE(ChangeLog_InitialState)
{
    ChangeLog<float> cl;
    BOOST_CHECK(cl.Empty());
    BOOST_CHECK_EQUAL(cl.Size(), 0);
}

BOOST_AUTO_TEST_CASE(ChangeLog_PushPopTopAndClear)
{
    ChangeLog<float> cl;
    cl.Push(ChangeLog<float>::ADD, 0.1f);
    cl.Push(ChangeLog<float>::REMOVE, 0.2f);
    cl.Push(ChangeLog<float>::REMOVE, 0.3f);
    cl.Push(ChangeLog<float>::MARKER, 0.4f);
    BOOST_CHECK(!cl.Empty());
    BOOST_CHECK_EQUAL(cl.Size(), 4);
    // testing TopAction and TopData
    BOOST_CHECK_EQUAL(cl.TopAction(), ChangeLog<float>::MARKER);
    BOOST_CHECK_EQUAL(cl.TopData(), 0.4f);
    BOOST_CHECK(!cl.Empty());
    BOOST_CHECK_EQUAL(cl.Size(), 4);

    cl.Clear();
    cl.Push(ChangeLog<float>::MARKER, 1.0f);
    cl.Push(ChangeLog<float>::ADD, 1.1f);
    BOOST_CHECK_EQUAL(cl.TopAction(), ChangeLog<float>::ADD);
    BOOST_CHECK_EQUAL(cl.TopData(), 1.1f);
    cl.Push(ChangeLog<float>::REMOVE, 1.2f);
    BOOST_CHECK(!cl.Empty());
    BOOST_CHECK_EQUAL(cl.Size(), 3);
    cl.Pop();
    cl.Pop();
    BOOST_CHECK(!cl.Empty());
    BOOST_CHECK_EQUAL(cl.Size(), 1);
    BOOST_CHECK_EQUAL(cl.TopAction(), ChangeLog<float>::MARKER);
    BOOST_CHECK_EQUAL(cl.TopData(), 1.0f);
    cl.Pop();
    BOOST_CHECK_EQUAL(cl.Size(), 0);
    // check clear on Empty changelog, ensure does not affect activation
    BOOST_CHECK(cl.Empty());
    cl.Clear();
    BOOST_CHECK(cl.Empty());
}

}

//---------------------------------------------------------------------------
