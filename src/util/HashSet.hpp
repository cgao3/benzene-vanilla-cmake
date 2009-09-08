//----------------------------------------------------------------------------
/** @file HashSet.hpp
 */
//----------------------------------------------------------------------------

#ifndef HASHSET_H
#define HASHSET_H

#include <boost/scoped_array.hpp>

#include "Benzene.hpp"
#include "Hash.hpp"
#include <cassert>

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** HashSet with 2^n slots. 

    Deletes and dynamic resizing are not supported.  Not thread-safe. 
    Performs simple linear probing on hash collisions.

    Uses a changelog for quick clears O(n) in number of added entries.
*/
class HashSet
{
public:
    /** Constructs a hashset with 2^bits slots. */
    HashSet(unsigned bits);

    /** Copy constructor. */
    HashSet(const HashSet& other);

    /** Destructor. */
    ~HashSet();

    /** Returns the lg2 of the number of slots. */
    unsigned bits() const;

    /** Returns the number of slots. */
    unsigned size() const;

    /** Returns number of objects stored. */
    unsigned count() const;

    /** Returns true if key is in set. */
    bool exists(hash_t key) const;

    /** Adds a key. 
     
        WILL ABORT IF TABLE IS FULL!

        Issues a warning if table becomes 1/4th full. 

        Make sure the table is much larger (at least 4x larger) than
        the maximum amount of data you ever expect to store in this
        table. This is to reduce the number of probes on subsequent
        get() calls, as well as to avoid the need for dynamic resizing
        of the table.
     */
    void add(hash_t key);
    
    /** Clears the table. */
    void clear();

    /** Copys other's data, overwriting everything in this table. */
    void operator=(const HashSet& other);

private:    
    
    static const int EMPTY_SLOT = -1;

    struct Data
    {
        hash_t key;
    };

    /** See bits() */
    unsigned m_bits;

    /** See size() */
    unsigned m_size;

    /** Equal to size() - 1. Used for bitmasking operations. */
    unsigned m_mask;

    /** See count() */
    unsigned m_count;

    /** Index into m_allocated at which data for this slot is located;
        index is equal to EMPTY_SLOT if unused.
     */
    boost::scoped_array<int> m_used;

    /** Allocated space for entries in the table. */
    boost::scoped_array<Data> m_allocated;

    /** History of slots that have been filled. */
    std::vector<int> m_changelog;

    void CopyFrom(const HashSet& other);
};

inline HashSet::HashSet(unsigned bits)
    : m_bits(bits),
      m_size(1 << bits),
      m_mask(m_size - 1),
      m_count(0),
      m_used(new int[m_size]),
      m_allocated(new Data[m_size]),
      m_changelog()
{
    m_changelog.reserve(m_size);
    for (unsigned i = 0; i < m_size; ++i)
        m_used[i] = EMPTY_SLOT;
}

inline HashSet::HashSet(const HashSet& other)
    : m_bits(other.m_bits),
      m_size(other.m_size),
      m_mask(other.m_mask),
      m_count(other.m_count),
      m_used(new int[m_size]),
      m_allocated(new Data[m_size]),
      m_changelog()
{
    m_changelog.reserve(m_size);
    CopyFrom(other);
}

inline HashSet::~HashSet()
{
}

inline unsigned HashSet::bits() const
{
    return m_bits;
}

inline unsigned HashSet::size() const
{
    return m_size;
}

inline unsigned HashSet::count() const
{
    return m_count;
}

/** Performs linear probing to find key. */
inline bool HashSet::exists(hash_t key) const
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
            return true;
        }
        index++;
    }
    return false;
}

inline void HashSet::add(hash_t key)
{
    if (m_count > m_size)
    {
        std::cerr << "HashSet: Full! Uhh oh..." << '\n';
        abort();
    }
    else if (m_count > m_size / 4)
        std::cerr << "HashSet: table becoming full..." << '\n';

    int offset = m_count++;
    m_allocated[offset].key = key;

    // Find an empty slot
    hash_t index = key;
    while (true)
    {
        index &= m_mask;
        if (m_used[index] == EMPTY_SLOT)
        {
            m_used[index] = offset;
            m_changelog.push_back(index);
            break;
        }
        index++;
    }
}

inline void HashSet::clear()
{
    for (unsigned i = 0; i < m_changelog.size(); ++i)
        m_used[m_changelog[i]] = EMPTY_SLOT;
    m_changelog.clear();
    m_count = 0;
}

inline void HashSet::CopyFrom(const HashSet& other)
{
    assert(m_size == other.m_size);
    m_count = other.m_count;
    for (unsigned i = 0; i < m_size; ++i)
    {
        m_used[i] = other.m_used[i];
        m_allocated[i] = other.m_allocated[i];
    }
    m_changelog = other.m_changelog;
}

inline void HashSet::operator=(const HashSet& other)
{
    CopyFrom(other);
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // HASHMAP_H
