//----------------------------------------------------------------------------
/** @file HashDB.hpp
 */
//----------------------------------------------------------------------------

#ifndef HASHDB_H
#define HASHDB_H

#include <boost/concept_check.hpp>

#include <cstdio>
#include <string>

#include <db.h>

#include "Benzene.hpp"
#include "Types.hpp"
#include "Hash.hpp"
#include "BenzeneException.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Class supports Pack(), Unpack(), and PackedSize(). */
template<class T>
struct PackableConcept
{
    void constraints() 
    {
        const T t;
        int size = t.PackedSize();
        size = 42;  // to avoid non-used warning
        byte* d = t.Pack();

        T a = t;
        a.Unpack(d);
    }
};

/** Concept of a state in a HashDB. */
template<class T>
struct HashDBStateConcept
{
    void constraints() 
    {
        boost::function_requires< boost::DefaultConstructibleConcept<T> >();
        boost::function_requires< boost::AssignableConcept<T> >();
        boost::function_requires< PackableConcept<T> >();
    }
};

//----------------------------------------------------------------------------

/** Front end for a Berkely DB hash table. */
template<class T>
class HashDB
{
    BOOST_CLASS_REQUIRE(T, benzene, HashDBStateConcept);

public:
    /** Opens database, creates it if it does not exist. */
    HashDB(const std::string& filename);

    /** Closes database. */    
    ~HashDB();

    /** Returns true if hash exists in database. */
    bool Exists(hash_t hash) const;

    /** Returns true if get is successful. */
    bool Get(hash_t hash, T& data) const;

    /** Returns true if put is successful. */
    bool Put(hash_t hash, const T& data);

    /** Generic Put; for adding non (hash, value) pairs. */
    bool Put(void* k, int ksize, void* d, int dsize);

    /** Generic Get. */
    bool Get(void* k, int ksize, void* d, int dsize);

    /** Flush the db to disk. */
    void Flush();

private:

    static const int PERMISSION_FLAGS = 0664;

    static const int CLOSE_FLAGS = 0;

    DB* m_db;

    /** Name of database file. */
    std::string m_filename;
};

template<class T>
HashDB<T>::HashDB(const std::string& filename)
    : m_db(0),
      m_filename(filename)
{
    int ret;
    if ((ret = db_create(&m_db, NULL, 0)) != 0) 
    {
        fprintf(stderr, "db_create: %s\n", db_strerror(ret));
        throw BenzeneException("HashDB: opening/creating db!");
    }
    if ((ret = m_db->open(m_db, NULL, filename.c_str(), NULL, 
                          DB_HASH, DB_CREATE, PERMISSION_FLAGS)) != 0) 
    {
        m_db->err(m_db, ret, "%s", m_filename.c_str());
        throw BenzeneException("HashDB: error opening db!");
    }
}

template<class T>
HashDB<T>::~HashDB()
{
    int ret;
    if ((ret = m_db->close(m_db, CLOSE_FLAGS)) != 0) 
    {
        m_db->err(m_db, ret, "%s", m_filename.c_str());
        throw BenzeneException("HashDB: error closing db!");
    }
    m_db = 0;
}

template<class T>
bool HashDB<T>::Exists(hash_t hash) const
{
    DBT key, data;
    memset(&key, 0, sizeof(key)); 
    memset(&data, 0, sizeof(data)); 
    key.data = &hash;
    key.size = sizeof(hash);

    int ret = m_db->get(m_db, NULL, &key, &data, 0);
    switch(ret) {
    case 0:
        return true;

    case DB_NOTFOUND:
        return false;

    default:
        m_db->err(m_db, ret, "%s", m_filename.c_str());
        throw BenzeneException("HashDB: error in Exists()!");
    }

    return true;
}

template<class T>
bool HashDB<T>::Get(hash_t hash, T& d) const
{
    DBT key, data;
    memset(&key, 0, sizeof(key)); 
    memset(&data, 0, sizeof(data)); 

    key.data = &hash;
    key.size = sizeof(hash);

    int ret = m_db->get(m_db, NULL, &key, &data, 0);
    switch(ret) {
    case 0:
        d.Unpack(static_cast<byte*>(data.data));
        return true;

    case DB_NOTFOUND:
        return false;

    default:
        m_db->err(m_db, ret, "%s", m_filename.c_str());
        throw BenzeneException("HashDB: error in Get()!");
    }

    return false;
}

template<class T>
bool HashDB<T>::Get(void* k, int ksize, void* d, int dsize)
{
    DBT key, data;
    memset(&key, 0, sizeof(key)); 
    memset(&data, 0, sizeof(data)); 

    key.data = k;
    key.size = ksize;

    int ret = m_db->get(m_db, NULL, &key, &data, 0);
    switch(ret) {
    case 0:
        memcpy(d, data.data, dsize);
        return true;

    case DB_NOTFOUND:
        return false;

    default:
        m_db->err(m_db, ret, "%s", m_filename.c_str());
        throw BenzeneException("HashDB: error in general Get()!");
    }

    return false;
}

template<class T>
bool HashDB<T>::Put(hash_t hash, const T& d)
{
    DBT key, data; 
    memset(&key, 0, sizeof(key)); 
    memset(&data, 0, sizeof(data)); 

    key.data = &hash;
    key.size = sizeof(hash);

    data.data = d.Pack();
    data.size = d.PackedSize();

    int ret;
    if ((ret = m_db->put(m_db, NULL, &key, &data, 0)) != 0) {
        m_db->err(m_db, ret, "%s", m_filename.c_str());
        throw BenzeneException("HashDB: error in Put()!");
    } 

    return true;
}

template<class T>
bool HashDB<T>::Put(void* k, int ksize, void* d, int dsize)
{
    DBT key, data; 
    memset(&key, 0, sizeof(key)); 
    memset(&data, 0, sizeof(data)); 

    key.data = k;
    key.size = ksize;

    data.data = d;
    data.size = dsize;

    int ret;
    if ((ret = m_db->put(m_db, NULL, &key, &data, 0)) != 0) {
        m_db->err(m_db, ret, "%s", m_filename.c_str());
        throw BenzeneException("HashDB: error in general Put()!");
    } 

    return true;
}

template<class T>
void HashDB<T>::Flush()
{
    m_db->sync(m_db, 0);
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // HASHDB_H

