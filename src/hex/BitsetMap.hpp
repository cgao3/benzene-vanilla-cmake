//----------------------------------------------------------------------------
/** @file BitsetMapBase.hpp */
//----------------------------------------------------------------------------

#ifndef BITSETMAP_HPP
#define BITSETMAP_HPP

#include "Hex.hpp"
#include "BenzeneBitset.hpp"
#include "BitsetIterator.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

template <class T>
class BitsetMapBase
{
public:
    T* operator[](HexPoint x) const;
    T* Put(HexPoint x, T* entry = new T());
    void Remove(HexPoint x);

    bitset_t Entries() const;

protected:
    BitsetMapBase();
    T* At(HexPoint x) const;
    void ClearEntries();
    void ResetEntries();

private:
    bitset_t m_set;
    T* m_entries[BITSETSIZE];
};

template <class T>
class BitsetMap : public BitsetMapBase<T>
{
public:
    void Delete(HexPoint x);
    ~BitsetMap();
    void Reset();
    using BitsetMapBase<T>::At;
    using BitsetMapBase<T>::Remove;
    using BitsetMapBase<T>::Entries;
private:
    using BitsetMapBase<T>::ResetEntries;
    void Destroy();
};

template <class T>
class BitsetUPairMap
{
public:
    class Nbs : public BitsetMapBase<T>
    {
    public:
        void Destroy(HexPoint x);
        void Reset(HexPoint x);
        using BitsetMapBase<T>::At;
        using BitsetMapBase<T>::Entries;
        using BitsetMapBase<T>::ResetEntries;
    };

    ~BitsetUPairMap();
    void Reset();

    const Nbs& operator[](HexPoint x) const;
    T* Put(HexPoint x, HexPoint y, T* entry = new T());
    void Delete(HexPoint x, HexPoint y, T* xy_entry);
    void Delete(HexPoint x, HexPoint y);

private:
    Nbs m_xmap[BITSETSIZE];
};

template <class T>
inline void BitsetMapBase<T>::ClearEntries()
{
    memset(m_entries, 0, sizeof(m_entries));
}

template <class T>
inline BitsetMapBase<T>::BitsetMapBase()
{
    ClearEntries();
}

template <class T>
inline T* BitsetMapBase<T>::At(HexPoint x) const
{
    return m_entries[x];
}

template <class T>
inline T* BitsetMapBase<T>::operator[](HexPoint x) const
{
    return At(x);
}

template <class T>
inline T* BitsetMapBase<T>::Put(HexPoint x, T* entry)
{
    m_set.set(x);
    m_entries[x] = entry;
    return entry;
}

template <class T>
inline void BitsetMapBase<T>::ResetEntries()
{
    ClearEntries();
    m_set.reset();
}

template <class T>
inline void BitsetMapBase<T>::Remove(HexPoint x)
{
    m_set.reset(x);
    m_entries[x] = 0;
}

template <class T>
inline bitset_t BitsetMapBase<T>::Entries() const
{
    return m_set;
}

template <class T>
inline void BitsetMap<T>::Delete(HexPoint x)
{
    delete At(x);
    Remove(x);
}

template <class T>
inline void BitsetMap<T>::Reset()
{
    Destroy();
    ResetEntries();
}

template <class T>
inline void BitsetMap<T>::Destroy()
{
    for (BitsetIterator i(Entries()); i; ++i)
        delete At(*i);
}

template <class T>
inline BitsetMap<T>::~BitsetMap()
{
    Destroy();
}

template <class T>
inline void BitsetUPairMap<T>::Nbs::Destroy(HexPoint x)
{
    for (BitsetIterator i(Entries()); i; ++i)
    {
        if (*i > x)
            break;
        delete At(*i);
    }
}

template <class T>
inline void BitsetUPairMap<T>::Nbs::Reset(HexPoint x)
{
    Destroy(x);
    ResetEntries();
}

template <class T>
inline BitsetUPairMap<T>::~BitsetUPairMap()
{
    for (int i = 0; i < BITSETSIZE; i++)
        m_xmap[i].Destroy(HexPoint(i));
}

template <class T>
inline void BitsetUPairMap<T>::Reset()
{
    for (int i = 0; i < BITSETSIZE; i++)
        m_xmap[i].Reset(HexPoint(i));
}

template <class T>
inline const typename BitsetUPairMap<T>::Nbs& BitsetUPairMap<T>::operator[](HexPoint x) const
{
    return m_xmap[x];
}

template <class T>
inline T* BitsetUPairMap<T>::Put(HexPoint x, HexPoint y, T* entry)
{
    m_xmap[x].Put(y, entry);
    m_xmap[y].Put(x, entry);
    return entry;
}

template <class T>
inline void BitsetUPairMap<T>::Delete(HexPoint x, HexPoint y, T* xy_entry)
{
    delete xy_entry;
    m_xmap[x].Remove(y);
    m_xmap[y].Remove(x);
}

template <class T>
inline void BitsetUPairMap<T>::Delete(HexPoint x, HexPoint y)
{
    T* entry = m_xmap[x][y];
    if (entry)
        Delete(x, y, entry);
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // BITSET_MAP_HPP
