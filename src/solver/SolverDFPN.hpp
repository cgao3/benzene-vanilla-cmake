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

    DfpnData();

    DfpnData( const DfpnBounds& bounds, const bitset_t& children, 
             HexPoint bestMove);

    ~DfpnData();
    
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
                          const bitset_t& children,  HexPoint bestMove)
    : m_bounds(bounds),
      m_children(children),
      m_bestMove(bestMove),
      m_initialized(true)
{ 
}

inline DfpnData::~DfpnData()
{
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

//----------------------------------------------------------------------------

typedef TransTable<DfpnData> DfpnHashTable;

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
                         const std::vector<DfpnBounds>& bounds);

        void PlayMove(HexColor color, HexPoint move);

        void UndoMove();

        void UpdateCurrentBounds(const DfpnBounds& bounds);

        void Write();

        void WriteForced();

    private:
        
        std::vector<HexPoint> m_children;

        std::vector<DfpnBounds> m_bounds;

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

    void MID(const DfpnBounds& n, int depth);

    void SelectChild(int& bestMove, std::size_t& delta2, 
                     const std::vector<DfpnBounds>& childrenDfpnBounds) const;

    void UpdateBounds(DfpnBounds& bounds, 
                      const std::vector<DfpnBounds>& childBounds) const;

    bool CheckAbort();

    void CheckBounds(const DfpnBounds& bounds) const;

    void LookupBounds(DfpnBounds& bound, HexColor colorToMove, HexPoint cell);

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
