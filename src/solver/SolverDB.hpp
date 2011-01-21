//----------------------------------------------------------------------------
/** @file SolverDB.hpp
 */
//----------------------------------------------------------------------------

#ifndef SolverDB_HPP
#define SolverDB_HPP

#include "Hex.hpp"
#include "BenzeneSolver.hpp"
#include "HexState.hpp"
#include <boost/concept_check.hpp>

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

struct SolverDBParameters
{
    /** States with fewer stones go into the database, otherwise the
        hashtable.*/
    std::size_t m_maxStones;
    
    /** States with fewer stones get transpositions/flipped states. */
    std::size_t m_transStones;

    bool m_useProofTranspositions;

    bool m_useFlippedStates;

    SolverDBParameters();
};

inline SolverDBParameters::SolverDBParameters()
    : m_maxStones(10),
      m_transStones(0),
      m_useProofTranspositions(false),
      m_useFlippedStates(false)
{
}

//----------------------------------------------------------------------------

/** Combines a hash table and a position database. */
template<class HASH, class DB, class DATA>
class SolverDB
{
public:
    SolverDB(boost::scoped_ptr<HASH>& hashTable, 
             boost::scoped_ptr<DB>& database, 
             const SolverDBParameters& param);

    ~SolverDB();

    bool Get(const HexState& state, DATA& data);

    void Put(const HexState& state, const DATA& data);

    HASH* HashTable();

    DB* Database();

    const SolverDBParameters& Parameters() const;

    SolverDBParameters& Parameters();

    void SetParameters(const SolverDBParameters& param);
    
private:
    boost::scoped_ptr<HASH>& m_hashTable;
    
    boost::scoped_ptr<DB>& m_database;

    SolverDBParameters m_param;

    bool UseDatabase() const;

    bool UseHashTable() const;

};

//----------------------------------------------------------------------------

template<class HASH, class DB, class DATA>
SolverDB<HASH, DB, DATA>::SolverDB(boost::scoped_ptr<HASH>& hashTable, 
                                   boost::scoped_ptr<DB>& database, 
                                   const SolverDBParameters& param)
    : m_hashTable(hashTable),
      m_database(database),
      m_param(param)
{
}

template<class HASH, class DB, class DATA>
SolverDB<HASH, DB, DATA>::~SolverDB()
{

}

template<class HASH, class DB, class DATA>
HASH* SolverDB<HASH, DB, DATA>::HashTable()
{
    return m_hashTable.get();
}

template<class HASH, class DB, class DATA>
DB* SolverDB<HASH, DB, DATA>::Database()
{
    return m_database.get();
}

template<class HASH, class DB, class DATA>
const SolverDBParameters& SolverDB<HASH, DB, DATA>::Parameters() const
{
    return m_param;
}

template<class HASH, class DB, class DATA>
SolverDBParameters& SolverDB<HASH, DB, DATA>::Parameters()
{
    return m_param;
}

template<class HASH, class DB, class DATA>
void SolverDB<HASH, DB, DATA>::SetParameters(const SolverDBParameters& p)
{
    return m_param = p;
}

template<class HASH, class DB, class DATA>
bool SolverDB<HASH, DB, DATA>::UseDatabase() const
{
    return m_database.get() != 0;
}

template<class HASH, class DB, class DATA>
bool SolverDB<HASH, DB, DATA>::UseHashTable() const
{
    return m_hashTable.get() != 0;
}

template<class HASH, class DB, class DATA>
bool SolverDB<HASH, DB, DATA>::Get(const HexState& state, DATA& data)
{
    if (UseDatabase() && state.Position().NumStones() <= m_param.m_maxStones)
        return m_database->Get(state, data);
    if (UseHashTable())
        return m_hashTable->Lookup(state.Hash(), &data);
    return false;
}

template<class HASH, class DB, class DATA>
void SolverDB<HASH, DB, DATA>::Put(const HexState& state, const DATA& data)
{
    if (UseDatabase() && state.Position().NumStones() <= m_param.m_maxStones)
        m_database->Put(state, data);
    else if (UseHashTable())
        m_hashTable->Store(state.Hash(), data);
}

//----------------------------------------------------------------------------

namespace SolverDBUtil
{
    /** Follows best move in DATA to create a variation. Variation ends
        when it hits a best move of INVALID_POINT or the move is not
        found in the database. */
    template<class HASH, class DB, class DATA>
    void GetVariation(const HexState& state, 
                      SolverDB<HASH, DB, DATA>& positions, PointSequence& pv);
    
}

template<class HASH, class DB, class DATA>
void SolverDBUtil::GetVariation(const HexState& origState,
                                SolverDB<HASH, DB, DATA>& positions, 
                                PointSequence& pv)
{
    boost::function_requires< HasBestMoveConcept<DATA> >();
    HexState state(origState);
    while (true) 
    {
        DATA data;
        if (!positions.Get(state, data))
            break;
        if (data.m_bestMove == INVALID_POINT)
            break;
        pv.push_back(data.m_bestMove);
        state.PlayMove(data.m_bestMove);
    }
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // SOLVERDB_HPP
