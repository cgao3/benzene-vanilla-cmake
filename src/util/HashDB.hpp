//----------------------------------------------------------------------------
/** @file HashDB.hpp */
//----------------------------------------------------------------------------

#ifndef HASHDB_H
#define HASHDB_H

#include <boost/concept_check.hpp>

#include <cstdio>
#include <cstring>
#include <string>

#include <db.h>

#include "SgHash.h"
#include "Benzene.hpp"
#include "Types.hpp"
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
    HashDB(const std::string& filename, const std::string& type);

    /** Closes database. */    
    ~HashDB();

    /** Returns true if hash exists in database. */
    bool Exists(SgHashCode hash) const;

    /** Returns true if get is successful. */
    bool Get(SgHashCode hash, T& data) const;

    /** Returns true if put is successful. */
    bool Put(SgHashCode hash, const T& data);

    /** Generic Put; for adding non (hash, value) pairs. */
    bool Put(void* k, int ksize, void* d, int dsize);

    /** Generic Get. */
    bool Get(void* k, int ksize, void* d, int dsize) const;

    /** Flush the db to disk. */
    void Flush();

    /** Returns statistics of the berkeley db. */
    std::string BDBStatistics();

private:

    static const int PERMISSION_FLAGS = 0664;

    static const int CLOSE_FLAGS = 0;

    struct Header
    {
        static const int MAX_LENGTH = 32;

        char m_type[MAX_LENGTH];
        
        Header()
        {
            memset(m_type, 0, MAX_LENGTH);
        }

        Header(const std::string& type)
        {
            strncpy(m_type, type.c_str(), MAX_LENGTH - 1);
            m_type[MAX_LENGTH - 1] = 0; // always null-terminated
        }

        bool operator==(const Header& other) const
        {
            return strcmp(m_type, other.m_type) == 0;
        }

        bool operator!=(const Header& other) const
        {
            return !operator==(other);
        }
    };

    DB* m_db;

    /** Name of database file. */
    std::string m_filename;

    bool GetHeader(Header& header) const;
    
    void PutHeader(Header& header);
};

template<class T>
HashDB<T>::HashDB(const std::string& filename, const std::string& type)
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
    Header newHeader(type);
    Header oldHeader;
    if (GetHeader(oldHeader))
    {
        if (newHeader != oldHeader)
            throw BenzeneException() 
                << "HashDB: Conflicting database types. "
                << "old: '" << oldHeader.m_type << "' "
                << "new: '" << newHeader.m_type << "'\n";
    }
    else
        PutHeader(newHeader);
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
bool HashDB<T>::GetHeader(Header& header) const
{
    static char key[32] = "dbtype";
    return Get(key, sizeof(key), &header, sizeof(header));
}

template<class T>
void HashDB<T>::PutHeader(Header& header)
{
    static char key[32] = "dbtype";
    Put(key, sizeof(key), &header, sizeof(header));
}

template<class T>
bool HashDB<T>::Exists(SgHashCode hash) const
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
bool HashDB<T>::Get(SgHashCode hash, T& d) const
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
bool HashDB<T>::Get(void* k, int ksize, void* d, int dsize) const
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
bool HashDB<T>::Put(SgHashCode hash, const T& d)
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

template<class T>
std::string HashDB<T>::BDBStatistics()
{
    DB_HASH_STAT* stats_ptr;
    int ret;
    if ((ret = m_db->stat(m_db, NULL, &stats_ptr, 0)) != 0) {
        m_db->err(m_db, ret, "%s", m_filename.c_str());
        return "Error; no stats returned.";
    }
    DB_HASH_STAT& stats = *stats_ptr;
    std::ostringstream os;
    os << "[\n";
    os << "magic=" << stats.hash_magic << '\n';
    os << "version=" << stats.hash_version << '\n';
    os << "metaflags=" << stats.hash_metaflags << '\n';
    os << "nkeys=" << stats.hash_nkeys << '\n';
    os << "ndata=" << stats.hash_ndata << '\n';
    os << "pagesize=" << stats.hash_pagesize << '\n';
    os << "fillfactor=" << stats.hash_ffactor << '\n';
    os << "buckets=" << stats.hash_buckets << '\n';
    os << "freepages=" << stats.hash_free << '\n';
    os << "bytesfree=" << stats.hash_bfree << '\n';
    os << "bigpages=" << stats.hash_bigpages << '\n';
    os << "bytesbfree=" << stats.hash_big_bfree << '\n';
    os << "overflowpages=" << stats.hash_overflows << '\n';
    os << "ovfl_free=" << stats.hash_ovfl_free << '\n';
    os << "dup_pages=" << stats.hash_dup << '\n';
    os << "dup_free=" << stats.hash_dup_free << '\n';
    os << ']';
    return os.str();
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // HASHDB_H

