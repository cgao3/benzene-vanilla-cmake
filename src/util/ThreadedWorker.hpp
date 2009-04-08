//----------------------------------------------------------------------------
/** @file ThreadedWorker.hpp
 */
//----------------------------------------------------------------------------

#ifndef THREADEDWORKER_HPP
#define THREADEDWORKER_HPP

#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>

//----------------------------------------------------------------------------

template<typename I, typename O, typename W>
class ThreadedWorker
{
public:
    
    ThreadedWorker(std::vector<W>& workers);

    ~ThreadedWorker();

    void DoWork(const std::vector<I>& work, 
                std::vector<std::pair<I,O> >& output);
    
private:

    void StartDoingWork();

    void WaitForThreadsToFinish();

    void TellThreadsToQuit();

    friend class Thread;

    /** Copyable object run in a boost::thread. */
    class Thread
    {
    public:
        Thread(std::size_t threadId, W& worker, 
               ThreadedWorker<I,O,W>& threadedWork);

        void operator()();

    private:

        std::size_t m_id;

        W& m_worker;
            
        ThreadedWorker<I,O,W>& m_boss;
    };

    /** Flag telling threads to exit. */
    bool m_quit;

    /** Threads must lock this mutex before getting work from list. */
    boost::mutex m_workMutex;

    /** Threads must lock this mutex before updating output. */
    boost::mutex m_outputMutex;

    /** Threads block on this barrier until told to start. */
    boost::barrier m_startWork;

    /** Threads block on this barrier until all are finished. */
    boost::barrier m_workFinished;

    /** Index of next problem to solve. */
    std::size_t m_workIndex;

    /** Problems to solve. */
    const std::vector<I>* m_workToDo;

    /** Solved problems. */
    std::vector<std::pair<I,O> >* m_output;

    /** The threads. */
    std::vector<boost::shared_ptr<boost::thread> > m_threads;
};

//----------------------------------------------------------------------------

template<typename I, typename O, typename W>
ThreadedWorker<I,O,W>::ThreadedWorker(std::vector<W>& workers)
    : m_quit(false),
      m_startWork(workers.size() + 1),
      m_workFinished(workers.size() + 1)
{
    for (std::size_t i = 0; i < workers.size(); ++i)
    {
        Thread runnable((int)i, workers[i], *this);
        boost::shared_ptr<boost::thread> thread(new boost::thread(runnable));
        m_threads.push_back(thread);
    }
}

template<typename I, typename O, typename W>
ThreadedWorker<I,O,W>::~ThreadedWorker()
{
    TellThreadsToQuit();
    for (std::size_t i = 0; i < m_threads.size(); ++i)
    {
        m_threads[i]->join();
        LogInfo() << "ThreadedWorker: joined " << i << '\n';
    }
}

template<typename I, typename O, typename W>
void ThreadedWorker<I,O,W>::DoWork(const std::vector<I>& work,
                                   std::vector<std::pair<I,O> >& output)
{
    m_workToDo = &work;
    m_workIndex = 0;
    m_output = &output;
    LogInfo() << "ThreadedWorker::DoWork(): Processing " 
              << work.size() << " jobs." << '\n';
    StartDoingWork();
    WaitForThreadsToFinish();
}

template<typename I, typename O, typename W>
ThreadedWorker<I,O,W>::Thread::Thread(std::size_t threadId, W& worker, 
                                      ThreadedWorker<I,O,W>& threadedWorker)
    : m_id(threadId),
      m_worker(worker),
      m_boss(threadedWorker)
{
}

template<typename I, typename O, typename W>
void ThreadedWorker<I,O,W>::Thread::operator()()
{
    while (true)
    {
        m_boss.m_startWork.wait();
        if (m_boss.m_quit) 
            break;
        LogInfo() << "[" << m_id << "]: starting..."  << '\n';
        while (true)
        {
            bool finished = false;
            const I* currentWork = 0;
            {
                boost::mutex::scoped_lock lock(m_boss.m_workMutex);
                if (m_boss.m_workIndex < m_boss.m_workToDo->size())
                    currentWork = &(*m_boss.m_workToDo)[m_boss.m_workIndex++];
                else
                    finished = true;
            }
            if (finished)
                break;
            O answer = m_worker(*currentWork);
            {
                boost::mutex::scoped_lock lock(m_boss.m_outputMutex);
                m_boss.m_output
                    ->push_back(std::make_pair(*currentWork, answer));
            }
        }
        LogInfo() << "[" << m_id << "]: finished." << '\n';
        m_boss.m_workFinished.wait();
    }
}

template<typename I, typename O, typename W>
void ThreadedWorker<I,O,W>::StartDoingWork()
{
    m_startWork.wait();
}

template<typename I, typename O, typename W>
void ThreadedWorker<I,O,W>::WaitForThreadsToFinish()
{
    m_workFinished.wait();
}

template<typename I, typename O, typename W>
void ThreadedWorker<I,O,W>::TellThreadsToQuit()
{
    m_quit = true;
    m_startWork.wait();
}

//----------------------------------------------------------------------------

#endif // THREADEDWORKER_HPP
