//----------------------------------------------------------------------------
/** @file HashDB.hpp
 */
//----------------------------------------------------------------------------

#ifndef HASHDB_H
#define HASHDB_H

#include <boost/concept_check.hpp>

#include <cassert>
#include <cstdio>
#include <string>

#include <db.h>

#include "Benzene.hpp"
#include "Types.hpp"
#include "Hash.hpp"

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

    /** Constructor. */
    HashDB();
    
    /** Destructor. */
    ~HashDB();

    //------------------------------------------------------------------------
    
    /** Opens filename, creates it if it does not exist. */
    bool Open(std::string filename);

    /** Closes the database. */
    bool Close();

    //------------------------------------------------------------------------

    /** Returns true if hash exists in database. */
    bool Exists(hash_t hash);

    /** Returns true if get is successful. */
    bool Get(hash_t hash, T& data) const;

    /** Returns true if put is successful. */
    bool Put(hash_t hash, const T& data);

    /** Generic Put; for adding non (hash, value) pairs. */
    bool Put(void* k, int ksize, void* d, int dsize);

    /** Generic Get. */
    bool Get(void* k, int ksize, void* d, int dsize);

    //------------------------------------------------------------------------

    /** Flush the db to disk. */
    void Flush();

    /** Returns pointer to DB handle. */
    DB* BerkelyDB();

private:

    static const int PERMISSION_FLAGS = 0664;
    static const int CLOSE_FLAGS = 0;

    DB* m_db;
    std::string m_filename;
};

template<class T>
HashDB<T>::HashDB()
    : m_db(0)
{
}

template<class T>
HashDB<T>::~HashDB()
{
    if (m_db) {
        Close();
    }
}

//----------------------------------------------------------------------------

template<class T>
bool HashDB<T>::Open(std::string filename)
{
    assert(m_db == 0);

    m_filename = filename;

    int ret;
    if ((ret = db_create(&m_db, NULL, 0)) != 0) {
        fprintf(stderr, "db_create: %s\n", db_strerror(ret));
        m_db = 0;
        return false;
    }

    if ((ret = m_db->open(m_db, NULL, filename.c_str(), NULL, 
                          DB_HASH, DB_CREATE, PERMISSION_FLAGS)) != 0) 
    {
        m_db->err(m_db, ret, "%s", m_filename.c_str());
        delete m_db;
        return false;
    }

    return true;
}

template<class T>
bool HashDB<T>::Close()
{
    assert(m_db);

    int ret;
    if ((ret = m_db->close(m_db, CLOSE_FLAGS)) != 0) {
        m_db->err(m_db, ret, "%s", m_filename.c_str());
        return false;
    }
    return true;    
}

//----------------------------------------------------------------------------

template<class T>
bool HashDB<T>::Exists(hash_t hash)
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
        assert(false);
    }

    return true;
}

template<class T>
bool HashDB<T>::Get(hash_t hash, T& d) const
{
    assert(m_db);
    
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
        assert(false);
    }

    return false;
}

template<class T>
bool HashDB<T>::Get(void* k, int ksize, void* d, int dsize)
{
    assert(m_db);
    
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
        assert(false);
    }

    return false;
}

//----------------------------------------------------------------------------

template<class T>
bool HashDB<T>::Put(hash_t hash, const T& d)
{
    assert(m_db);

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
        return false;
    } 

    return true;
}

template<class T>
bool HashDB<T>::Put(void* k, int ksize, void* d, int dsize)
{
    assert(m_db);

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
        return false;
    } 

    return true;
}

//----------------------------------------------------------------------------

template<class T>
void HashDB<T>::Flush()
{
    m_db->sync(m_db, 0);
}

template<class T>
DB* HashDB<T>::BerkelyDB()
{
    return m_db;
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // HASHDB_H

