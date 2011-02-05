//----------------------------------------------------------------------------
/** @file HashMap.hpp
    Contains a thread-safe lock-free constant-size hash map.
 */
//----------------------------------------------------------------------------

#ifndef HASHMAP_HPP
#define HASHMAP_HPP

#include <boost/concept_check.hpp>
#include <boost/scoped_array.hpp>

#include "SgHash.h"
#include "AtomicMemory.hpp"
#include "Benzene.hpp"
#include "BenzeneAssert.hpp"
#include "Logger.hpp"
#include "SafeBool.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Concept of a state in a hash table. */
template<class T>
struct HashMapStateConcept
{
    void constraints() 
    {
        boost::function_requires< boost::DefaultConstructibleConcept<T> >();
        boost::function_requires< boost::AssignableConcept<T> >();
    }
};

//----------------------------------------------------------------------------

/** Lock-free HashMap with 2^n slots. 

    Deletes and dynamic resizing are not supported. Thread-safe, so
    multiple threads can read/write data at the same time. Performs
    simple linear probing on hash collisions.

    Requires gcc builtins for atomic access.

    References:
    - Gao, H, Groote J.F., Hesselink W.H: 
    <a href="http://www.cs.rug.nl/~wim/mechver/hashtable/index.html"
    Lock-free Dynamic Hash table with Open Addressing</a>
    - Cliff Click Jr:
    <a href="http://blogs.azulsystems.com/cliff/2007/03/a_nonblocking_h.html"
    A Non-Blocking HashTable</a>
*/
template<typename T>
class HashMap
{
    BOOST_CLASS_REQUIRE(T, benzene, HashMapStateConcept);

public:
    /** Constructs a hashmap with 2^bits slots. */
    HashMap(unsigned bits);

    /** Copy constructor. */
    HashMap(const HashMap<T>& other);

    /** Destructor. */
    ~HashMap();

    /** Returns the lg2 of the number of slots. */
    unsigned Bits() const;

    /** Returns the number of slots. */
    unsigned Size() const;

    /** Returns number of objects stored. */
    unsigned Count() const;

    /** Retrieves object with key. 
        Returns true on success and object is copied into
        out. Otherwise, returns false.*/
    bool Get(SgHashCode key, T& out) const;

    /** Adds a new (key, object) pair.
     
        WILL ABORT IF TABLE IS FULL!

        Issues a warning if table becomes 1/4th full. 

        Make sure the table is much larger (at least 4x larger) than
        the maximum amount of data you ever expect to store in this
        table to reduce the number of probes on subsequent Get()
        calls.

        This function can possibly create duplicate entries. This is
        because Add() does not check the key of the slots it runs over
        while searching for an empty slot. So if two threads both try
        to Add() k1 at the same time, each will write to its own
        slot. Only the first slot written will ever be used by Get().
    */
    void Add(SgHashCode key, const T& obj);

    /** Updates the value of previously added object.
        Returns true on success and false on failure. */        
    bool Update(SgHashCode key, const T& obj);

    /** Returns true if key exists in map. */
    bool Exists(SgHashCode key) const;
    
    /** Clears the table. */
    void Clear();

    /** Copys other's data, overwriting everything in this table. */
    void operator=(const HashMap<T>& other);

private:
    template<typename U>
    friend class HashMapConstIterator;

    static const int EMPTY_SLOT = -1;

    struct Data
    {
        SgHashCode key;

        T value;
    };

    /** See Bits() */
    unsigned m_bits;

    /** See Size() */
    unsigned m_size;

    /** Equal to size() - 1. Used for bitmasking operations. */
    unsigned m_mask;

    /** See Count() */
    volatile unsigned m_count;

    /** Index into m_allocated at which data for this slot is located;
        index is equal to EMPTY_SLOT if unused. */
    boost::scoped_array<volatile int> m_used;

    /** Allocated space for entries in the table. */
    boost::scoped_array<Data> m_allocated;

    void CopyFrom(const HashMap<T>& other);

    bool FindKey(const SgHashCode& key, unsigned& slot) const;
};

template<typename T>
HashMap<T>::HashMap(unsigned bits)
    : m_bits(bits),
      m_size(1 << bits),
      m_mask(m_size - 1),
      m_count(0),
      m_used(new volatile int[m_size]),
      m_allocated(new Data[m_size])
{
    Clear();
}

template<typename T>
HashMap<T>::HashMap(const HashMap<T>& other)
    : m_bits(other.m_bits),
      m_size(other.m_size),
      m_mask(other.m_mask),
      m_count(other.m_count),
      m_used(new volatile int[m_size]),
      m_allocated(new Data[m_size])
{
    CopyFrom(other);
}

template<typename T>
HashMap<T>::~HashMap()
{
}

template<typename T>
inline unsigned HashMap<T>::Bits() const
{
    return m_bits;
}

template<typename T>
inline unsigned HashMap<T>::Size() const
{
    return m_size;
}

template<typename T>
inline unsigned HashMap<T>::Count() const
{
    return m_count;
}

/** Performs linear probing to find key. */
template<typename T>
bool HashMap<T>::FindKey(const SgHashCode& key, unsigned& slot) const
{
    // Search for slot containing this key.
    // Limit number of probes to a single pass: Table should never
    // fill up, but I'm being extra paranoid anyway.
    unsigned index = key.Hash(m_size);
    for (int guard = m_size; guard > 0; --guard)
    {
        index &= m_mask;
        if (m_used[index] == EMPTY_SLOT)
            return false;
        if (m_used[index] != EMPTY_SLOT
            && m_allocated[m_used[index]].key == key)
        {
            slot = index;
            return true;
        }
        index++;
    }
    return false;
}

template<typename T>
bool HashMap<T>::Get(SgHashCode key, T& out) const
{
    unsigned slot;
    if (FindKey(key, slot))
    {
        out = m_allocated[m_used[slot]].value;
        return true;
    }
    return false;
}

template<typename T>
bool HashMap<T>::Update(SgHashCode key, const T& value)
{
    unsigned slot;
    if (FindKey(key, slot))
    {
        m_allocated[m_used[slot]].value = value;
        return true;
    }
    return false;
}

template<typename T>
bool HashMap<T>::Exists(SgHashCode key) const
{
    unsigned slot;
    return FindKey(key, slot);
}

template<typename T>
void HashMap<T>::Add(SgHashCode key, const T& value)
{
    if (m_count > m_size)
    {
        LogSevere() << "HashMap: Full! Uhh oh...\n";
        abort();
    }
    else if (m_count > m_size / 4)
        LogWarning() << "HashMap: table becoming full...\n";

    // Atomic: get offset into allocated memory and increment m_count.
    int offset = FetchAndAdd(&m_count, 1u);

    // Copy data over
    m_allocated[offset].key = key;
    m_allocated[offset].value = value;

    // Find an empty slot
    unsigned index = key.Hash(m_size);
    while (true)
    {
        index &= m_mask;
        // Atomic: grab slot if unused
        if (CompareAndSwap(&m_used[index], EMPTY_SLOT, offset))
            break;
        index++;
    }
}

template<typename T>
void HashMap<T>::Clear()
{
    m_count = 0;
    for (unsigned i = 0; i < m_size; ++i)
        m_used[i] = EMPTY_SLOT;
}

template<typename T>
void HashMap<T>::CopyFrom(const HashMap<T>& other)
{
    BenzeneAssert(m_size == other.m_size);
    m_count = other.m_count;
    for (unsigned i = 0; i < m_size; ++i)
    {
        m_used[i] = other.m_used[i];
        m_allocated[i] = other.m_allocated[i];
    }
}

template<typename T>
void HashMap<T>::operator=(const HashMap<T>& other)
{
    CopyFrom(other);
}

//----------------------------------------------------------------------------

/** Iterator over a HashMap. 
    Iterates over the elements in the order they were added to the map. */
template<typename T>
class HashMapConstIterator : public SafeBool<HashMapConstIterator<T> >
{
public:
    HashMapConstIterator(const HashMap<T>& map);

    /** Returns hash of current entry. */
    SgHashCode Hash() const;

    /** Returns data in current entry. */
    const T& Data() const;

    /** Moves to next entry in the map. */
    void operator++();

    /** Used by SafeBool. */
    bool boolean_test() const;

private:
    const HashMap<T>& m_map;

    unsigned m_index;
};

template<typename T>
inline HashMapConstIterator<T>::HashMapConstIterator(const HashMap<T>& map)
    : m_map(map),
      m_index(0)
{
}

template<typename T>
inline SgHashCode HashMapConstIterator<T>::Hash() const
{
    return m_map.m_allocated[m_index].key;
}

template<typename T>
inline const T& HashMapConstIterator<T>::Data() const
{
    return m_map.m_allocated[m_index].value;
}

template<typename T>
inline void HashMapConstIterator<T>::operator++()
{
    ++m_index;
}

template<typename T>
inline bool HashMapConstIterator<T>::boolean_test() const
{
    return m_index < m_map.m_count;
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // HASHMAP_HPP
