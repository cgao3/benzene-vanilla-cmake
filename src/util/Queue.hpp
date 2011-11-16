//----------------------------------------------------------------------------
/** @file Queue.hpp */
//----------------------------------------------------------------------------

#ifndef QUEUE_HPP
#define QUEUE_HPP

#include "Benzene.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Implements a simple queue. */
template <class T>
class Queue
{
private:
    std::vector<T> m_vec;
    std::size_t m_idx;

public:
    Queue() : m_idx(0) { }

    bool IsEmpty() const
    {
        return m_idx == m_vec.size();
    }

    void Push(const T& elem)
    {
        m_vec.push_back(elem);
    }

    T Pop()
    {
        return m_vec[m_idx++];
    }

    void Clear()
    {
        m_idx = 0;
        m_vec.clear();
    }
};

//---------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

 #endif // QUEUE_HPP
