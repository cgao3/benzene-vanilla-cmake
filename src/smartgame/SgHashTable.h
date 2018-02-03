//----------------------------------------------------------------------------
/** @file SgHashTable.h
    Hash table. */
//----------------------------------------------------------------------------

#ifndef SG_HASHTABLE_H
#define SG_HASHTABLE_H

#include "SgHash.h"
#include "SgWrite.h"

//----------------------------------------------------------------------------

/** Entry in a HashTable: code and data */
template <class DATA>
struct SgHashEntry
{
    SgHashEntry()
        : m_hash(),
          m_data()
    { }

    SgHashEntry(const SgHashCode& code, const DATA& data)
        : m_hash(code),
          m_data(data)
    { }

    SgHashCode m_hash;

    DATA m_data;
};

//----------------------------------------------------------------------------

/** SgHashTable implements an array of DATA.
	The implementation probes the table in BLOCK_SIZE consecutive locations,
    as in the Fruit chess program.
	See http://arctrix.com/nas/chess/fruit/fruit_21_linux.zip, file trans.cpp
    (accessed on Oct 23, 2011).
    The new entry is always written, overwriting the least valuable among
    the BLOCK_SIZE entries.
    The table size is increased by BLOCK_SIZE - 1 entries to avoid
    an expensive modulo operation in the scan.
    A good value for BLOCK_SIZE is 4.
*/
template <class DATA, int BLOCK_SIZE = 1>
class SgHashTable
{
public:
    /** Local const iterator */
    class Iterator
    {
    public:
        Iterator(const SgHashTable& array);

        const SgHashEntry<DATA>& operator*() const;

        const SgHashEntry<DATA>* operator->() const;

        void operator++();

        operator bool() const;

    private:
        void SkipInvalidEntries();

        const SgHashEntry<DATA>* m_end;
        
        const SgHashEntry<DATA>* m_current;
    };

    /** Create a hash table with 'maxHash' entries. */
    explicit SgHashTable(int maxHash);

    ~SgHashTable();

    /** Leaves the positions in the hash table, but set all depths to zero, so
        that only the best move is valid, not the value. The hash entries will
        easily be replaced by fresh information. */
    void Age();

    /** Clear the hash table by marking all entries as invalid. */
    void Clear();

    /** Return true and the data stored under that code, or false if
        none stored. */
    bool Lookup(const SgHashCode& code, DATA* data) const;

    /** Size of hash table. */
    int MaxHash() const;

    /** Try to store 'data' under the hash code 'code'.
        Return whether the data was stored. The only reason for not storing
        it would be some 'better' data already hashing to the same hash code. */
    bool Store(const SgHashCode& code, const DATA& data);

    /** number of collisions on store */
    size_t NuCollisions() const
    {
        return m_nuCollisions;
    }

    /** total number of stores attempted */
    size_t NuStores() const
    {
        return m_nuStores;
    }

    /** total number of lookups attempted */
    size_t NuLookups() const
    {
        return m_nuLookups;
    }

    /** number of successful lookups */
    size_t NuFound() const
    {
        return m_nuFound;
    }

private:
    friend class Iterator;

    /** complete hash code for each entry */
    SgHashEntry<DATA>* m_entry;

    /** size of hash table */
    int m_maxHash;

    // @todo the following statistics can be made debug only
    // @todo pass a HashStatistics class to the HashTable when constructed

    /** number of collisions on store */
    mutable size_t m_nuCollisions;

    /** total number of stores attempted */
    mutable size_t m_nuStores;

    /** total number of lookups attempted */
    mutable size_t m_nuLookups;

    /** number of successful lookups */
    mutable size_t m_nuFound;

    /** not implemented */
    SgHashTable(const SgHashTable&);

    /** not implemented */
    SgHashTable& operator=(const SgHashTable&);
};

template <class DATA, int BLOCK_SIZE>
SgHashTable<DATA, BLOCK_SIZE>::Iterator::Iterator(const SgHashTable& ht)
    : m_end(ht.m_entry + ht.m_maxHash + BLOCK_SIZE - 1),
      m_current(ht.m_entry)
{
    SkipInvalidEntries();
}

template <class DATA, int BLOCK_SIZE>
const SgHashEntry<DATA>& SgHashTable<DATA, BLOCK_SIZE>::Iterator::operator*() const
{
    SG_ASSERT(*this);
    SG_ASSERT(m_current);
    return *m_current;
}

template <class DATA, int BLOCK_SIZE>
const SgHashEntry<DATA>* SgHashTable<DATA, BLOCK_SIZE>::Iterator::operator->() const
{
    SG_ASSERT(*this);
    SG_ASSERT(m_current);
    return m_current;
}

template <class DATA, int BLOCK_SIZE>
void SgHashTable<DATA, BLOCK_SIZE>::Iterator::operator++()
{
    ++m_current;
    SkipInvalidEntries();
}

template <class DATA, int BLOCK_SIZE>
SgHashTable<DATA, BLOCK_SIZE>::Iterator::operator bool() const
{
    return m_current < m_end;
}

template <class DATA, int BLOCK_SIZE>
void SgHashTable<DATA, BLOCK_SIZE>::Iterator::SkipInvalidEntries()
{
    while (m_current < m_end && !m_current->m_data.IsValid())
        ++m_current;
}

template <class DATA, int BLOCK_SIZE>
SgHashTable<DATA, BLOCK_SIZE>::SgHashTable(int maxHash)
 : m_entry(0),
   m_maxHash(maxHash),
   m_nuCollisions(0),
   m_nuStores(0),
   m_nuLookups(0),
   m_nuFound(0)
{
    m_entry = new SgHashEntry<DATA>[m_maxHash + BLOCK_SIZE - 1];
    Clear();
}

template <class DATA, int BLOCK_SIZE>
SgHashTable<DATA, BLOCK_SIZE>::~SgHashTable()
{
    delete[] m_entry;
}

template <class DATA, int BLOCK_SIZE>
void SgHashTable<DATA, BLOCK_SIZE>::Age()
{
    for (int i = m_maxHash + BLOCK_SIZE - 2; i >= 0; --i)
        m_entry[i].m_data.AgeData();
}

template <class DATA, int BLOCK_SIZE>
void SgHashTable<DATA, BLOCK_SIZE>::Clear()
{
    for (int i = m_maxHash + BLOCK_SIZE - 2; i >= 0; --i)
    {
        m_entry[i].m_data.Invalidate();
    }
}

template <class DATA, int BLOCK_SIZE>
int SgHashTable<DATA, BLOCK_SIZE>::MaxHash() const
{
    return m_maxHash;
}

template <class DATA, int BLOCK_SIZE>
bool SgHashTable<DATA, BLOCK_SIZE>::Store(const SgHashCode& code, const DATA& data)
{
    ++m_nuStores;
    int h = code.Hash(m_maxHash);
    int best = -1;
    bool collision = true;
    for (int i = h; i < h + BLOCK_SIZE; i++)
    {
        if (! m_entry[i].m_data.IsValid() || m_entry[i].m_hash == code)
        {
            best = i;
            collision = false;
            break;
        }
        else if (  best == -1 
                || m_entry[best].m_data.IsBetterThan(m_entry[i].m_data)
                )
            best = i;
    }
    if (collision)
    	++m_nuCollisions;
    SG_ASSERTRANGE(best, h, h + BLOCK_SIZE - 1);
    SgHashEntry<DATA>& entry = m_entry[best];
    entry.m_hash = code;
    entry.m_data = data;
    return true;
}

template <class DATA, int BLOCK_SIZE>
bool SgHashTable<DATA, BLOCK_SIZE>::Lookup(const SgHashCode& code, DATA* data) const
{
    ++m_nuLookups;
    int h = code.Hash(m_maxHash);
    for (int i = h; i < h + BLOCK_SIZE; i++)
     {
        const SgHashEntry<DATA>& entry = m_entry[i];
        if (entry.m_data.IsValid())
        {
            if (entry.m_hash == code)
            {
                *data = entry.m_data;
                ++m_nuFound;
                return true;
            }
        }
        else
            return false;
     }
     return false;
}

//----------------------------------------------------------------------------

/** Writes statistics on hash table use (not the content) */
template <class DATA, int BLOCK_SIZE>
std::ostream& operator<<(std::ostream& out, const SgHashTable<DATA, BLOCK_SIZE>& hash)
{    
    out << "HashTableStatistics:\n"
        << SgWriteLabel("Stores") << hash.NuStores() << '\n'
        << SgWriteLabel("LookupAttempt") << hash.NuLookups() << '\n'
        << SgWriteLabel("LookupSuccess") << hash.NuFound() << '\n'
        << SgWriteLabel("Collisions") << hash.NuCollisions() << '\n';
    return out;
}

#endif // SG_HASHTABLE_H
