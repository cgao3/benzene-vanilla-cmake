//----------------------------------------------------------------------------
/** @file DfsSolver.hpp */
//----------------------------------------------------------------------------

#ifndef DFSSOLVER_H
#define DFSSOLVER_H

#include "SgTimer.h"
#include "SgHashTable.h"

#include "Hex.hpp"
#include "HexBoard.hpp"
#include "VC.hpp"
#include "DfsData.hpp"
#include "SolverDB.hpp"
#include "StateDB.hpp"
#include "HexEval.hpp"

#include <boost/scoped_ptr.hpp>

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Transposition table for use in DfsSolver. */
typedef SgHashTable<DfsData> DfsHashTable;

/** Database for use in DfsSolver. */
class DfsDB : public StateDB<DfsData>
{
public:
    static const std::string DFS_DB_VERSION;

    DfsDB(const std::string& filename)
        : StateDB<DfsData>(filename, DFS_DB_VERSION)
    { }
};

/** Solver database combining both of the above. */
typedef SolverDB<DfsHashTable, DfsDB, DfsData> DfsStates;

//----------------------------------------------------------------------------

/** Stats for a branch of the search tree. */
struct DfsBranchStatistics
{
    /** Total states in tree if no DB and no TT. */
    std::size_t total_states;

    /** States actually visited; includes leafs, tt and db hits. */
    std::size_t explored_states;

    /** Expanded nodes; non leaf, non tt and db hit states. */
    std::size_t expanded_states;

    /** Number of expanded nodes assuming perfect move ordering 
        (assuming the same set of winning moves). */
    std::size_t minimal_explored;
        
    /** Decompositions found; if black is to move, it must be a
        decomposition for white. */
    std::size_t decompositions;

    /** Decompositions where the player to move won. */
    std::size_t decompositions_won;
    
    /** Total number of moves to consider in expanded states. 
        Includes moves that are later pruned (by mustplay or
        from skipping due to finding a win). */
    std::size_t moves_to_consider;
    
    /** Number of expanded states that had winning moves. */
    std::size_t winning_expanded;
    
    /** Number of branches tried before win was found. */
    std::size_t branches_to_win;

    /** States pruned by mustplay pruning. */
    std::size_t pruned;
    
    /** Number of proofs that were successfully shrunk. */
    std::size_t shrunk;
    
    /** Total number of cells removed in all successful proof
        shrinkings. */
    std::size_t cells_removed;
    
    DfsBranchStatistics();

    void operator+=(const DfsBranchStatistics& o);
};

inline DfsBranchStatistics::DfsBranchStatistics()
    : total_states(0), 
      explored_states(0), 
      expanded_states(0), 
      minimal_explored(0), 
      decompositions(0), 
      decompositions_won(0), 
      moves_to_consider(0), 
      winning_expanded(0), 
      branches_to_win(0),
      pruned(0), 
      shrunk(0), 
      cells_removed(0)
{
}

inline void DfsBranchStatistics::operator+=(const DfsBranchStatistics& o)
{
    total_states += o.total_states;
    explored_states += o.explored_states;
    expanded_states += o.expanded_states;
    minimal_explored += o.minimal_explored;
    decompositions += o.decompositions;
    decompositions_won += o.decompositions_won;
    moves_to_consider += o.moves_to_consider;
    winning_expanded += o.winning_expanded;
    branches_to_win += o.branches_to_win;
    pruned += o.pruned;
    shrunk += o.shrunk;
    cells_removed += o.cells_removed;
}

//----------------------------------------------------------------------------

/** Stats for the entire search tree broken down by level. */
struct DfsHistogram
{
    /** Map of # of stones to a counter. */
    typedef std::map<std::size_t, std::size_t> StatsMap;

    /** Terminal states encountered at each depth. */
    StatsMap terminal;
        
    /** Internal states encountered at each depth. */
    StatsMap states;
    
    /** Winning states encountered at each depth. */
    StatsMap winning;
    
    StatsMap size_of_winning_states;
    
    StatsMap size_of_losing_states;
    
    /** Branches taken to find winning move at each depth. */
    StatsMap branches;
    
    /** Size of original mustplay in winning states. */
    StatsMap mustplay;
    
    /** States under losing moves before winning move. */
    StatsMap states_under_losing;
    
    /** DB/TT hits at each depth. */
    StatsMap tthits;
    
    /** Writes histogram in human-readable format to a string. */
    std::string Write();
};

//----------------------------------------------------------------------------

/** Contains all relevant data for a solution to a state. */
struct DfsSolutionSet
{
    bitset_t proof;
    
    int m_numMoves;
    
    PointSequence pv;
    
    DfsBranchStatistics stats;
    
    DfsSolutionSet();

    void SetPV(HexPoint cell);

    void SetPV(HexPoint cell, const PointSequence& pv);
};

inline DfsSolutionSet::DfsSolutionSet()
    : m_numMoves(0)
{
}

inline void DfsSolutionSet::SetPV(HexPoint cell)
{
    pv.clear();
    pv.push_back(cell);
}

inline void DfsSolutionSet::SetPV(HexPoint cell, const PointSequence& old)
{
    pv.clear();
    pv.push_back(cell);
    pv.insert(pv.end(), old.begin(), old.end());
}

//----------------------------------------------------------------------------

namespace DfsMoveOrderFlags
{
    /** Each move is played and the size of the resulting mustplay is
        stored. Moves are ordered in increasing order of mustplay.
        This is a very expensive move ordering, since the vcs
        and inferior cells must be updated for every possible move in
        every possible state.  However, the move ordering is usually
        very good. */
    static const int WITH_MUSTPLAY = 1;    

    /** Resistance score is used to break ties instead of distance
        from the center of the board. */
    static const int WITH_RESIST = 2;

    /** Moves near center of board get higher priority than moves near
        the edge of the board. */
    static const int FROM_CENTER = 4;
}

//----------------------------------------------------------------------------

/** Determines the winner of a gamestate.
    DfsSolver uses a mustplay driven depth-first search to determine the
    winner in the given state.
*/
class DfsSolver 
{
public:
    DfsSolver();

    ~DfsSolver();

    //------------------------------------------------------------------------

    /** Solves state using the given previously solved positions.
        Returns color of winner or EMPTY if aborted before state was
        solved.
    */
    HexColor Solve(const HexState& state, HexBoard& board, 
                   DfsSolutionSet& solution,
                   DfsStates& positions, int depthLimit = -1, 
                   double timeLimit = -1.0);

    //------------------------------------------------------------------------

    /** @name Parameters */
    // @{

    /** Controls whether gamestates decomposible into separate
        components have each side solved separately and the proofs
        combined as necessary. */
    bool UseDecompositions() const;

    /** See UseDecompositions() */
    void SetUseDecompositions(bool enable);

    /** Depth from root in which the current variation is printed. */
    int ProgressDepth() const;

    /** See ProgressDepth() */
    void SetProgressDepth(int depth);

    /** Depth at which the current state is dumped to the log. */
    std::size_t UpdateDepth() const;

    /** See UpdateDepth() */
    void SetUpdateDepth(std::size_t depth);

    /** Whether ICE is used to provably shrink proofs. */
    bool ShrinkProofs() const;

    /** See ShrinkProofs() */
    void SetShrinkProofs(bool enable);

    /** Use newly acquired ICE-info after the move ordering stage to
        prune the moves to consider. */
    bool BackupIceInfo() const;

    /** See BackupIceInfo() */
    void SetBackupIceInfo(bool enable);

    bool UseGuiFx() const;

    /** See UseGuiFx() */
    void SetUseGuiFx(bool enable);

    /** Returns the move order flags. */
    int MoveOrdering() const;

    /** See MoveOrdering() */
    void SetMoveOrdering(int flags);

    // @}

    //------------------------------------------------------------------------

    /** Dumps the stats on # of states, branching factors, etc, for
        the last run. */
    void DumpStats(const DfsSolutionSet& solution) const;

    /** Returns histogram of last search. */
    DfsHistogram Histogram() const;

private:

    //------------------------------------------------------------------------
    
    /** Globabl statistics for the current solver run. */
    struct GlobalStatistics
    {
        /** Times HexBoard::PlayMove() was called. */
        std::size_t played;

        GlobalStatistics()
            : played(0)
        { }
    };

    //------------------------------------------------------------------------

    DfsStates* m_positions;

    HexBoard* m_workBrd;

    SgTimer m_timer;

    std::vector<std::pair<int, int> > m_completed;

    bool m_aborted;

    mutable DfsHistogram m_histogram;

    mutable GlobalStatistics m_statistics;
    
    boost::scoped_ptr<HexState> m_state;

    /** See UseDecompositions() */
    bool m_use_decompositions;

    /** See UpdateDepth() */
    std::size_t m_update_depth;

    /** See ShrinkProofs() */
    bool m_shrink_proofs;

    /** See BackupIceInfo() */
    bool m_backup_ice_info;

    /** See UseGuiFx() */
    bool m_use_guifx;

    /** See MoveOrdering() */
    int m_move_ordering;

    std::size_t m_last_histogram_dump;

    int m_depthLimit;
    
    double m_timeLimit;

    //------------------------------------------------------------------------

    void PlayMove(HexPoint cell);
    
    void UndoMove(HexPoint cell);

    bool CheckTransposition(DfsData& state) const;

    void StoreState(const DfsData& state, const bitset_t& proof);

    bool CheckAbort();
    
    bool HandleLeafNode(DfsData& state, bitset_t& proof) const;

    bool HandleTerminalNode(DfsData& state, bitset_t& proof) const;

    bool OrderMoves(bitset_t& mustplay, DfsSolutionSet& solution, 
                    std::vector<HexMoveValue>& moves);

    bool SolveState(PointSequence& variation, DfsSolutionSet& solution);

    bool SolveDecomposition(PointSequence& variation,
                            DfsSolutionSet& solution, HexPoint group);

    bool SolveInteriorState(PointSequence& variation, 
                            DfsSolutionSet& solution);

    void HandleProof(const PointSequence& variation,
                     bool winning_state, DfsSolutionSet& solution);
};

//----------------------------------------------------------------------------

inline bool DfsSolver::UseDecompositions() const
{
    return m_use_decompositions;
}

inline void DfsSolver::SetUseDecompositions(bool enable)
{
    m_use_decompositions = enable;
}

inline std::size_t DfsSolver::UpdateDepth() const
{
    return m_update_depth;
}

inline void DfsSolver::SetUpdateDepth(std::size_t depth)
{
    m_update_depth = depth;
}

inline bool DfsSolver::ShrinkProofs() const
{
    return m_shrink_proofs;
}

inline void DfsSolver::SetShrinkProofs(bool enable)
{
    m_shrink_proofs = enable;
}

inline bool DfsSolver::BackupIceInfo() const
{
    return m_backup_ice_info;
}

inline void DfsSolver::SetBackupIceInfo(bool enable)
{
    m_backup_ice_info = enable;
}

inline bool DfsSolver::UseGuiFx() const
{
    return m_use_guifx;
}

inline void DfsSolver::SetUseGuiFx(bool enable)
{
    m_use_guifx = enable;
}

inline int DfsSolver::MoveOrdering() const
{
    return m_move_ordering;
}

inline void DfsSolver::SetMoveOrdering(int flags)
{
    m_move_ordering = flags;
}

inline DfsHistogram DfsSolver::Histogram() const
{
    return m_histogram;
}

//----------------------------------------------------------------------------

/** Methods in DfsSolver that do not need DfsSolver's private data. */
namespace DfsSolverUtil 
{
    /** Computes distance from the center of the board. */
    int DistanceFromCenter(const ConstBoard& brd, HexPoint p);
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // DFSSOLVER_H
