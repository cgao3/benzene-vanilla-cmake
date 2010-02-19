//----------------------------------------------------------------------------
/** @file PositionDB.hpp
 */
//----------------------------------------------------------------------------

#ifndef POSITIONDB_HPP
#define POSITIONDB_HPP

#include <boost/concept_check.hpp>
#include "HashDB.hpp"
#include "HexState.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

namespace {

hash_t GetHash(const HexState& state)
{
    hash_t hash1 = state.Hash();
    HexState rotatedState(state);
    rotatedState.Position().RotateBoard();
    hash_t hash2 = rotatedState.Hash();
    return std::min(hash1, hash2);
}

/** Data must be stored for the state of the minimum hash. */
inline bool NeedToRotate(const HexState& state, hash_t minHash)
{
    return state.Hash() != minHash;
}

}

//----------------------------------------------------------------------------

/** Class is rotatable by calling Rotate(). */
template<class T>
struct RotatableConcept
{
    void constraints() 
    {
        const ConstBoard& brd = ConstBoard::Get(1, 1);
        T t;
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

    struct Statistics
    {
        std::size_t m_gets;

        std::size_t m_hits;

        std::size_t m_puts;

        std::size_t m_rotations;

        std::string Write() const;

        Statistics();
    };

    /** Opens database, creates it if it does not exist. */
    PositionDB(const std::string& filename, const std::string& type);

    /** Closes database. */    
    ~PositionDB();

    /** Returns true if position exists in database. */
    bool Exists(const HexState& pos) const;

    /** Returns true if get is successful. */
    bool Get(const HexState& pos, T& data) const;

    /** Returns true if put is successful. */
    bool Put(const HexState& brd, const T& data);

    void Flush();

    Statistics GetStatistics() const;

private:
    HashDB<T> m_db;

    mutable Statistics m_stats;
};

template<class T>
PositionDB<T>::PositionDB(const std::string& filename,
                          const std::string& type)
    : m_db(filename, type),
      m_stats()
{
}

template<class T>
PositionDB<T>::~PositionDB()
{
}

template<class T>
bool PositionDB<T>::Exists(const HexState& state) const
{
    return m_db.Exists(GetHash(state));
}

template<class T>
bool PositionDB<T>::Get(const HexState& state, T& data) const
{
    m_stats.m_gets++;
    hash_t hash = GetHash(state);
    if (!m_db.Get(hash, data))
        return false;
    m_stats.m_hits++;
    if (NeedToRotate(state, hash))
    {
        m_stats.m_rotations++;
        data.Rotate(state.Position().Const());
    }
    return true;
}

template<class T>
bool PositionDB<T>::Put(const HexState& state, const T& data)
{
    m_stats.m_puts++;
    hash_t hash = GetHash(state);
    T myData(data);
    if (NeedToRotate(state, hash))
    {
        m_stats.m_rotations++;
        myData.Rotate(state.Position().Const());
    }
    return m_db.Put(hash, myData);
}

template<class T>
void PositionDB<T>::Flush()
{
    m_db.Flush();
}

template<class T>
typename PositionDB<T>::Statistics 
PositionDB<T>::GetStatistics() const
{
    return m_stats;
}

template<class T>
PositionDB<T>::Statistics::Statistics()
    : m_gets(0), 
      m_hits(0), 
      m_puts(0), 
      m_rotations(0) 
{ 
}

template<class T>
std::string PositionDB<T>::Statistics::Write() const
{
    std::ostringstream os;
    os << "PositionDB statistics\n"
       << "Reads      " << m_gets << '\n'
       << "Hits       " << m_hits << '\n'
       << "Writes     " << m_puts << '\n'
       << "Rotations  " << m_rotations;
    return os.str();
}

//----------------------------------------------------------------------------

/** Set of positions; handles rotations. */
class PositionSet
{
public:
    PositionSet();

    ~PositionSet();

    void Insert(const HexState& state);

    bool Exists(const HexState& state) const;

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

inline void PositionSet::Insert(const HexState& state)
{
    m_set.insert(GetHash(state));
}

inline bool PositionSet::Exists(const HexState& state) const
{
    return m_set.count(GetHash(state)) > 0;
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

    bool Exists(const HexState& state) const;

    T& operator[](const HexState& state);

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
bool PositionMap<T>::Exists(const HexState& state) const
{
    return m_map.find(GetHash(state)) != m_map.end();
}

template<class T>
inline T& PositionMap<T>::operator[](const HexState& state)
{
    return m_map[GetHash(state)];
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // POSITIONDB_HPP

