//----------------------------------------------------------------------------
/** @file DfsSolver.hpp
 */
//----------------------------------------------------------------------------

#ifndef DFSSOLVER_H
#define DFSSOLVER_H

#include <boost/scoped_ptr.hpp>

#include "Hex.hpp"
#include "HexBoard.hpp"
#include "VC.hpp"
#include "TransTable.hpp"
#include "DfsData.hpp"
#include "SolverDB.hpp"
#include "PositionDB.hpp"
#include "HexEval.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Transposition table for use in DfsSolver. */
typedef TransTable<DfsData> DfsHashTable;

/** Database for use in DfsSolver. */
typedef PositionDB<DfsData> DfsDB;

/** Solver database combining both of the above. */
typedef SolverDB<DfsHashTable, DfsDB, DfsData> DfsPositions;

//----------------------------------------------------------------------------

/** Stats for a branch of the search tree. */
struct DfsBranchStatistics
{
    /** Total states in tree if no DB and no TT. */
    unsigned total_states;

    /** States actually visited; includes leafs, tt and db hits. */
    unsigned explored_states;

    /** Expanded nodes; non leaf, non tt and db hit states. */
    unsigned expanded_states;

    /** Number of expanded nodes assuming perfect move ordering 
        (assuming the same set of winning moves). */
    unsigned minimal_explored;
        
    /** Decompositions found; if black is to move, it must be a
        decomposition for white. */
    unsigned decompositions;

    /** Decompositions where the player to move won. */
    unsigned decompositions_won;
    
    /** Total number of moves to consider in expanded states. 
        Includes moves that are later pruned (by mustplay or
        from skipping due to finding a win). */
    unsigned moves_to_consider;
    
    /** Number of expanded states that had winning moves. */
    unsigned winning_expanded;
    
    /** Number of branches tried before win was found. */
    unsigned branches_to_win;

    /** States pruned by mustplay pruning. */
    unsigned pruned;
    
    /** Number of proofs that were successfully shrunk. */
    unsigned shrunk;
    
    /** Total number of cells removed in all successful proof
        shrinkings. */
    unsigned cells_removed;
    
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

/** Contains all relevant data for a solution to a state. */
struct DfsSolutionSet
{
    bitset_t proof;
    
    int moves_to_connection;
    
    PointSequence pv;
    
    DfsBranchStatistics stats;
    
    DfsSolutionSet();
};

inline DfsSolutionSet::DfsSolutionSet()
    : moves_to_connection(0)
{
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
};

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
    HexColor Solve(HexBoard& board, HexColor toplay, DfsSolutionSet& solution,
                   DfsPositions& positions, int depthLimit = -1, 
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
    int UpdateDepth() const;

    /** See UpdateDepth() */
    void SetUpdateDepth(int depth);

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

private:

    //------------------------------------------------------------------------
    
    /** Globabl statistics for the current solver run. */
    struct GlobalStatistics
    {
        /** Times HexBoard::PlayMove() was called. */
        unsigned played;

        GlobalStatistics()
            : played(0)
        { }
    };

    //------------------------------------------------------------------------

    /** Stats for the entire search tree broken down by level. */
    struct Histogram
    {
        /** Map of # of stones to a counter. */
        typedef std::map<int, std::size_t> StatsMap;

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

        /** Dumps histogram to a string. */
        std::string Dump();
    };

    //------------------------------------------------------------------------

    DfsPositions* m_positions;

    double m_start_time;
        
    double m_end_time;

    std::vector<std::pair<int, int> > m_completed;

    bool m_aborted;

    mutable Histogram m_histogram;

    mutable GlobalStatistics m_statistics;
    
    /** Board with no fillin. */
    boost::scoped_ptr<StoneBoard> m_stoneboard;

    /** See UseDecompositions() */
    bool m_use_decompositions;

    /** See UpdateDepth() */
    int m_update_depth;

    /** See ShrinkProofs() */
    bool m_shrink_proofs;

    /** See BackupIceInfo() */
    bool m_backup_ice_info;

    /** See UseGuiFx() */
    bool m_use_guifx;

    /** See MoveOrdering() */
    int m_move_ordering;

    unsigned m_last_histogram_dump;

    int m_depthLimit;
    
    double m_timeLimit;

    //------------------------------------------------------------------------

    void PlayMove(HexBoard& brd, HexPoint cell, HexColor color);
    
    void UndoMove(HexBoard& brd, HexPoint cell);

    bool CheckTransposition(DfsData& state) const;

    void StoreState(HexColor color, const DfsData& state, 
                    const bitset_t& proof);

    bool CheckAbort();
    
    bool HandleLeafNode(const HexBoard& brd, HexColor color, 
                        DfsData& state, bitset_t& proof) const;

    bool HandleTerminalNode(const HexBoard& brd, HexColor color, 
                            DfsData& state, bitset_t& proof) const;

    bool OrderMoves(HexBoard& brd, HexColor color, bitset_t& mustplay, 
                    DfsSolutionSet& solution, 
                    std::vector<HexMoveValue>& moves);

    bool solve_state(HexBoard& brd, HexColor tomove, PointSequence& variation, 
                     DfsSolutionSet& solution);

    bool solve_decomposition(HexBoard& brd, HexColor color, 
                             PointSequence& variation,
                             DfsSolutionSet& solution, HexPoint group);

    bool solve_interior_state(HexBoard& brd, HexColor color, 
                              PointSequence& variation, 
                              DfsSolutionSet& solution);

    void handle_proof(const HexBoard& brd, HexColor color, 
                      const PointSequence& variation,
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

inline int DfsSolver::UpdateDepth() const
{
    return m_update_depth;
}

inline void DfsSolver::SetUpdateDepth(int depth)
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

//----------------------------------------------------------------------------

/** Methods in DfsSolver that do not need DfsSolver's private data. */
namespace DfsSolverUtil 
{
    /** Computes distance from the center of the board. */
    int DistanceFromCenter(const ConstBoard& brd, HexPoint p);

    /** Determines if this is a winning state. If so, proof is set to
        the winning proof. */
    bool isWinningState(const HexBoard& brd, HexColor color, 
                        bitset_t& proof);

    /** Determines if the given board state is losing for color. If
        so, proof is set. */
    bool isLosingState(const HexBoard& brd, HexColor color, bitset_t& proof);

    /** Computes the moves to consider for this state. 
        @todo Switch to PlayerUtils::MovesToConsider()
    */
    bitset_t MovesToConsider(const HexBoard& brd, HexColor color);
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // DFSSOLVER_H
