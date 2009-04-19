//----------------------------------------------------------------------------
/** @file
 */
//----------------------------------------------------------------------------

#ifndef SORTEDSEQUENCE_H
#define SORTEDSEQUENCE_H

#include <vector>
#include <iostream>
#include "Benzene.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** 
    
*/
class SortedSequence
{
public:

    /** Creates an empty sorted sequence. */
    SortedSequence();

    /** Creates a seq with max values max, and num indices. */
    SortedSequence(int max, int num);

    /** Creates a sequence with max value max and current indices seq; 
        seq must be strictly increasing. */
    SortedSequence(int max, const std::vector<int>& seq);

    /** Returns true if no more valid sequences. */
    bool finished() const;

    /** Returns a reference to the nth index. 
        If these value is changed to make the sequence not strictly
        increasing, further output is undefined. */
    int& operator[](int n);

    /** Updates indices to next valid sorted sequence. */
    void operator++();

    /** Returns the indices as a vector of integers. */
    std::vector<int>& indices();

private:
    int m_max;
    std::vector<int> m_seq;
};


inline SortedSequence::SortedSequence()
    : m_max(0),
      m_seq(1, 1)
{ }

inline SortedSequence::SortedSequence(int max, int num)
    : m_max(max),
      m_seq(num, 0)
{
    // handle the case where num == 0 explicitly
    if (num == 0) {
        m_max = 1;
        m_seq = std::vector<int>(1, 0);
    }
    
    for (int i=0; i<(int)m_seq.size(); ++i) 
        m_seq[i] = i;
}

inline SortedSequence::SortedSequence(int max, const std::vector<int>& seq)
    : m_max(max),
      m_seq(seq)
{ }

inline bool SortedSequence::finished() const
{
    return (m_seq[0] > m_max-(int)m_seq.size());
}

inline int& SortedSequence::operator[](int n)
{
    assert(0 <= n && n < (int)m_seq.size());
    return m_seq[n];
}

inline std::vector<int>& SortedSequence::indices()
{
    return m_seq;
}

inline void SortedSequence::operator++()
{
    unsigned i = m_seq.size()-1;
    int off = 0;
    while (true) {
        m_seq[i]++;

        if (m_seq[i] < m_max-off)
            break;

        if (i == 0) 
            break;

        i--;
        off++;
    }

    for (; i<m_seq.size()-1; ++i)
        m_seq[i+1] = m_seq[i]+1;
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // SORTEDSEQUENCE_H
