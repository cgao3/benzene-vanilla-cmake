//----------------------------------------------------------------------------
/** @file StateDB.hpp */
//----------------------------------------------------------------------------

#ifndef POSITIONDB_HPP
#define POSITIONDB_HPP

#include <boost/concept_check.hpp>
#include "HashDB.hpp"
#include "HexState.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

namespace {

SgHashCode GetHash(const HexState& state)
{
    SgHashCode hash1 = state.Hash();
    HexState rotatedState(state);
    rotatedState.Position().RotateBoard();
    SgHashCode hash2 = rotatedState.Hash();
    return (hash1 < hash2) ? hash1 : hash2;
}

/** Data must be stored for the state of the minimum hash. */
inline bool NeedToRotate(const HexState& state, SgHashCode minHash)
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
struct StateDBStateConcept
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
class StateDB
{
    BOOST_CLASS_REQUIRE(T, benzene, StateDBStateConcept);

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
    StateDB(const std::string& filename, const std::string& type);

    /** Closes database. */    
    ~StateDB();

    /** Returns true if position exists in database. */
    bool Exists(const HexState& pos) const;

    /** Returns true if get is successful. */
    bool Get(const HexState& pos, T& data) const;

    /** Returns true if put is successful. */
    bool Put(const HexState& brd, const T& data);

    void Flush();

    Statistics GetStatistics() const;

    std::string BDBStatistics();

private:
    HashDB<T> m_db;

    mutable Statistics m_stats;
};

template<class T>
StateDB<T>::StateDB(const std::string& filename, const std::string& type)
    : m_db(filename, type),
      m_stats()
{
}

template<class T>
StateDB<T>::~StateDB()
{
}

template<class T>
bool StateDB<T>::Exists(const HexState& state) const
{
    return m_db.Exists(GetHash(state));
}

template<class T>
bool StateDB<T>::Get(const HexState& state, T& data) const
{
    m_stats.m_gets++;
    SgHashCode hash = GetHash(state);
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
bool StateDB<T>::Put(const HexState& state, const T& data)
{
    m_stats.m_puts++;
    SgHashCode hash = GetHash(state);
    T myData(data);
    if (NeedToRotate(state, hash))
    {
        m_stats.m_rotations++;
        myData.Rotate(state.Position().Const());
    }
    return m_db.Put(hash, myData);
}

template<class T>
void StateDB<T>::Flush()
{
    m_db.Flush();
}

template<class T>
typename StateDB<T>::Statistics 
StateDB<T>::GetStatistics() const
{
    return m_stats;
}

template<class T>
StateDB<T>::Statistics::Statistics()
    : m_gets(0), 
      m_hits(0), 
      m_puts(0), 
      m_rotations(0) 
{ 
}

template<class T>
std::string StateDB<T>::Statistics::Write() const
{
    std::ostringstream os;
    os << "StateDB statistics\n"
       << "Reads      " << m_gets << '\n'
       << "Hits       " << m_hits << '\n'
       << "Writes     " << m_puts << '\n'
       << "Rotations  " << m_rotations;
    return os.str();
}

template<class T>
std::string StateDB<T>::BDBStatistics()
{
    return m_db.BDBStatistics();
}

//----------------------------------------------------------------------------

/** Set of positions; handles rotations. */
class StateSet
{
public:
    StateSet();

    ~StateSet();

    void Insert(const HexState& state);

    bool Exists(const HexState& state) const;

    std::size_t Size() const;

private:
    std::set<SgHashCode> m_set;
};

inline StateSet::StateSet()
{
}

inline StateSet::~StateSet()
{
}

inline void StateSet::Insert(const HexState& state)
{
    m_set.insert(GetHash(state));
}

inline bool StateSet::Exists(const HexState& state) const
{
    return m_set.count(GetHash(state)) > 0;
}

inline std::size_t StateSet::Size() const
{
    return m_set.size();
}

//----------------------------------------------------------------------------

/** Map of positions; handles rotations. */
template<class T>
class StateMap
{
public:
    StateMap();

    ~StateMap();

    bool Exists(const HexState& state) const;

    T& operator[](const HexState& state);

    std::size_t Size() const;

private:
    std::map<SgHashCode, T> m_map;
};

template<class T>
inline StateMap<T>::StateMap()
{
}

template<class T>
inline StateMap<T>::~StateMap()
{
}

template<class T>
bool StateMap<T>::Exists(const HexState& state) const
{
    return m_map.find(GetHash(state)) != m_map.end();
}

template<class T>
inline T& StateMap<T>::operator[](const HexState& state)
{
    return m_map[GetHash(state)];
}

template<class T>
inline std::size_t StateMap<T>::Size() const
{
    return m_map.size();
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // POSITIONDB_HPP

