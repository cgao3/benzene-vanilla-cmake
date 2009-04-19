//----------------------------------------------------------------------------
/** @file
 */
//----------------------------------------------------------------------------

#include <boost/test/auto_unit_test.hpp>

#include "ChangeLog.hpp"

using namespace benzene;

//---------------------------------------------------------------------------

namespace {

BOOST_AUTO_TEST_CASE(ChangeLog_InitialState)
{
    ChangeLog<float> cl;
    BOOST_CHECK(cl.empty());
    BOOST_CHECK_EQUAL(cl.size(), 0);
}

BOOST_AUTO_TEST_CASE(ChangeLog_PushPopTopAndClear)
{
    ChangeLog<float> cl;
    cl.push(ChangeLog<float>::ADD, 0.1f);
    cl.push(ChangeLog<float>::REMOVE, 0.2f);
    cl.push(ChangeLog<float>::REMOVE, 0.3f);
    cl.push(ChangeLog<float>::MARKER, 0.4f);
    BOOST_CHECK(!cl.empty());
    BOOST_CHECK_EQUAL(cl.size(), 4);
    // testing topAction and topData
    BOOST_CHECK_EQUAL(cl.topAction(), ChangeLog<float>::MARKER);
    BOOST_CHECK_EQUAL(cl.topData(), 0.4f);
    BOOST_CHECK(!cl.empty());
    BOOST_CHECK_EQUAL(cl.size(), 4);

    cl.clear();
    cl.push(ChangeLog<float>::MARKER, 1.0f);
    cl.push(ChangeLog<float>::ADD, 1.1f);
    BOOST_CHECK_EQUAL(cl.topAction(), ChangeLog<float>::ADD);
    BOOST_CHECK_EQUAL(cl.topData(), 1.1f);
    cl.push(ChangeLog<float>::REMOVE, 1.2f);
    BOOST_CHECK(!cl.empty());
    BOOST_CHECK_EQUAL(cl.size(), 3);
    cl.pop();
    cl.pop();
    BOOST_CHECK(!cl.empty());
    BOOST_CHECK_EQUAL(cl.size(), 1);
    BOOST_CHECK_EQUAL(cl.topAction(), ChangeLog<float>::MARKER);
    BOOST_CHECK_EQUAL(cl.topData(), 1.0f);
    cl.pop();
    BOOST_CHECK_EQUAL(cl.size(), 0);
    // check clear on empty changelog, ensure does not affect activation
    BOOST_CHECK(cl.empty());
    cl.clear();
    BOOST_CHECK(cl.empty());
}

BOOST_AUTO_TEST_CASE(ChangeLog_Dump)
{
    ChangeLog<float> cl;
    BOOST_CHECK_EQUAL(cl.dump(), "");
    cl.push(ChangeLog<float>::ADD, 2.0f);
    cl.push(ChangeLog<float>::MARKER, 2.1f);
    cl.push(ChangeLog<float>::REMOVE, 2.2f);
    std::ostringstream s;
    s << "0:    ADD: 2" << std::endl
      << "1: MARKER" << std::endl
      << "2: REMOVE: 2.2" << std::endl;
    BOOST_CHECK_EQUAL(cl.dump(), s.str());
}

}

//---------------------------------------------------------------------------
