//----------------------------------------------------------------------------
/** @file SortedSequenceTest.cpp */
//----------------------------------------------------------------------------

#include <boost/test/auto_unit_test.hpp>

#include "SortedSequence.hpp"

using namespace benzene;

//---------------------------------------------------------------------------

namespace {

BOOST_AUTO_TEST_CASE(SortedSequence_Empty)
{
    SortedSequence seq;
    BOOST_CHECK(seq.finished());

    SortedSequence ss(5, 0);
    BOOST_CHECK(!ss.finished());
    BOOST_CHECK_EQUAL(ss[0], 0);

    ++ss;
    BOOST_CHECK(ss.finished());
}

BOOST_AUTO_TEST_CASE(SortedSequence_one)
{
    SortedSequence seq(4, 1);
    
    BOOST_CHECK(!seq.finished());
    BOOST_CHECK_EQUAL(seq[0], 0);

    ++seq;
    BOOST_CHECK(!seq.finished());
    BOOST_CHECK_EQUAL(seq[0], 1);

    ++seq;
    BOOST_CHECK(!seq.finished());
    BOOST_CHECK_EQUAL(seq[0], 2);

    ++seq;
    BOOST_CHECK(!seq.finished());
    BOOST_CHECK_EQUAL(seq[0], 3);

    ++seq;
    BOOST_CHECK(seq.finished());
}

BOOST_AUTO_TEST_CASE(SortedSequence_two)
{
    std::vector<std::size_t> v(2);
    v[0] = 1;
    v[1] = 3;
    SortedSequence seq(5, v);
    
    BOOST_CHECK(!seq.finished());
    BOOST_CHECK_EQUAL(seq[0], 1);
    BOOST_CHECK_EQUAL(seq[1], 3);

    ++seq;
    BOOST_CHECK(!seq.finished());
    BOOST_CHECK_EQUAL(seq[0], 1);
    BOOST_CHECK_EQUAL(seq[1], 4);

    ++seq;
    BOOST_CHECK(!seq.finished());
    BOOST_CHECK_EQUAL(seq[0], 2);
    BOOST_CHECK_EQUAL(seq[1], 3);

    ++seq;
    BOOST_CHECK(!seq.finished());
    BOOST_CHECK_EQUAL(seq[0], 2);
    BOOST_CHECK_EQUAL(seq[1], 4);

    ++seq;
    BOOST_CHECK(!seq.finished());
    BOOST_CHECK_EQUAL(seq[0], 3);
    BOOST_CHECK_EQUAL(seq[1], 4);

    ++seq;
    BOOST_CHECK(seq.finished());
}

BOOST_AUTO_TEST_CASE(SortedSequence_three)
{
    SortedSequence seq(5, 3);
    
    BOOST_CHECK(!seq.finished());
    BOOST_CHECK_EQUAL(seq[0], 0);
    BOOST_CHECK_EQUAL(seq[1], 1);
    BOOST_CHECK_EQUAL(seq[2], 2);

    ++seq;
    BOOST_CHECK(!seq.finished());
    BOOST_CHECK_EQUAL(seq[0], 0);
    BOOST_CHECK_EQUAL(seq[1], 1);
    BOOST_CHECK_EQUAL(seq[2], 3);

    ++seq;
    BOOST_CHECK(!seq.finished());
    BOOST_CHECK_EQUAL(seq[0], 0);
    BOOST_CHECK_EQUAL(seq[1], 1);
    BOOST_CHECK_EQUAL(seq[2], 4);

    ++seq;
    BOOST_CHECK(!seq.finished());
    BOOST_CHECK_EQUAL(seq[0], 0);
    BOOST_CHECK_EQUAL(seq[1], 2);
    BOOST_CHECK_EQUAL(seq[2], 3);

    ++seq;
    BOOST_CHECK(!seq.finished());
    BOOST_CHECK_EQUAL(seq[0], 0);
    BOOST_CHECK_EQUAL(seq[1], 2);
    BOOST_CHECK_EQUAL(seq[2], 4);

    ++seq;
    BOOST_CHECK(!seq.finished());
    BOOST_CHECK_EQUAL(seq[0], 0);
    BOOST_CHECK_EQUAL(seq[1], 3);
    BOOST_CHECK_EQUAL(seq[2], 4);

    ++seq;
    BOOST_CHECK(!seq.finished());
    BOOST_CHECK_EQUAL(seq[0], 1);
    BOOST_CHECK_EQUAL(seq[1], 2);
    BOOST_CHECK_EQUAL(seq[2], 3);

    ++seq;
    BOOST_CHECK(!seq.finished());
    BOOST_CHECK_EQUAL(seq[0], 1);
    BOOST_CHECK_EQUAL(seq[1], 2);
    BOOST_CHECK_EQUAL(seq[2], 4);

    ++seq;
    BOOST_CHECK(!seq.finished());
    BOOST_CHECK_EQUAL(seq[0], 1);
    BOOST_CHECK_EQUAL(seq[1], 3);
    BOOST_CHECK_EQUAL(seq[2], 4);

    ++seq;
    BOOST_CHECK(!seq.finished());
    BOOST_CHECK_EQUAL(seq[0], 2);
    BOOST_CHECK_EQUAL(seq[1], 3);
    BOOST_CHECK_EQUAL(seq[2], 4);

    ++seq;
    BOOST_CHECK(seq.finished());
}

}

//---------------------------------------------------------------------------
