//----------------------------------------------------------------------------
/** @file HashTable.hpp
 */
//----------------------------------------------------------------------------

#ifndef HASHMAP_H
#define HASHMAP_H

#include <boost/concept_check.hpp>

#include <vector>
#include "Hash.hpp"

//----------------------------------------------------------------------------

/** Concept of a state in a hash table. */
template<class T>
struct HashTableStateConcept
{
    void constraints() 
    {
        boost::function_requires< boost::DefaultConstructibleConcept<T> >();
        boost::function_requires< boost::AssignableConcept<T> >();
    }
};

//----------------------------------------------------------------------------

/** 
    The HashTable has 2^n slots, each slot contains room for a single
    element of type T.  The table is initialized with elements using
    T's default constructor.
*/
template<typename T>
class HashTable
{
    BOOST_CLASS_REQUIRE(T, , HashTableStateConcept);

public:

    /** Constructs a hashmap with 2^bits entries. */
    HashTable(unsigned bits);

    /** Destructor. */
    virtual ~HashTable();

    /** Returns the lg2 of the number of entries. */
    unsigned bits() const;

    /** Returns the number of entries in the hash map. */
    unsigned size() const;

    /** Returns access to element in slot for hash. */
    T& operator[] (hash_t hash);

    /** Clears the table. */
    void clear();

private:    
    unsigned m_bits;
    unsigned m_size;
    unsigned m_mask;
    std::vector<T> m_data;
};

template<typename T>
HashTable<T>::HashTable(unsigned bits)
    : m_bits(bits),
      m_size(1 << bits),
      m_mask(m_size - 1),
      m_data(m_size)
{
}

template<typename T>
HashTable<T>::~HashTable()
{
}

template<typename T>
inline unsigned HashTable<T>::bits() const
{
    return m_bits;
}

template<typename T>
inline unsigned HashTable<T>::size() const
{
    return m_size;
}

template<typename T>
T& HashTable<T>::operator[](hash_t hash)
{
    return m_data[hash & m_mask];
}

template<typename T>
void HashTable<T>::clear()
{
    m_data = std::vector<T>(m_size);
}

#endif // HASHMAP_H

//----------------------------------------------------------------------------

