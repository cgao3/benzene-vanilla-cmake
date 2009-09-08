//----------------------------------------------------------------------------
/** @file HashMap.hpp
    Contains a thread-safe lock-free constant-size hash map.
 */
//----------------------------------------------------------------------------

#ifndef HASHMAP_H
#define HASHMAP_H

#include <boost/concept_check.hpp>
#include <boost/scoped_array.hpp>

#include "Benzene.hpp"
#include "Hash.hpp"
#include "Logger.hpp"
#include <cassert>

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

    Deletes and dynamic resizing are not supported.  Thread-safe, so
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
    unsigned bits() const;

    /** Returns the number of slots. */
    unsigned size() const;

    /** Returns number of objects stored. */
    unsigned count() const;

    /** Retrieves object with key. Returns true on success. */
    bool get(hash_t key, T& out) const;

    /** Stores a (key, object) pair. 
     
        WILL ABORT IF TABLE IS FULL!

        Issues a warning if table becomes 1/4th full. 

        Make sure the table is much larger (at least 4x larger) than
        the maximum amount of data you ever expect to store in this
        table. This is to reduce the number of probes on subsequent
        get() calls, as well as to avoid the need for dynamic resizing
        of the table.

        This function can possibly create duplicate entries. This is
        because put() does not check the key of the slots it runs over
        while searching for an empty slot. So if two threads both
        write try to put() k1 at the same time, each will write to its
        own slot. Only the first slot written will ever be used by
        get().
     */
    void put(hash_t key, const T& obj);
    
    /** Clears the table. */
    void clear();

    /** Copys other's data, overwriting everything in this table. */
    void operator=(const HashMap<T>& other);

private:    
    
    static const int EMPTY_SLOT = -1;

    struct Data
    {
        hash_t key;
        T value;
    };

    /** See bits() */
    unsigned m_bits;

    /** See size() */
    unsigned m_size;

    /** Equal to size() - 1. Used for bitmasking operations. */
    unsigned m_mask;

    /** See count() */
    volatile unsigned m_count;

    /** Index into m_allocated at which data for this slot is located;
        index is equal to EMPTY_SLOT if unused.
     */
    boost::scoped_array<volatile int> m_used;

    /** Allocated space for entries in the table. */
    boost::scoped_array<Data> m_allocated;

    void CopyFrom(const HashMap<T>& other);
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
    clear();
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
inline unsigned HashMap<T>::bits() const
{
    return m_bits;
}

template<typename T>
inline unsigned HashMap<T>::size() const
{
    return m_size;
}

template<typename T>
inline unsigned HashMap<T>::count() const
{
    return m_count;
}

/** Performs linear probing to find key. */
template<typename T>
bool HashMap<T>::get(hash_t key, T& out) const
{
    // Search for slot containing this key.
    // Limit number of probes to a single pass: Table should never
    // fill up, but I'm being extra paranoid anyway.
    hash_t index = key;
    for (int guard = m_size; guard > 0; --guard)
    {
        index &= m_mask;
        if (m_used[index] == EMPTY_SLOT)
            return false;
        if (m_used[index] != EMPTY_SLOT
            && m_allocated[m_used[index]].key == key)
        {
            out = m_allocated[m_used[index]].value;
            return true;
        }
        index++;
    }
    return false;
}

template<typename T>
void HashMap<T>::put(hash_t key, const T& value)
{
    if (m_count > m_size)
    {
        LogSevere() << "HashMap: Full! Uhh oh..." << '\n';
        abort();
    }
    else if (m_count > m_size / 4)
        LogWarning() << "HashMap: table becoming full..." << '\n';

    // Atomic: get offset into allocated memory and increment m_count.
    int offset = __sync_fetch_and_add(&m_count, 1);

    // Copy data over
    m_allocated[offset].key = key;
    m_allocated[offset].value = value;

    // Find an empty slot
    hash_t index = key;
    while (true)
    {
        index &= m_mask;
        // Atomic: grab slot if unused
        if (__sync_bool_compare_and_swap(&m_used[index], EMPTY_SLOT, offset))
            break;
        index++;
    }
}

template<typename T>
void HashMap<T>::clear()
{
    m_count = 0;
    for (unsigned i = 0; i < m_size; ++i)
        m_used[i] = EMPTY_SLOT;
}

template<typename T>
void HashMap<T>::CopyFrom(const HashMap<T>& other)
{
    assert(m_size == other.m_size);
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

_END_BENZENE_NAMESPACE_

#endif // HASHMAP_H
