//----------------------------------------------------------------------------
/** @file BitsetMap.hpp */
//----------------------------------------------------------------------------

#ifndef BITSETMAP_HPP
#define BITSETMAP_HPP

#include "Hex.hpp"
#include "BenzeneBitset.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

template <class T>
class BitsetMap
{
public:
    BitsetMap();

    T* operator[](HexPoint x) const;
    void Put(HexPoint x, T* entry);

    bitset_t Entries() const;

protected:
    bitset_t m_set;
    T* m_entries[BITSETSIZE];

    void ClearEntries();
};

template <class T>
inline void BitsetMap<T>::ClearEntries()
{
    memset(m_entries, 0, sizeof(m_entries));
}

template <class T>
inline BitsetMap<T>::BitsetMap()
{
    ClearEntries();
}

template <class T>
inline T* BitsetMap<T>::operator[](HexPoint x) const
{
    return m_entries[x];
}

template <class T>
inline void BitsetMap<T>::Put(HexPoint x, T* entry)
{
    m_set.set(x);
    m_entries[x] = entry;
}

template <class T>
inline bitset_t BitsetMap<T>::Entries() const
{
    return m_set;
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // BITSET_MAP_HPP
