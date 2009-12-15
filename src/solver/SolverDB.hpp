//----------------------------------------------------------------------------
/** @file
 */
//----------------------------------------------------------------------------

#ifndef SOLVERDB_H
#define SOLVERDB_H

#include "Hex.hpp"
#include "HexException.hpp"
#include "SolvedState.hpp"
#include "StoneBoard.hpp"
#include "HashDB.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Database of solved positions. */
class SolverDB
{
public:

    //------------------------------------------------------------------------
    
    /** Settings for this DB of solved positions. */
    struct Settings
    {
        /** Width of the board. */
        int width;

        /** Height of the board. */
        int height;

        /** Store transpositions for all states with fewer stones than
            this. */
        int trans_stones; 

        /** Maximum number of stones allowed for states in this DB. */
        int maxstones; 

        Settings()
            : width(0), height(0), trans_stones(0), maxstones(0)
        { }
        
        Settings(int w, int h, int t, int s)
            : width(w), height(h), trans_stones(t), maxstones(s)
        { }

        bool operator==(const Settings& o) const
        {
            return (width == o.width && height == o.height && 
                    trans_stones == o.trans_stones && 
                    maxstones == o.maxstones);
        }

        bool operator!=(const Settings& o) const
        {
            return !operator==(o);
        }

        std::string toString() const
        {
            std::ostringstream os;
            os << "["
               << "width=" << width << " "
               << "height=" << height << " "
               << "trans_stones=" << trans_stones << " "
               << "maxstones=" << maxstones 
               << "]";
            return os.str();
        }
    };
    
    //------------------------------------------------------------------------

    /** Statistics. */
    struct Statistics
    {
        unsigned gets;             /** Number of successful get calls. */
        unsigned saved;            /** Number of states below gets.    */
        unsigned puts;             /** Number of successful put calls. */
        unsigned writes;           /** Number of chunks written.       */

        Statistics() 
            : gets(0), saved(0), puts(0), writes(0)
        { }
    };

    //------------------------------------------------------------------------

    /** Opens (or creates if it does not already exist) a SolverDB for
        the given board dimensions, max number of stones, and number
        of stones for transposition states. Throws an exception if an
        error occur (eg: db already exists but with different
        settings). */
    SolverDB(int width, int height, int maxstones, int transtones,
              const std::string& filename) 
        throw (HexException);


    /** Opens an existing database for the given boardsize. */
    SolverDB(int width, int height, const std::string& filename)
        throw(HexException);

    /** Destructor. */
    ~SolverDB();

    //------------------------------------------------------------------------

    /** Returns the settings for this DB. */
    Settings settings() const;

    /** Return the current statistics. */
    Statistics stats() const;

    /** Helper for settings().maxstones. */
    int maxstones() const;

    //------------------------------------------------------------------------

    /** Gets the DB data for the given state. Returns false if state
        is not in DB. If return is true, data is stored in state. 
        Checks for rotations as well. */
    bool get(const StoneBoard& brd, SolvedState& state);

    /** Returns true if state (or its rotation) exists in db. */
    bool check(const StoneBoard& brd);

    /** Stores state in the db under the given board position.
        Returns number of states written (0 or 1). */
    int write(const StoneBoard& brd, const SolvedState& state);

    /** Stores state in db for all transpositions of the given proof
        if state has fewer than m_settings.trans_stones
        stones. Returns number of tranpositions written. */
    int put(const StoneBoard& brd, const SolvedState& state,
            const bitset_t& proof);

private:
    Settings m_settings;

    HashDB<SolvedState> m_db;

    mutable Statistics m_stats;
};

inline int SolverDB::maxstones() const
{
    return m_settings.maxstones;
}

inline SolverDB::Statistics SolverDB::stats() const
{
    return m_stats;
}

inline SolverDB::Settings SolverDB::settings() const
{
    return m_settings;
}

//----------------------------------------------------------------------------

/** SolverDB utilities. */
namespace SolverDBUtil 
{
    /** Computes and stores in db the transpostions of this proof on
        the given boardstate. Returns number of db entries
        successfully added or updated. */
    int StoreTranspositions(SolverDB& db, const StoneBoard& brd, 
                            const SolvedState& state, const bitset_t& proof);

    /** Computes and stores in db the flipped transpostions of this
        proof on the given boardstate. Returns number of db entries
        successfully added or updated. */
    int StoreFlippedStates(SolverDB& db, const StoneBoard& brd, 
                           const SolvedState& state, const bitset_t& proof);
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // SOLVERDB_H
