//----------------------------------------------------------------------------
/** @file SortedSequence.hpp */
//----------------------------------------------------------------------------

#ifndef SORTEDSEQUENCE_H
#define SORTEDSEQUENCE_H

#include <vector>
#include <iostream>
#include "Benzene.hpp"
#include "BenzeneAssert.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Iterates over valid sorted sequences. */
class SortedSequence
{
public:
    /** Creates an empty sorted sequence. */
    SortedSequence();

    /** Creates a seq with max values max and num indices. */
    SortedSequence(std::size_t max, std::size_t num);

    /** Creates a sequence with max value max and current indices seq; 
        seq must be strictly increasing. */
    SortedSequence(std::size_t max, const std::vector<std::size_t>& seq);

    /** Returns true if no more valid sequences. */
    bool finished() const;

    /** Returns the nth index. */
    const std::size_t& operator[](std::size_t n) const;

    /** Updates indices to next valid sorted sequence. */
    void operator++();

private:
    std::size_t m_max;

    std::vector<std::size_t> m_seq;
};


inline SortedSequence::SortedSequence()
    : m_max(1),
      m_seq(1, 1)
{ }

inline SortedSequence::SortedSequence(std::size_t max, std::size_t num)
    : m_max(max),
      m_seq(num, 0)
{
    // handle the case where num == 0 explicitly
    if (num == 0) 
    {
        m_max = 1;
        m_seq = std::vector<std::size_t>(1, 0);
    }
    for (std::size_t i = 0; i < m_seq.size(); ++i) 
        m_seq[i] = i;
}

inline SortedSequence::SortedSequence(std::size_t max, 
                                      const std::vector<std::size_t>& seq)
    : m_max(max),
      m_seq(seq)
{ }

inline bool SortedSequence::finished() const
{
    return m_seq[0] > m_max - m_seq.size();
}

inline const std::size_t& SortedSequence::operator[](std::size_t n) const
{
    BenzeneAssert(n < m_seq.size());
    return m_seq[n];
}

inline void SortedSequence::operator++()
{
    std::size_t i = m_seq.size() - 1;
    std::size_t off = 0;
    while (true) 
    {
        m_seq[i]++;

        if (m_seq[i] < m_max - off)
            break;

        if (i == 0) 
            break;

        i--;
        off++;
    }

    for (; i < m_seq.size() - 1; ++i)
        m_seq[i + 1] = m_seq[i] + 1;
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // SORTEDSEQUENCE_H
