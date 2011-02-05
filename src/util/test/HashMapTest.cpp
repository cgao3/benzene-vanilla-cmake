//---------------------------------------------------------------------------
/** @file HashMapTest.cpp */
//---------------------------------------------------------------------------

#include <boost/test/auto_unit_test.hpp>

#include "SgSystem.h"
#include "HashMap.hpp"

using namespace benzene;

//---------------------------------------------------------------------------

namespace {

BOOST_AUTO_TEST_CASE(HashMap_AllTests)
{
    int data = 0;
    HashMap<int> hm(5);

    // check size()
    BOOST_CHECK_EQUAL(hm.Size(), 32u);
    BOOST_CHECK_EQUAL(hm.Count(), 0u);

    // table is empty, so better get fail!
    BOOST_CHECK(!hm.Get(1, data));

    // check Add()/Get()
    hm.Add(1, 5);
    BOOST_CHECK(hm.Exists(1));
    BOOST_CHECK(hm.Get(1, data));
    BOOST_CHECK_EQUAL(data, 5);
    BOOST_CHECK(!hm.Get(2, data));
    BOOST_CHECK_EQUAL(hm.Count(), 1u);

    // check collision will not clobber values (33 = 1 mod 32)
    hm.Add(33, 11);
    BOOST_CHECK(hm.Get(1, data));
    BOOST_CHECK_EQUAL(data, 5);
    BOOST_CHECK(hm.Get(33, data));
    BOOST_CHECK_EQUAL(data, 11);
    BOOST_CHECK_EQUAL(hm.Count(), 2u);

    // check update changes value
    hm.Update(1, 42);
    BOOST_CHECK(hm.Get(1, data));
    BOOST_CHECK_EQUAL(data, 42);
    BOOST_CHECK(hm.Get(33, data));
    BOOST_CHECK_EQUAL(data, 11);
    BOOST_CHECK_EQUAL(hm.Count(), 2u);
   
    // check operator=
    HashMap<int> mm(5);
    mm = hm;
    BOOST_CHECK(mm.Get(1, data));
    BOOST_CHECK_EQUAL(data, 42);
    BOOST_CHECK(mm.Get(33, data));
    BOOST_CHECK_EQUAL(data, 11);
    BOOST_CHECK_EQUAL(mm.Count(), 2u);

    // check copy constructor
    HashMap<int> blah(mm);
    BOOST_CHECK(blah.Get(1, data));
    BOOST_CHECK_EQUAL(data, 42);
    BOOST_CHECK(blah.Get(33, data));
    BOOST_CHECK_EQUAL(data, 11);
    BOOST_CHECK_EQUAL(blah.Count(), 2u);
}

BOOST_AUTO_TEST_CASE(HashMap_ConstIterator)
{
    HashMap<int> hm(5);
    {
        HashMapConstIterator<int> it(hm);
        BOOST_CHECK(!it);
    }
    hm.Add(3, 5);
    {
        HashMapConstIterator<int> it(hm);
        BOOST_CHECK(it);
        BOOST_CHECK_EQUAL(it.Hash(), 3);
        BOOST_CHECK_EQUAL(it.Data(), 5);
        ++it;
        BOOST_CHECK(!it);
    }
    hm.Add(2, 7);
    {
        HashMapConstIterator<int> it(hm);
        BOOST_CHECK(it);
        BOOST_CHECK_EQUAL(it.Hash(), 3);
        BOOST_CHECK_EQUAL(it.Data(), 5);
        ++it;
        BOOST_CHECK(it);
        BOOST_CHECK_EQUAL(it.Hash(), 2);
        BOOST_CHECK_EQUAL(it.Data(), 7);
        ++it;
        BOOST_CHECK(!it);
    }
}

}

//---------------------------------------------------------------------------
