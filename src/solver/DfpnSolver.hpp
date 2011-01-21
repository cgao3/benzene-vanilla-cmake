//----------------------------------------------------------------------------
/** @file DfpnSolver.hpp */
//----------------------------------------------------------------------------

#ifndef SOLVERDFPN_HPP
#define SOLVERDFPN_HPP

#include "SgSystem.h"
#include "SgHashTable.h"
#include "SgStatistics.h"
#include "SgTimer.h"

#include "Hex.hpp"
#include "HexBoard.hpp"
#include "HexState.hpp"
#include "Game.hpp"
#include "StateDB.hpp"
#include "SolverDB.hpp"

#include <limits>
#include <boost/scoped_ptr.hpp>

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** @defgroup dfpn Depth-First Proof Number Search
    Hex Solver Using DFPN
    
    Based on [reference Martin & Kishi's paper]. 
*/

//----------------------------------------------------------------------------

/** Statistics tracker used in dfpn search.
    @ingroup dfpn
*/
typedef SgStatisticsExt<float, std::size_t> DfpnStatistics;

//----------------------------------------------------------------------------

/** Type used for bounds. 
    @ingroup dfpn
*/
typedef unsigned DfpnBoundType;

/** Bounds used in Dfpn search. 
    @ingroup dfpn
*/
struct DfpnBounds
{
    /** Denotes a proven state. */
    static const DfpnBoundType INFTY = 2000000000;

    /** Maximum amount of work. Must be less than INFTY. */ 
    static const DfpnBoundType MAX_WORK = INFTY / 2;

    /** Proof number.
        Estimated amount of work to prove this state winning. */
    DfpnBoundType phi;

    /** Disproof number.
        Estimated amount of work to prove this state losing. */
    DfpnBoundType delta;

    DfpnBounds();

    DfpnBounds(DfpnBoundType p, DfpnBoundType d);


    /** Returns true if phi is greater than other's phi and delta is
        greater than other's delta. */
    bool GreaterThan(const DfpnBounds& other) const;

    /** Returns true if bounds are winning (phi is 0). */
    bool IsWinning() const;

    /** Returns true if bounds are losing (delta is 0). */
    bool IsLosing() const;
    
    /** Returns true if IsWinning() or IsLosing() is true. */
    bool IsSolved() const;

    void CheckConsistency() const;

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

inline DfpnBounds::DfpnBounds(DfpnBoundType p, DfpnBoundType d)
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

inline bool DfpnBounds::GreaterThan(const DfpnBounds& other) const
{
    return (phi > other.phi) && (delta > other.delta);
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

    HexPoint FirstMove(std::size_t index) const;

    void PlayMove(std::size_t index, HexState& state) const;

    void UndoMove(std::size_t index, HexState& state) const;

private:
    friend class DfpnData;
    friend class DfpnSolver;

    std::vector<HexPoint> m_children;
};

inline std::size_t DfpnChildren::Size() const
{
    return m_children.size();
}

inline HexPoint DfpnChildren::FirstMove(std::size_t index) const
{
    return m_children[index];
}

//----------------------------------------------------------------------------

/** State in DfpnHashTable.
    Do not forget to update DFPN_DB_VERSION if this class changes in a
    way that invalidiates old databases.  
    @ingroup dfpn
 */
class DfpnData
{
public:
    DfpnBounds m_bounds;

    DfpnChildren m_children;

    HexPoint m_bestMove;
    
    size_t m_work;

    bitset_t m_maxProofSet;

    float m_evaluationScore;

    DfpnData();

    DfpnData(const DfpnBounds& bounds, const DfpnChildren& children, 
             HexPoint bestMove, size_t work, bitset_t maxProofSet,
             float evaluationScore);

    ~DfpnData();

    std::string Print() const; 
    
    /** @name SgHashTable methods. */
    // @{

    bool IsValid() const;

    void Invalidate();
    
    bool IsBetterThan(const DfpnData& data) const;

    // @}

    /** @name PositionDBStateConcept */
    // @{

    int PackedSize() const;

    byte* Pack() const;

    void Unpack(const byte* data);

    void Rotate(const ConstBoard& brd);

    // @}

private:
    bool m_isValid;
};


inline DfpnData::DfpnData()
    : m_isValid(false)
{
}

inline DfpnData::DfpnData(const DfpnBounds& bounds, 
                          const DfpnChildren& children, 
                          HexPoint bestMove, size_t work,
                          bitset_t maxProofSet, float evaluationScore)
    : m_bounds(bounds),
      m_children(children),
      m_bestMove(bestMove),
      m_work(work),
      m_maxProofSet(maxProofSet),
      m_evaluationScore(evaluationScore),
      m_isValid(true)
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
       << "maxpfset=not printing"
       << "evaluation=" << m_evaluationScore
       << ']';
    return os.str();
}

inline bool DfpnData::IsBetterThan(const DfpnData& data) const
{
    SG_UNUSED(data);
    return true;
}

inline bool DfpnData::IsValid() const
{
    return m_isValid;
}

inline void DfpnData::Invalidate()
{
    m_isValid = false;
}

/** Extends global output operator for DfpnData. */
inline std::ostream& operator<<(std::ostream& os, const DfpnData& data)
{
    os << data.Print();
    return os;
}

//----------------------------------------------------------------------------

/** History of moves played from root state to current state. 
    @ingroup dfpn
*/
class DfpnHistory
{
public:
    DfpnHistory();

    /** Adds a new state to the history. */
    void Push(HexPoint m_move, SgHashCode hash);

    /** Removes last stated added from history. */
    void Pop();

    /** Returns number of moves played so far. */
    int Depth() const;

    /** Hash of last state. */
    SgHashCode LastHash() const;

    /** Move played from parent state to bring us to this state. */
    HexPoint LastMove() const;

private:

    /** Move played from state. */
    std::vector<HexPoint> m_move;

    /** Hash of state. */
    std::vector<SgHashCode> m_hash;
};

inline DfpnHistory::DfpnHistory()
{
    m_move.push_back(INVALID_POINT);
    m_hash.push_back(0);
}

inline void DfpnHistory::Push(HexPoint move, SgHashCode hash)
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
    BenzeneAssert(!m_move.empty());
    return static_cast<int>(m_move.size() - 1);
}

inline SgHashCode DfpnHistory::LastHash() const
{
    return m_hash.back();
}

inline HexPoint DfpnHistory::LastMove() const
{
    return m_move.back();
}

//----------------------------------------------------------------------------

/** Interface for listeners of DfpnSolver. 
    @ingroup dfpn
 */
class DfpnListener
{
public:
    virtual ~DfpnListener();

    /** Called when a state is solved. */
    virtual void StateSolved(const DfpnHistory& history, const DfpnData& data) = 0;
};

//----------------------------------------------------------------------------

/** Hashtable used in dfpn search.  
    @ingroup dfpn
*/
typedef SgHashTable<DfpnData> DfpnHashTable;

/** Database of solved positions. 
    @ingroup dfpn
*/
class DfpnDB : public StateDB<DfpnData>
{
public:
    static const std::string DFPN_DB_VERSION;

    DfpnDB(const std::string& filename)
        : StateDB<DfpnData>(filename, DFPN_DB_VERSION)
    { }
};

/** Combines a hashtable with a position db.
    @ingroup dfpn
*/
typedef SolverDB<DfpnHashTable, DfpnDB, DfpnData> DfpnStates;

//----------------------------------------------------------------------------

/** Hex solver using DFPN search. 
    @ingroup dfpn
*/
class DfpnSolver 
{
public:

    DfpnSolver();

    ~DfpnSolver();

    /** Solves the given state using the given hashtable. 
        Returns the color of the winning player (EMPTY if it could
        not determine a winner in time). */
    HexColor StartSearch(const HexState& state, HexBoard& brd, 
                         DfpnStates& positions, PointSequence& pv);

    HexColor StartSearch(const HexState& state, HexBoard& brd, 
                         DfpnStates& positions, PointSequence& pv,
                         const DfpnBounds& maxBounds);

    void AddListener(DfpnListener& listener);

    /** Returns various histograms pertaining to the evaluation
        function from the last search. */
    std::string EvaluationInfo() const;

    /** Updates bounds from this state to start of game or a state
        is not in position set. */
    void PropagateBackwards(const Game& game, DfpnStates& pos);

    //------------------------------------------------------------------------

    /** @name Parameters */
    // @{

    /** Dumps output about root state what gui can display. */
    bool UseGuiFx() const;

    /** See UseGuiFx() */
    void SetUseGuiFx(bool enable);

    /** Maximum time search is allowed to run before aborting. 
        Set to 0 for no timelimit. */
    double Timelimit() const;

    /** See Timelimit() */
    void SetTimelimit(double timelimit);

    /** Widening base affects what number of the moves to consider
        are always looked at by the dfpn search (omitting losing moves),
        regardless of branching factor. This amount is added to the
        proportion computed by the WideningFactor (see below).
        The base must be set to at least 1. */
    int WideningBase() const;

    /** See WideningBase() */
    void SetWideningBase(int wideningBase);

    /** Widening factor affects what fraction of the moves to consider
        are looked at by the dfpn search (omitting losing moves).
        Must be in the range (0, 1], where 1 ensures no pruning. */
    float WideningFactor() const;

    /** See WideningFactor() */
    void SetWideningFactor(float wideningFactor);

    // @}

private:

    /** Handles guifx output. */
    class GuiFx
    {
    public:

        GuiFx();

        void SetChildren(const DfpnChildren& children,
                         const std::vector<DfpnData>& bounds);

        void PlayMove(HexColor color, std::size_t index);

        void UndoMove();

        void UpdateCurrentBounds(const DfpnBounds& bounds);

        void Write();

        void WriteForced();

    private:
        
        DfpnChildren m_children;

        std::vector<DfpnData> m_data;

        HexColor m_color;

        std::size_t m_index;

        double m_timeOfLastWrite;

        std::size_t m_indexAtLastWrite;
        
        double m_delay;

        void DoWrite();
    };

    boost::scoped_ptr<HexState> m_state;

    HexBoard* m_workBoard;

    DfpnStates* m_positions;

    std::vector<DfpnListener*> m_listener;

    SgTimer m_timer;

    /** See UseGuiFx() */
    bool m_useGuiFx;

    /** See TimeLimit() */
    double m_timelimit;

    /** See WideningBase() */
    int m_wideningBase;

    /** See WideningFactor() */
    float m_wideningFactor;

    /** Number of calls to CheckAbort() before we check the timer.
        This is to avoid expensive calls to SgTime::Get(). Try to scale
        this so that it is checked twice a second. */
    size_t m_checkTimerAbortCalls;

    bool m_aborted;

    GuiFx m_guiFx;

    size_t m_numTerminal;

    size_t m_numMIDcalls;

    size_t m_numVCbuilds;

    SgStatisticsExt<float, std::size_t> m_prunedSiblingStats;

    SgStatisticsExt<float, std::size_t> m_moveOrderingPercent;

    SgStatisticsExt<float, std::size_t> m_moveOrderingIndex;

    SgStatisticsExt<float, std::size_t> m_considerSetSize;

    SgStatisticsExt<float, std::size_t> m_deltaIncrease;

    size_t m_totalWastedWork;

    SgHistogram<float, std::size_t> m_allEvaluation;

    SgHistogram<float, std::size_t> m_allSolvedEvaluation;

    SgHistogram<float, std::size_t> m_winningEvaluation;

    SgHistogram<float, std::size_t> m_losingEvaluation;

    size_t MID(const DfpnBounds& n, DfpnHistory& history);

    void SelectChild(std::size_t& bestIndex, DfpnBoundType& delta2, 
                     const std::vector<DfpnData>& childrenDfpnBounds,
                     size_t maxChildIndex) const;

    void UpdateBounds(DfpnBounds& bounds, 
                      const std::vector<DfpnData>& childBounds,
                      size_t maxChildIndex) const;

    bool CheckAbort();

    void LookupData(DfpnData& data, const DfpnChildren& children, 
                    std::size_t childIndex, HexState& state);

    bool TTRead(const HexState& state, DfpnData& data);

    void TTWrite(const HexState& state, const DfpnData& data);

    void DumpGuiFx(const std::vector<HexPoint>& children,
                   const std::vector<DfpnBounds>& childBounds) const;

    void PrintStatistics(HexColor winner, const PointSequence& p) const;

    size_t ComputeMaxChildIndex(const std::vector<DfpnData>&
                                childrenData) const;

    void DeleteChildren(DfpnChildren& children,
                        std::vector<DfpnData>& childrenData,
                        bitset_t deleteChildren) const;

    void NotifyListeners(const DfpnHistory& history, const DfpnData& data);
};

inline void DfpnSolver::AddListener(DfpnListener& listener)
{
    if (std::find(m_listener.begin(), m_listener.end(), &listener)
        != m_listener.end())
        m_listener.push_back(&listener);
}

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

inline int DfpnSolver::WideningBase() const
{
    return m_wideningBase;
}

inline void DfpnSolver::SetWideningBase(int wideningBase)
{
    m_wideningBase = wideningBase;
}

inline float DfpnSolver::WideningFactor() const
{
    return m_wideningFactor;
}

inline void DfpnSolver::SetWideningFactor(float wideningFactor)
{
    m_wideningFactor = wideningFactor;
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // SOLVERDFPN_HPP
