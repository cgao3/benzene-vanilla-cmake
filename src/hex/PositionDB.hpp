//----------------------------------------------------------------------------
/** @file PositionDB.hpp
 */
//----------------------------------------------------------------------------

#ifndef POSITIONDB_HPP
#define POSITIONDB_HPP

#include <boost/concept_check.hpp>
#include "HashDB.hpp"
#include "StoneBoard.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

namespace {

hash_t GetHash(const StoneBoard& pos)
{
    hash_t hash1 = pos.Hash();
    StoneBoard rotatedBrd(pos);
    rotatedBrd.RotateBoard();
    hash_t hash2 = rotatedBrd.Hash();
    return std::min(hash1, hash2);
}

/** Data must be stored for the boardstate of the minimum hash. */
inline bool NeedToRotate(const StoneBoard& pos, hash_t minHash)
{
    return pos.Hash() != minHash;
}

}

//----------------------------------------------------------------------------

/** Class is rotatable by calling Rotate(). */
template<class T>
struct RotatableConcept
{
    void constraints() 
    {
        const T t;
        const ConstBoard& brd = ConstBoard::Get(1, 1);
        t.Rotate(brd);
    }
};

/** Concept of a state in a HashDB. */
template<class T>
struct PositionDBStateConcept
{
    void constraints() 
    {
        boost::function_requires< HashDBStateConcept<T> >();
        boost::function_requires< RotatableConcept<T> >();
    }
};

//----------------------------------------------------------------------------

/** Database of hex positions handling rotations. */
template<class T>
class PositionDB
{
    BOOST_CLASS_REQUIRE(T, benzene, PositionDBStateConcept);

public:
    /** Opens database, creates it if it does not exist. */
    PositionDB(const std::string& filename);

    /** Closes database. */    
    ~PositionDB();

    /** Returns true if position exists in database. */
    bool Exists(const StoneBoard& pos) const;

    /** Returns true if get is successful. */
    bool Get(const StoneBoard& pos, T& data) const;

    /** Returns true if put is successful. */
    bool Put(const StoneBoard& brd, const T& data);

    void Flush();

private:
    HashDB<T> m_db;
};

template<class T>
PositionDB<T>::PositionDB(const std::string& filename)
    : m_db(filename)
{
}

template<class T>
PositionDB<T>::~PositionDB()
{
}

template<class T>
bool PositionDB<T>::Exists(const StoneBoard& brd) const
{
    return m_db.Exists(GetHash(brd));
}

template<class T>
bool PositionDB<T>::Get(const StoneBoard& brd, T& data) const
{
    hash_t hash = GetHash(brd);
    if (!m_db.Get(hash, data))
        return false;
    if (NeedToRotate(brd, hash))
        data.Rotate(brd.Const());
    return true;
}

template<class T>
bool PositionDB<T>::Put(const StoneBoard& brd, const T& data)
{
    hash_t hash = GetHash(brd);
    T myData(data);
    if (NeedToRotate(brd, hash))
        myData.Rotate(brd.Const());
    return m_db.Put(hash, myData);
}

template<class T>
void PositionDB<T>::Flush()
{
    m_db.Flush();
}

//----------------------------------------------------------------------------

/** Set of positions; handles rotations. */
class PositionSet
{
public:
    PositionSet();

    ~PositionSet();

    void Insert(const StoneBoard& brd);

    bool Exists(const StoneBoard& brd) const;

    std::size_t Size() const;

private:
    std::set<hash_t> m_set;
};

inline PositionSet::PositionSet()
{
}

inline PositionSet::~PositionSet()
{
}

inline void PositionSet::Insert(const StoneBoard& brd)
{
    m_set.insert(GetHash(brd));
}

inline bool PositionSet::Exists(const StoneBoard& brd) const
{
    return m_set.count(GetHash(brd)) > 0;
}

inline std::size_t PositionSet::Size() const
{
    return m_set.size();
}

//----------------------------------------------------------------------------

/** Map of positions; handles rotations. */
template<class T>
class PositionMap
{
public:
    PositionMap();

    ~PositionMap();

    bool Exists(const StoneBoard& pos) const;

    T& operator[](const StoneBoard& pos);

private:
    std::map<hash_t, T> m_map;
};

template<class T>
inline PositionMap<T>::PositionMap()
{
}

template<class T>
inline PositionMap<T>::~PositionMap()
{
}

template<class T>
bool PositionMap<T>::Exists(const StoneBoard& pos) const
{
    return m_map.find(GetHash(pos)) != m_map.end();
}

template<class T>
inline T& PositionMap<T>::operator[](const StoneBoard& pos)
{
    return m_map[GetHash(pos)];
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // POSITIONDB_HPP

