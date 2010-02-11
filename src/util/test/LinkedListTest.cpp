//---------------------------------------------------------------------------
/** @file LinkedListTest.cpp
 */
//---------------------------------------------------------------------------

#include "SgSystem.h"
#include "SgRandom.h"

#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/test/auto_unit_test.hpp>

#include "LinkedList.hpp"

using namespace benzene;

//---------------------------------------------------------------------------

/** Set to true if verify correctness in lock-free mode. */
#define TEST_THREADING  0

//---------------------------------------------------------------------------

namespace {

Pool<int> pool;

BOOST_AUTO_TEST_CASE(LinkedList_BasicAdd)
{
    LinkedList<int> a(pool);
    
    BOOST_CHECK(a.Empty());
    
    a.Add(2);
    a.Add(3);
    a.Add(1);
    a.Add(7);
    a.Add(4);
    a.Add(3);
    a.Add(2);
    a.Add(1);
    a.Add(7);

    BOOST_CHECK(!a.Empty());

    ListIterator<int> it(a);
    BOOST_CHECK(it);
    BOOST_CHECK_EQUAL(*it, 1);
    ++it;
    BOOST_CHECK(it);
    BOOST_CHECK_EQUAL(*it, 2);
    ++it;
    BOOST_CHECK(it);
    BOOST_CHECK_EQUAL(*it, 3);
    ++it;
    BOOST_CHECK(it);
    BOOST_CHECK_EQUAL(*it, 4);
    ++it;
    BOOST_CHECK(it);
    BOOST_CHECK_EQUAL(*it, 7);
    ++it;
    BOOST_CHECK(!it);
}

BOOST_AUTO_TEST_CASE(LinkedList_BasicRemove)
{
    LinkedList<int> a(pool);
    
    BOOST_CHECK(a.Empty());
    
    a.Add(1);
    a.Remove(1);
    BOOST_CHECK(a.Empty());

    a.Add(1);
    a.Remove(2);
    BOOST_CHECK(!a.Empty());

    {
        ListIterator<int> it(a);
        BOOST_CHECK(it);
        BOOST_CHECK_EQUAL(*it, 1);
        ++it;
        BOOST_CHECK(!it);
    }

    a.Add(2);
    a.Add(3);
    a.Remove(2);
    {
        ListIterator<int> it(a);
        BOOST_CHECK(it);
        BOOST_CHECK_EQUAL(*it, 1);
        ++it;
        BOOST_CHECK(it);
        BOOST_CHECK_EQUAL(*it, 3);
        ++it;
        BOOST_CHECK(!it);
    }

    a.Remove(1);
    {
        ListIterator<int> it(a);
        BOOST_CHECK(it);
        BOOST_CHECK_EQUAL(*it, 3);
        ++it;
        BOOST_CHECK(!it);
    }

    a.Remove(3);
    {
        BOOST_CHECK(a.Empty());
        ListIterator<int> it(a);
        BOOST_CHECK(!it);
    }
}

#if TEST_THREADING

struct Thread
{
    int m_threadId;

    boost::barrier& m_barrier;

    LinkedList<int>& m_lst;

    SgRandom m_random;

    Thread(int threadId, boost::barrier& b, LinkedList<int>& l)
        : m_threadId(threadId), m_barrier(b), m_lst(l)
    {
    };

    void operator()()
    {
        m_barrier.wait();
        for (int i = 0; i < 100; ++i)
        {
            if (m_threadId && (i % m_threadId == 0))
            {
                int num = m_random.Int(100);
                int count = 0;
                for (ListIterator<int> it(m_lst); it; ++it, ++count)
                {
                    if (count == num)
                    {
                        m_lst.Remove(*it);
                        break;
                    }
                }
            }
            m_lst.Add(m_random.Int(100000) / (m_threadId+1));
        }
    }
};

BOOST_AUTO_TEST_CASE(LinkedList_Threading)
{
    const int NUM_THREADS = 10;
    LinkedList<int> a(pool);
    std::vector<boost::shared_ptr<boost::thread> > threads;
    boost::barrier barrier(NUM_THREADS + 1);
    for (int i = 0; i < NUM_THREADS; ++i)
    {
        Thread runnable(i, barrier, a);
        boost::shared_ptr<boost::thread> thread(new boost::thread(runnable));
        threads.push_back(thread);
    }
    barrier.wait();
    for (int i = 0; i < NUM_THREADS; ++i)
    {
        threads[i]->join();
    }
    std::cout << '[';
    for (ListIterator<int> it(a); it; ++it)
        std::cout << *it << ", ";
    std::cout << "]\n";

    ListIterator<int> it(a);
    int old = *it;
    ++it;
    while (it)
    {
        
        BOOST_CHECK(old < *it);
        old = *it;
        ++it;
    }
}

#endif // TEST_THREADING

}

//---------------------------------------------------------------------------
