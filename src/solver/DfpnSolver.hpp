//----------------------------------------------------------------------------
/** @file DfpnSolver.hpp
 */
//----------------------------------------------------------------------------

#ifndef SOLVERDFPN_HPP
#define SOLVERDFPN_HPP

#include "SgSystem.h"
#include "SgStatistics.h"
#include "SgTimer.h"

#include "Hex.hpp"
#include "HexBoard.hpp"
#include "TransTable.hpp"

#include <limits>
#include <boost/scoped_ptr.hpp>

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** @defgroup dfpn Depth-First Proof Number Search
    Hex Solver Using DFPN
    
    Based on [reference Martin & Kishi's paper]. 
   
    @todo
    - Make game independent.
    - Heuristic leaf scores.
    - Add database read/write.
    - Re-use move ordering between siblings (worked in go).
*/

//----------------------------------------------------------------------------

/** Statistics tracker used in dfpn search.
    @ingroup dfpn
*/
typedef SgStatisticsExt<float, std::size_t> DfpnStatistics;

//----------------------------------------------------------------------------

/** Maximum bound. */
static const std::size_t INFTY = 2000000000;

/** Bounds used in Dfpn search. 
    @ingroup dfpn
*/
struct DfpnBounds
{
    std::size_t phi;

    std::size_t delta;

    DfpnBounds();

    DfpnBounds(std::size_t p, std::size_t d);

    /** Returns true if bounds are winning, ie, phi is 0. */
    bool IsWinning() const;

    /** Returns true if bounds are losing, ie delta is 0. */
    bool IsLosing() const;
    
    /** Returns true if IsWinning() or IsLosing() is true. */
    bool IsSolved() const;

    /** Print bounds in human readable format. */
    std::string Print() const;

    /** Sets the bounds to (0, INFTY). */
    static void SetToWinning(DfpnBounds& bounds);

    /** Sets the bounds to (INFTY, 0). */
    static void SetToLosing(DfpnBounds& bounds);
};

inline DfpnBounds::DfpnBounds()
    : phi(INFTY), 
      delta(INFTY)
{
}

inline DfpnBounds::DfpnBounds(std::size_t p, std::size_t d)
    : phi(p), 
      delta(d)
{
}

inline std::string DfpnBounds::Print() const
{
    std::ostringstream os;
    os << "[" << phi << ", " << delta << "]";
    return os.str();
}

inline bool DfpnBounds::IsWinning() const
{
    return phi == 0;
}

inline bool DfpnBounds::IsLosing() const
{
    return delta == 0;
}

inline bool DfpnBounds::IsSolved() const
{
    return IsWinning() || IsLosing();
}

inline void DfpnBounds::SetToWinning(DfpnBounds& bounds)
{
    bounds.phi = 0;
    bounds.delta = INFTY;
}

inline void DfpnBounds::SetToLosing(DfpnBounds& bounds)
{
    bounds.phi = INFTY;
    bounds.delta = 0;
}

/** Extends global output operator for DfpnBounds. */
inline std::ostream& operator<<(std::ostream& os, const DfpnBounds& bounds)
{
    os << bounds.Print();
    return os;
}

//----------------------------------------------------------------------------

/** Children of a dfpn state. 
    @ingroup dfpn
*/
class DfpnChildren
{
public:
    DfpnChildren();

    void SetChildren(const std::vector<HexPoint>& children);
    
    std::size_t Size() const;

    HexPoint FirstMove(int index) const;

    const std::vector<HexPoint>& Moves(int index) const;

    void PlayMove(int index, StoneBoard& brd) const;

    void UndoMove(int index, StoneBoard& brd) const;

private:
    std::vector<std::vector<HexPoint> > m_children;

};

inline std::size_t DfpnChildren::Size() const
{
    return m_children.size();
}

inline HexPoint DfpnChildren::FirstMove(int index) const
{
    return m_children[index][0];
}

inline const std::vector<HexPoint>& DfpnChildren::Moves(int index) const
{
    return m_children[index];
}

//----------------------------------------------------------------------------

/** State in hashtable.
    @ingroup dfpn
 */
class DfpnData
{
public:

    DfpnBounds m_bounds;

    DfpnChildren m_children;

    HexPoint m_bestMove;
    
    size_t m_work;

    hash_t m_parentHash;

    HexPoint m_moveParentPlayed;

    DfpnData();

    DfpnData(const DfpnBounds& bounds, const DfpnChildren& children, 
             HexPoint bestMove, size_t work, hash_t parentHash,
             HexPoint moveParentPlayed);

    ~DfpnData();

    std::string Print() const; 
    
    bool Initialized() const;
    
    bool ReplaceWith(const DfpnData& data) const;

private:

    bool m_initialized;
};


inline DfpnData::DfpnData()
    : m_initialized(false)
{ 
}

inline DfpnData::DfpnData(const DfpnBounds& bounds, 
                          const DfpnChildren& children, 
                          HexPoint bestMove, size_t work, hash_t parentHash,
                          HexPoint moveParentPlayed)
    : m_bounds(bounds),
      m_children(children),
      m_bestMove(bestMove),
      m_work(work),
      m_parentHash(parentHash),
      m_moveParentPlayed(moveParentPlayed),
      m_initialized(true)
{ 
}

inline DfpnData::~DfpnData()
{
}

inline std::string DfpnData::Print() const
{
    std::ostringstream os;
    os << '[' 
       << "bounds=" << m_bounds << ' '
       << "children=" << m_children.Size() << ' '
       << "bestmove=" << m_bestMove << ' '
       << "work=" << m_work << ' '
       << "parent=" << HashUtil::toString(m_parentHash) << ' '
       << "parentmove=" << m_moveParentPlayed
       << ']';
    return os.str();
}

inline bool DfpnData::ReplaceWith(const DfpnData& data) const
{
    SG_UNUSED(data);
    return true;
}

inline bool DfpnData::Initialized() const
{
    return m_initialized;
}

/** Extends global output operator for DfpnData. */
inline std::ostream& operator<<(std::ostream& os, const DfpnData& data)
{
    os << data.Print();
    return os;
}

//----------------------------------------------------------------------------
/** Hashtable used in dfpn search.  
    @ingroup dfpn
*/
typedef TransTable<DfpnData> DfpnHashTable;

//----------------------------------------------------------------------------

/** Stores enough information on an encountered transposition that the
    bounds can be fixed in the state it starts from. 
    @ingroup dfpn
*/
struct DfpnTransposition
{
    static const size_t MAX_LENGTH = 8;

    /** Hash of dependant state; used as an id for this
        transposition. */
    hash_t m_hash;
    
    std::vector<HexPoint> m_rightMove;

    std::vector<hash_t> m_rightHash;

    std::vector<HexPoint> m_leftMove;

    std::vector<hash_t> m_leftHash;
    
    DfpnTransposition();

    DfpnTransposition(hash_t hash);

    bool IsMinPath(DfpnHashTable& hashTable, 
                   const std::vector<HexPoint>& move,
                   const std::vector<hash_t>& hash) const;

    bool ModifyBounds(hash_t currentHash, DfpnBounds& bounds, 
                      DfpnHashTable& hashTable) const;
};

/** Stores a number of transpositions. */
struct DfpnTranspositions
{
    static const size_t NUM_SLOTS = 8;

    std::vector<DfpnTransposition> m_slot;

    void Add(hash_t hash, size_t length, 
             HexPoint* rightMove, hash_t* rightHashes, 
             HexPoint* leftMove, hash_t* leftHashes);

    size_t ModifyBounds(hash_t currentHash, DfpnBounds& bounds, 
                        DfpnHashTable& hashTable, 
                        DfpnStatistics& slotStats) const;
};

//----------------------------------------------------------------------------

/** History of moves played from root state to current state. 
    @ingroup dfpn
*/
class DfpnHistory
{
public:
    DfpnHistory();

    /** Adds a new state to the history. */
    void Push(HexPoint m_move, hash_t hash);

    /** Removes last stated added from history. */
    void Pop();

    /** Returns number of moves played so far. */
    int Depth() const;

    /** Hash of last state. */
    hash_t LastHash() const;

    /** Move played from parent state to bring us to this state. */
    HexPoint LastMove() const;

    DfpnTranspositions& Transpositions();

    void NotifyCommonAncestor(DfpnHashTable& hashTable, DfpnData data,
                              hash_t hash, DfpnStatistics& stats);

private:

    /** Move played from state. */
    std::vector<HexPoint> m_move;

    /** Hash of state. */
    std::vector<hash_t> m_hash;

    /** Stores up to NUM_SLOTS tranposed decendents. */
    std::vector<DfpnTranspositions> m_transposition;
};

inline DfpnHistory::DfpnHistory()
{
    m_move.push_back(INVALID_POINT);
    m_hash.push_back(0);
    m_transposition.push_back(DfpnTranspositions());
}

inline void DfpnHistory::Push(HexPoint move, hash_t hash)
{
    m_move.push_back(move);
    m_hash.push_back(hash);
    m_transposition.push_back(DfpnTranspositions());
}

inline void DfpnHistory::Pop()
{
    m_move.pop_back();
    m_hash.pop_back();
    m_transposition.pop_back();
}

inline int DfpnHistory::Depth() const
{
    return m_move.size() - 1;
}

inline hash_t DfpnHistory::LastHash() const
{
    return m_hash.back();
}

inline HexPoint DfpnHistory::LastMove() const
{
    return m_move.back();
}

inline DfpnTranspositions& DfpnHistory::Transpositions()
{
    return m_transposition.back();
}

//----------------------------------------------------------------------------

/** Hex solver using DFPN search. 
    @ingroup dfpn
*/
class DfpnSolver 
{
public:

    DfpnSolver();

    ~DfpnSolver();

    /** Solves the given state using the given hashtable. */
    HexColor StartSearch(HexBoard& brd, DfpnHashTable& hashtable);
    
    /** Dumps output about root state what gui can display. */
    bool UseGuiFx() const;

    /** See UseGuiFx() */
    void SetUseGuiFx(bool enable);

    /** Maximum time search is allowed to run before aborting. 
        Set to 0 for no timelimit. */
    double Timelimit() const;

    /** See Timelimit() */
    void SetTimelimit(double timelimit);

    /** Fix bounds due to transpositions double counting descendant
        states. */
    bool UseBoundsCorrection() const;
    
    /** See UseBoundsCorrection() */
    void SetUseBoundsCorrection(bool flag);

    /** Prune Unique Probes
        @todo PHIL DOCUMENT THIS! */
    bool UseUniqueProbes() const;

    /** See UseUniqueProbes() */
    void SetUseUniqueProbes(bool flag);

private:

    /** Handles guifx output. */
    class GuiFx
    {
    public:

        GuiFx();

        void SetChildren(const DfpnChildren& children,
                         const std::vector<DfpnData>& bounds);

        void PlayMove(HexColor color, int index);

        void UndoMove();

        void UpdateCurrentBounds(const DfpnBounds& bounds);

        void Write();

        void WriteForced();

    private:
        
        DfpnChildren m_children;

        std::vector<DfpnData> m_data;

        HexColor m_color;

        int m_index;

        double m_timeOfLastWrite;

        int m_indexAtLastWrite;

        double m_delay;

        void DoWrite();
    };

    boost::scoped_ptr<StoneBoard> m_brd;

    HexBoard* m_workBoard;

    DfpnHashTable* m_hashTable;

    SgTimer m_timer;

    /** See UseGuiFx() */
    bool m_useGuiFx;

    /** See BoundsCorrection() */
    bool m_useBoundsCorrection;

    /** See UniqueProbes() */
    bool m_useUniqueProbes;

    /** See TimeLimit() */
    double m_timelimit;

    /** Number of calls to CheckAbort() before we check the timer.
        This is to avoid expensive calls to SgTime::Get(). Try to scale
        this so that it is checked twice a second. */
    size_t m_checkTimerAbortCalls;

    bool m_aborted;

    GuiFx m_guiFx;

    size_t m_numTerminal;

    size_t m_numTranspositions;
    
    DfpnStatistics m_transStats;

    DfpnStatistics m_slotStats;

    size_t m_numBoundsCorrections;

    size_t m_numMIDcalls;

    size_t m_numUniqueProbes;

    size_t m_numProbeChecks;

    size_t MID(const DfpnBounds& n, DfpnHistory& history);

    void SelectChild(int& bestMove, std::size_t& delta2, 
                     const std::vector<DfpnData>& childrenDfpnBounds) const;

    void UpdateBounds(hash_t parentHash, DfpnBounds& bounds, 
                      const std::vector<DfpnData>& childBounds) const;

    bool CheckAbort();

    void CheckBounds(const DfpnBounds& bounds) const;

    void LookupData(DfpnData& data, const DfpnChildren& children, 
                    int childIndex, size_t delta);

    void TTStore(hash_t hash, const DfpnData& data);

    void GetVariation(const StoneBoard& state, 
                      std::vector<HexPoint>& pv) const;

    std::string PrintVariation(const std::vector<HexPoint>& pv) const;

    void DumpGuiFx(const std::vector<HexPoint>& children,
                   const std::vector<DfpnBounds>& childBounds) const;
};

inline bool DfpnSolver::UseGuiFx() const
{
    return m_useGuiFx;
}

inline void DfpnSolver::SetUseGuiFx(bool enable)
{
    m_useGuiFx = enable;
}

inline double DfpnSolver::Timelimit() const
{
    return m_timelimit;
}

inline void DfpnSolver::SetTimelimit(double timelimit)
{
    m_timelimit = timelimit;
}

inline bool DfpnSolver::UseBoundsCorrection() const
{
    return m_useBoundsCorrection;
}

inline void DfpnSolver::SetUseBoundsCorrection(bool flag)
{
    m_useBoundsCorrection = flag;
}

inline bool DfpnSolver::UseUniqueProbes() const
{
    return m_useUniqueProbes;
}

inline void DfpnSolver::SetUseUniqueProbes(bool flag)
{
    m_useUniqueProbes = flag;
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // SOLVERDFPN_HPP
