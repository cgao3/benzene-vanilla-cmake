//----------------------------------------------------------------------------
/** @file Queue.hpp */
//----------------------------------------------------------------------------

#ifndef QUEUE_HPP
#define QUEUE_HPP

#include "Benzene.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Implements simple queue for vc builder.
    Current implementation behaves like stack,
    but it dosen't matter for vc builder. */
template <class T>
class Queue
{
private:
    std::vector<T> m_vec;

public:
    bool IsEmpty() const
    {
        return m_vec.empty();
    }

    void Push(const T& elem)
    {
        m_vec.push_back(elem);
    }

    T Pop()
    {
        T ret = m_vec.back();
        m_vec.pop_back();
        return ret;
    }
};

//---------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

 #endif // QUEUE_HPP
 