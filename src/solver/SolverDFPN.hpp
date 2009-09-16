//----------------------------------------------------------------------------
/** @file SolverDFPN.hpp
 */
//----------------------------------------------------------------------------

#ifndef SOLVERDFPN_HPP
#define SOLVERDFPN_HPP

#include "SgSystem.h"
#include "SgTimer.h"

#include "Hex.hpp"
#include "HexBoard.hpp"
#include "TransTable.hpp"

#include <limits>
#include <boost/scoped_ptr.hpp>

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Maximum bound. */
static const std::size_t INFTY = 2000000000;

/** Bounds used in Dfpn search. */
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

class DfpnData
{
public:

    DfpnBounds m_bounds;

    bitset_t m_children;

    HexPoint m_bestMove;
    
    size_t m_work;

    hash_t m_parentHash;

    DfpnData();

    DfpnData( const DfpnBounds& bounds, const bitset_t& children, 
              HexPoint bestMove, size_t work, hash_t parentHash);

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

inline DfpnData::DfpnData(const DfpnBounds& bounds, const bitset_t& children, 
                          HexPoint bestMove, size_t work, hash_t parentHash)
    : m_bounds(bounds),
      m_children(children),
      m_bestMove(bestMove),
      m_work(work),
      m_parentHash(parentHash),
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
       << "children=" << m_children.count() << ' '
       << "bestmove=" << m_bestMove << ' '
       << "work=" << m_work << ' '
       << "parent=" << HashUtil::toString(m_parentHash) 
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

typedef TransTable<DfpnData> DfpnHashTable;

//----------------------------------------------------------------------------

/** History of moves played from root state to current state. */
class DfpnHistory
{
public:
    DfpnHistory();

    /** Adds a new state to the history. */
    void PushBack(HexPoint m_move, hash_t hash);

    /** Removes last stated added from history. */
    void Pop();

    /** Returns number of moves played so far. */
    int Depth() const;

    /** Hash of last state. */
    hash_t LastHash() const;

private:

    /** Move played from state. */
    std::vector<HexPoint> m_move;

    /** Hash of state. */
    std::vector<hash_t> m_hash;
};

inline DfpnHistory::DfpnHistory()
{
    m_move.push_back(INVALID_POINT);
    m_hash.push_back(0);
}

inline void DfpnHistory::PushBack(HexPoint move, hash_t hash)
{
    m_move.push_back(move);
    m_hash.push_back(hash);
}

inline void DfpnHistory::Pop()
{
    m_move.pop_back();
    m_hash.pop_back();
}

inline int DfpnHistory::Depth() const
{
    return m_move.size() - 1;
}

inline hash_t DfpnHistory::LastHash() const
{
    return m_hash.back();
}

//----------------------------------------------------------------------------

/** Hex solver using DFPN search. */
class SolverDFPN 
{
public:

    SolverDFPN();

    ~SolverDFPN();

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

private:

    /** Handles guifx output. */
    class GuiFx
    {
    public:

        GuiFx();

        void SetChildren(const std::vector<HexPoint>& children,
                         const std::vector<DfpnData>& bounds);

        void PlayMove(HexColor color, HexPoint move);

        void UndoMove();

        void UpdateCurrentBounds(const DfpnBounds& bounds);

        void Write();

        void WriteForced();

    private:
        
        std::vector<HexPoint> m_children;

        std::vector<DfpnData> m_data;

        HexColor m_color;

        HexPoint m_move;

        double m_timeOfLastWrite;

        HexPoint m_moveAtLastWrite;

        double m_delay;

        void DoWrite();
    };

    boost::scoped_ptr<StoneBoard> m_brd;

    HexBoard* m_workBoard;

    DfpnHashTable* m_hashTable;

    SgTimer m_timer;

    /** See UseGuiFx() */
    bool m_useGuiFx;

    /** See TimeLimit() */
    double m_timelimit;

    /** Number of calls to CheckAbort() before we check the timer.
        This is to avoid expensive calls to SgTime::Get(). Try to scale
        this so that it is checked twice a second. */
    size_t m_checkTimerAbortCalls;

    bool m_aborted;

    GuiFx m_guiFx;

    size_t m_numTerminal;

    size_t m_numMIDcalls;

    size_t MID(const DfpnBounds& n, DfpnHistory& history);

    void SelectChild(int& bestMove, std::size_t& delta2, 
                     const std::vector<DfpnData>& childrenDfpnBounds) const;

    void UpdateBounds(hash_t parentHash, DfpnBounds& bounds, 
                      const std::vector<DfpnData>& childBounds) const;

    bool CheckAbort();

    void CheckBounds(const DfpnBounds& bounds) const;

    void LookupData(DfpnData& data, HexColor colorToMove, HexPoint cell);

    void TTStore(hash_t hash, const DfpnData& data);

    void GetVariation(const StoneBoard& state, 
                      std::vector<HexPoint>& pv) const;

    std::string PrintVariation(const std::vector<HexPoint>& pv) const;

    void DumpGuiFx(const std::vector<HexPoint>& children,
                   const std::vector<DfpnBounds>& childBounds) const;
};

inline bool SolverDFPN::UseGuiFx() const
{
    return m_useGuiFx;
}

inline void SolverDFPN::SetUseGuiFx(bool enable)
{
    m_useGuiFx = enable;
}

inline double SolverDFPN::Timelimit() const
{
    return m_timelimit;
}

inline void SolverDFPN::SetTimelimit(double timelimit)
{
    m_timelimit = timelimit;
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // SOLVERDFPN_HPP
