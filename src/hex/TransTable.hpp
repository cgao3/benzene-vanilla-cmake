//----------------------------------------------------------------------------
/** @file TransTable.hpp
 */
//----------------------------------------------------------------------------

#ifndef TRANSTABLE_H
#define TRANSTABLE_H

#include <boost/concept_check.hpp>

#include "Hex.hpp"
#include "HashTable.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Concept of a state in a transposition table. 
 
    @todo WHY IS HASH REQUIRED HERE? WHAT WAS I THINKING?!?!
 */
template<class T>
struct TransTableStateConcept
{
    void constraints() 
    {
        boost::function_requires< HashTableStateConcept<T> >();

        const T a;

        // FIXME: are these done properly?
        if (a.Initialized()) { }

        hash_t hash = a.Hash();
        hash++; // to avoid compiler warning

        const T b;

        if (a.ReplaceWith(b)) { }
    }
};

//----------------------------------------------------------------------------

/** Transposition table. The state class must meet the requirements of
    TransTableStateConcept.  */
template<class T>
class TransTable
{
    BOOST_CLASS_REQUIRE(T, benzene, TransTableStateConcept);

public:

    /** Creates table with 2^n slots. */
    TransTable(int bits);

    /** Destructor. */
    ~TransTable();

    /** Returns lg2 of number of entries. */
    unsigned bits() const;

    /** Returns the number of slots in the TT. */
    unsigned size() const;

    /** Clears the table. */
    void clear();

    /** Stores data in slot determined by data.Hash(). New data
        overwrites old only if "old.ReplaceWith(new)" is true.
    */
    bool put(const T& data);
    
    /** Returns true if the slot for hash contains a state with that
        hash value, data is copied into data if so. Otherwise, nothing
        is copied into data.

        Does not check for hash collisions; calling code is
        responsible for that if CHECK_HASH_COLLISIONS is defined and
        non-zero.
    */
    bool get(hash_t hash, T& data);

    /** Returns statistics in string form. */
    std::string stats() const;

private:

    // -----------------------------------------------------------------------

    struct Statistics
    {
        unsigned reads;
        unsigned hits;
        unsigned writes;
        unsigned collisions;

        Statistics() : reads(0), hits(0), writes(0), collisions(0) { }
    };
   
    // -----------------------------------------------------------------------

    HashTable<T> m_hashtable;
    Statistics m_stats;
};

//----------------------------------------------------------------------------

template<typename T>
TransTable<T>::TransTable(int bits)
    : m_hashtable(bits),
      m_stats()
{
}

template<typename T>
TransTable<T>::~TransTable()
{
}

template<typename T>
inline unsigned TransTable<T>::bits() const
{
    return m_hashtable.bits();
}

template<typename T>
inline unsigned TransTable<T>::size() const
{
    return m_hashtable.size();
}

template<typename T>
inline void TransTable<T>::clear()
{
    m_hashtable.clear();
}

template<typename T>
bool TransTable<T>::put(const T& data)
{
    hash_t hash = data.Hash();

    if (m_hashtable[hash].ReplaceWith(data)) {
        m_stats.writes++;
        m_hashtable[hash] = data;
    }
    return true;
}

template<typename T>
bool TransTable<T>::get(hash_t hash, T& data)
{
    m_stats.reads++;
    T& old = m_hashtable[hash];
    bool ret = old.Initialized() && (old.Hash() == hash);
    if (ret) {
        data = old;
        m_stats.hits++; 
    }
    return ret;
}

template<typename T>
std::string TransTable<T>::stats() const
{  
    std::ostringstream os;
    os << '\n'
       << "TT statistics\n"
       << "      reads: " << m_stats.reads << std::endl
       << "       hits: " << m_stats.hits << " (" 
       << "     writes: " << m_stats.writes << std::endl
       << " collisions: " << m_stats.collisions << std::endl;
    return os.str();
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // TRANSTABLE_H
