//----------------------------------------------------------------------------
/** @file
 */
//----------------------------------------------------------------------------

#ifndef BITSET_ITERATOR_HPP
#define BITSET_ITERATOR_HPP

#include "Benzene.hpp"
#include "Hex.hpp"
#include "SafeBool.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Iterates over the set bits in a bitset. 
    Similar to PointIterator. */
class BitsetIterator : public SafeBool<BitsetIterator>
{
public:

    /** Constructor. */
    BitsetIterator(const bitset_t& bs);

    /** Returns the HexPoint at the current location. */
    HexPoint operator*();

    /** Moves to the next set point in the bistset. */
    void operator++();

    /** Used by SafeBool. */
    bool boolean_test() const;

private:
    void find_next_set_bit();

    size_t m_index;
    bitset_t m_bitset;
};

inline BitsetIterator::BitsetIterator(const bitset_t& bs)
    : m_index(bs._Find_first()),
      m_bitset(bs)
{
}

inline void BitsetIterator::find_next_set_bit()
{
    m_index = m_bitset._Find_next(m_index);
}

inline HexPoint BitsetIterator::operator*()
{
    return static_cast<HexPoint>(m_index);
}

inline void BitsetIterator::operator++()
{
    find_next_set_bit();
}
        
inline bool BitsetIterator::boolean_test() const
{
    return (m_index < FIRST_INVALID);
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // BITSET_ITERATOR_HPP
