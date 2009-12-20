//----------------------------------------------------------------------------
/** @file DfsSolver.cpp
 */
//----------------------------------------------------------------------------

#include "SgSystem.h"

#include "Hex.hpp"
#include "VCSet.hpp"
#include "HexProp.hpp"
#include "HexBoard.hpp"
#include "GraphUtils.hpp"
#include "Resistance.hpp"
#include "DfsSolver.hpp"
#include "Time.hpp"
#include "VCUtils.hpp"
#include "BoardUtils.hpp"
#include "BitsetIterator.hpp"
#include "PlayerUtils.hpp"
#include "ProofUtil.hpp"

#include <cmath>
#include <algorithm>
#include <boost/scoped_ptr.hpp>

using namespace benzene;

//----------------------------------------------------------------------------

DfsSolver::DfsSolver()
    : m_positions(0),
      m_use_decompositions(true),
      m_update_depth(4),
      m_shrink_proofs(true),
      m_backup_ice_info(true),
      m_use_guifx(false),
      m_move_ordering(DfsMoveOrderFlags::WITH_MUSTPLAY 
                      | DfsMoveOrderFlags::WITH_RESIST 
                      | DfsMoveOrderFlags::FROM_CENTER)
{
}

DfsSolver::~DfsSolver()
{
}

//----------------------------------------------------------------------------

HexColor DfsSolver::Solve(HexBoard& brd, HexColor toPlay, 
                          DfsSolutionSet& solution, DfsPositions& positions,
                          int depthLimit, double timeLimit)
{
    m_positions = &positions;
    m_depthLimit = depthLimit;
    m_timeLimit = timeLimit;
    
    m_aborted = false;
    m_start_time = Time::Get();
    m_histogram = Histogram();
    m_last_histogram_dump = 0;
    m_statistics = GlobalStatistics();
    m_stoneboard.reset(new StoneBoard(brd.GetState()));

    // DfsSolver currently cannot handle permanently inferior cells.
    if (brd.ICE().FindPermanentlyInferior())
        throw BenzeneException("Permanently Inferior not supported "
                               "in DfsSolver!");

    // Check if move already is already solved
    DfsData state;
    bool win = false;
    if (CheckTransposition(state))
    {
        LogInfo() << "DfsSolver: Found cached result!\n";
        win = state.m_win;
        solution.moves_to_connection = state.m_numMoves;
        solution.pv.clear();
        SolverDBUtil::GetVariation(*m_stoneboard, toPlay, positions,
                                   solution.pv);
        solution.proof = DefaultProofForWinner(brd, state.m_win ? 
                                               toPlay : !toPlay);
    }
    else
    {
        brd.ComputeAll(toPlay);
        m_completed.resize(BITSETSIZE);
        PointSequence variation;
        win = solve_state(brd, toPlay, variation, solution);
    }
    solution.proof &= brd.GetState().GetEmpty();
    m_end_time = Time::Get();
    if (m_aborted) 
        return EMPTY;
    return win ? toPlay : !toPlay;
}

//----------------------------------------------------------------------------

bitset_t 
DfsSolver::DefaultProofForWinner(const HexBoard& brd, HexColor winner) const
{
    return (brd.GetState().GetColor(winner) | brd.GetState().GetEmpty()) 
        - brd.GetDead();
}

bool DfsSolver::CheckTransposition(DfsData& state) const
{
    return m_positions->Get(*m_stoneboard, state);
}

void DfsSolver::StoreState(const DfsData& state, const bitset_t& proof)
{
    m_positions->Put(*m_stoneboard, state);
    const SolverDBParameters& param = m_positions->Parameters();
    if (m_stoneboard->NumStones() <= param.m_transStones)
    {
        if (param.m_useProofTranspositions)
            ProofUtil::StoreTranspositions(*m_positions->Database(),
                                           *m_stoneboard, state, proof);
        if (param.m_useFlippedStates)
            ProofUtil::StoreFlippedStates(*m_positions->Database(), 
                                          *m_stoneboard, state, proof);
    }
}

//----------------------------------------------------------------------------

/** Checks timelimit and SgUserAbort(). Sets m_aborted if necessary,
    aborting the search. Returns true if search should be aborted,
    false otherwise. */
bool DfsSolver::CheckAbort()
{
    if (!m_aborted)
    {
        if (SgUserAbort()) 
        {
            m_aborted = true;
            LogInfo() << "DfsSolver::CheckAbort(): Abort flag!\n";
        }
        else if ((m_timeLimit > 0) && 
                 ((Time::Get() - m_start_time) > m_timeLimit))
        {
            m_aborted = true;
            LogInfo() << "DfsSolver::CheckAbort(): Timelimit!\n";
        }
    }
    return m_aborted;
}

/** Returns true if node is terminal. Fills in state if terminal.
    State's bestmove field is not specified here.
*/
bool DfsSolver::HandleTerminalNode(const HexBoard& brd, HexColor color,
                                   DfsData& state, bitset_t& proof) const
{
    int numstones = m_stoneboard->NumStones();
    if (DfsSolverUtil::isWinningState(brd, color, proof)) 
    {
        state.m_win = true;
        state.m_numMoves = 0;
        state.m_numStates = 1;
        m_histogram.terminal[numstones]++;
        return true;
    } 
    else if (DfsSolverUtil::isLosingState(brd, color, proof)) 
    {
        state.m_win = false;
        state.m_numMoves = 0;
        state.m_numStates = 1;
        m_histogram.terminal[numstones]++;
        return true;
    } 
    return false;
}

/** Returns true if current state is a terminal node (win/loss), or a
    DB/TT hit. If so, info is stored in state. */
bool DfsSolver::HandleLeafNode(const HexBoard& brd, HexColor color, 
                               DfsData& state, bitset_t& proof) const
{
    if (HandleTerminalNode(brd, color, state, proof))
        return true;
    if (CheckTransposition(state))
        proof = DefaultProofForWinner(brd, state.m_win ? color : !color);
    return false;
}

//----------------------------------------------------------------------------

/** Solves the current state in brd for the color to move. Handles
    decompositions if option is turned on. */
bool DfsSolver::solve_state(HexBoard& brd, HexColor color, 
                            PointSequence& variation, DfsSolutionSet& solution)
{
    if (CheckAbort()) 
        return false;

    // Check for VC/DB/TT states
    {
        DfsData state;
        bitset_t proof;
        if (HandleLeafNode(brd, color, state, proof)) 
        {
            solution.stats.explored_states = 1;
            solution.stats.minimal_explored = 1;
            solution.stats.total_states += state.m_numStates;
            
            solution.pv.clear();
            solution.moves_to_connection = state.m_numMoves;
            solution.proof = proof;
            
            return state.m_win;
        }
    }

    // Solve decompositions if they exist, otherwise solve the state
    // normally.
    bool winning_state = false;
    {
        HexPoint group;
        if (m_use_decompositions
            && BoardUtils::FindSplittingDecomposition(brd, !color, group))
        {
            winning_state = solve_decomposition(brd, color, variation, 
                                                solution, group);
        } 
        else 
        {
            winning_state = solve_interior_state(brd, color, variation, 
                                                 solution);
        }
    }

    // Shrink, verify, and store proof in DB/TT.
    handle_proof(brd, color, variation, winning_state, solution);

    // Dump histogram every 1M moves
    if ((m_statistics.played / 1000000) > (m_last_histogram_dump)) 
    {
        LogInfo() << m_histogram.Dump() << '\n';
        m_last_histogram_dump = m_statistics.played / 1000000;
    }
    return winning_state;
}

/** Solves each side of the decompsosition; combines proofs if
    necessary. */
bool DfsSolver::solve_decomposition(HexBoard& brd, HexColor color, 
                                    PointSequence& variation,
                                    DfsSolutionSet& solution,
                                    HexPoint group)
{
    solution.stats.decompositions++;

    LogFine() << "FOUND DECOMPOSITION FOR " << !color << '\n'
	      << "Group: "<< group << '\n' << brd << '\n';

    // compute the carriers for each side 
    PointToBitset nbs;
    GraphUtils::ComputeDigraph(brd.GetGroups(), !color, nbs);
    bitset_t stopset = nbs[group];

    bitset_t carrier[2];
    carrier[0] = 
        GraphUtils::BFS(HexPointUtil::colorEdge1(!color), nbs, stopset);
    carrier[1] = 
        GraphUtils::BFS(HexPointUtil::colorEdge2(!color), nbs, stopset);

    if ((carrier[0] & carrier[1]).any())
        throw BenzeneException()
            << "DfsSolver::solve_decomposition:\n"
            << "Side0:" << brd.Write(carrier[0]) << '\n'
            << "Side1:" << brd.Write(carrier[1]) << '\n';
        
    // solve each side
    DfsData state;
    DfsSolutionSet dsolution[2];
    for (int s = 0; s < 2; ++s) 
    {
        LogFine() << "----------- Side" << s << ":" 
		  << brd.Write(carrier[s]) << '\n';

        bool win = false;
        brd.PlayStones(!color, carrier[s^1] & brd.Const().GetCells(), color);

        // check if new stones caused terminal state; if not, solve it
        bitset_t proof;
        if (HandleTerminalNode(brd, color, state, proof)) 
        {
            win = state.m_win;
            dsolution[s].stats.expanded_states = 0;
            dsolution[s].stats.explored_states = 1;
            dsolution[s].stats.minimal_explored = 1;
            dsolution[s].stats.total_states = 1;
            dsolution[s].proof = proof;
            dsolution[s].moves_to_connection = state.m_numMoves;
            dsolution[s].pv.clear();
        } 
        else 
        {
            win = solve_interior_state(brd, color, variation, dsolution[s]);
        }
        brd.UndoMove();
        
        // abort if we won this side
        if (win) 
        {
            LogFine() << "##### WON SIDE " << s << " #####" << '\n'
		      << brd.Write(dsolution[s].proof) << '\n'
		      << "explored_states: " 
		      << dsolution[s].stats.explored_states << '\n';
            
            solution.pv = dsolution[s].pv;
            solution.proof = dsolution[s].proof;
            solution.moves_to_connection = dsolution[s].moves_to_connection;
            solution.stats += dsolution[s].stats;
            solution.stats.decompositions_won += 
                dsolution[s].stats.decompositions_won + 1;
            return true;
        } 
    }
        
    // combine the two losing proofs
    solution.pv = dsolution[0].pv;
    solution.pv.insert(solution.pv.end(), 
                       dsolution[1].pv.begin(), 
                       dsolution[1].pv.end());

    solution.moves_to_connection = 
        dsolution[0].moves_to_connection + 
        dsolution[1].moves_to_connection;
    
    solution.proof = 
        (dsolution[0].proof & carrier[0]) | 
        (dsolution[1].proof & carrier[1]) |
        brd.GetState().GetColor(!color);
    solution.proof = solution.proof - brd.GetDead();

    int s0 = (int)dsolution[0].stats.explored_states;
    int s1 = (int)dsolution[1].stats.explored_states;
    
    LogFine() << "##### LOST BOTH SIDES! #####" << '\n'
	      << "Side0: " << s0 << " explored." << '\n'
	      << "Side1: " << s1 << " explored." << '\n'
	      << "Saved: " << (s0*s1) - (s0+s1) << '\n'
	      << brd.Write(solution.proof) << '\n';
    return false;
}

/** Does the recursive mustplay search; calls solve_state() on child
    states. */
bool DfsSolver::solve_interior_state(HexBoard& brd, HexColor color, 
                                     PointSequence& variation,
                                     DfsSolutionSet& solution)
{
    int depth = variation.size();
    int numstones = m_stoneboard->NumStones();

    // Set initial proof for this state to be the union of all
    // opponent winning semis.  We need to do this because we use the
    // semis to restrict the search (ie, the mustplay).
    // Proof will also include all opponent stones. 
    //
    // Basically, we are assuming the opponent will win from this state;
    // if we win instead, we use the proof generated from that state,
    // not this one. 
    solution.proof = DfsSolverUtil::InitialProof(brd, color);

    // Get the moves to consider
    bitset_t mustplay = DfsSolverUtil::MovesToConsider(brd, color, 
                                                       solution.proof);
    LogFine() << "mustplay: [" << HexPointUtil::ToString(mustplay) << " ]\n";

    // output progress for the gui
    if (depth == m_update_depth && m_use_guifx)
    {
        std::ostringstream os;
        os << "gogui-gfx:\n";
        os << "solver\n";
        os << "VAR";
        HexColor toplay = (variation.size()&1) ? !color : color;
        for (std::size_t i = 0; i < variation.size(); ++i) 
        {
            os << " " << ((toplay == BLACK) ? "B" : "W");
            os << " " << variation[i];
            toplay = !toplay;
        }
        os << '\n';
        os << "LABEL ";
        const InferiorCells& inf = brd.GetInferiorCells();
        os << inf.GuiOutput();
        os << BoardUtils::GuiDumpOutsideConsiderSet(brd.GetState(), 
                                                    mustplay, inf.All());
        os << '\n';
        os << "TEXT";
        for (int i = 0; i < depth; ++i) 
        {
            os << " " << m_completed[i].first 
               << "/" << m_completed[i].second;
        }
        os << '\n';
        os << '\n';
        std::cout << os.str();
        std::cout.flush();
    } 

    // If mustplay is empty then this is a losing state.
    if (mustplay.none()) 
    {
        LogFine() << "Empty reduced mustplay.\n"
		  << brd.Write(solution.proof) << '\n';
        m_histogram.terminal[numstones]++;
        solution.stats.total_states = 1;
        solution.stats.explored_states = 1;
        solution.stats.minimal_explored = 1;
        solution.pv.clear();
        solution.moves_to_connection = 0;
        return false;  
    }

    bitset_t original_mustplay = mustplay;

    solution.stats.total_states = 1;
    solution.stats.explored_states = 1;
    solution.stats.minimal_explored = 1;
    solution.stats.expanded_states = 1;
    solution.stats.moves_to_consider = mustplay.count();
    m_histogram.states[numstones]++;

    // Order moves in the mustplay.
    //
    // @note If we want to find all winning moves then 
    // we need to stop OrderMoves() from aborting on a win.
    //
    // @note OrderMoves() will handle VC/DB/TT hits, and remove them
    // from consideration. It is possible that there are no moves, in
    // which case we fall through the loop below with no problem (the
    // state is a loss).
    solution.moves_to_connection = -1;
    std::vector<HexMoveValue> moves;
    bool winning_state = OrderMoves(brd, color, mustplay, solution, moves);

    //----------------------------------------------------------------------
    // Expand all moves in mustplay that were not leaf states.
    //----------------------------------------------------------------------
    std::size_t states_under_losing = 0;
    bool made_it_through = false;

    for (unsigned index=0; 
         !winning_state && index<moves.size(); 
         ++index) 
    {
        HexPoint cell = moves[index].point();
        
        // note the level of completion
        m_completed[depth] = std::make_pair(index, moves.size());

        // skip moves that were pruned by the proofs of previous moves
        if (!mustplay.test(cell)) 
        {
            solution.stats.pruned++;
            continue;
        }

	made_it_through = true;
        DfsSolutionSet child_solution;
        PlayMove(brd, cell, color);
        variation.push_back(cell);

        bool win = !solve_state(brd, !color, variation, child_solution);

        variation.pop_back();
        UndoMove(brd, cell);

        solution.stats += child_solution.stats;

        if (win) 
        {
            // Win: copy proof over, copy pv, abort!
            winning_state = true;
            solution.proof = child_solution.proof;

            solution.pv.clear();
            solution.pv.push_back(cell);
            solution.pv.insert(solution.pv.end(), child_solution.pv.begin(), 
                               child_solution.pv.end());

            solution.moves_to_connection = 
                child_solution.moves_to_connection + 1;

            // set minimal tree-size explicitly to be child's minimal size
            // plus 1.
            solution.stats.minimal_explored = 
                child_solution.stats.minimal_explored + 1;

            solution.stats.winning_expanded++;
            solution.stats.branches_to_win += index+1;

            m_histogram.winning[numstones]++;
            m_histogram.size_of_winning_states[numstones] 
                += child_solution.stats.explored_states;

            m_histogram.branches[numstones] += index+1;
            m_histogram.states_under_losing[numstones] += states_under_losing;
            m_histogram.mustplay[numstones] += original_mustplay.count();

	    if (solution.moves_to_connection == -1) 
            {
		LogInfo() << "child_solution.moves_to_connection == " 
			  << child_solution.moves_to_connection << '\n';
	    }
	    HexAssert(solution.moves_to_connection != -1);	    
        } 
        else 
        {
            // Loss: add returned proof to current proof. Prune
            // mustplay by proof.  Maintain PV to longest loss.

            mustplay &= child_solution.proof;
            solution.proof |= child_solution.proof;
            states_under_losing += child_solution.stats.explored_states;

            m_histogram.size_of_losing_states[numstones] 
                += child_solution.stats.explored_states;

            if (child_solution.moves_to_connection + 1 > 
                solution.moves_to_connection) 
            {
                solution.moves_to_connection = 
                    child_solution.moves_to_connection + 1;

                solution.pv.clear();
                solution.pv.push_back(cell);
                solution.pv.insert(solution.pv.end(), 
                                   child_solution.pv.begin(), 
                                   child_solution.pv.end());
            }
	    if (solution.moves_to_connection == -1) 
            {
		LogInfo() << "child_solution.moves_to_connection == " 
			  << child_solution.moves_to_connection << '\n';
	    }
	    HexAssert(solution.moves_to_connection != -1);
        }
    }

    if (solution.moves_to_connection == -1) 
    {
	LogInfo() << "moves_to_connection == -1 and "
		  << "made_it_through = " << made_it_through << '\n';
    }
    HexAssert(solution.moves_to_connection != -1);
    return winning_state;
}

/** Shrinks/verifies proof then stores it. */
void DfsSolver::handle_proof(const HexBoard& brd, HexColor color, 
                             const PointSequence& variation,
                             bool winning_state, DfsSolutionSet& solution)
{
    if (m_aborted)
        return;
    HexColor winner = (winning_state) ? color : !color;
    HexColor loser = !winner;
    // Verify loser's stones do not intersect proof
    if ((brd.GetState().GetColor(loser) & solution.proof).any()) 
        throw BenzeneException()
            << "DfsSolver::handle_proof:\n"
            << color << " to play.\n"
            << loser << " loses.\n"
            << "Losing stones hit proof:\n"
            << brd.Write(solution.proof) << '\n'
            << brd << '\n'
            << "PV: " << HexPointUtil::ToString(variation) << '\n';
    // Verify dead cells do not intersect proof
    if ((brd.GetDead() & solution.proof).any()) 
        throw BenzeneException()
            << "DfsSolver::handle_proof:\n"
            << color << " to play.\n"
            << loser << " loses.\n"
            << "Dead cells hit proof:\n"
            << brd.Write(solution.proof) << '\n'
            << brd << '\n'
            << "PV: " << HexPointUtil::ToString(variation) << '\n';
    // Shrink proof
    bitset_t old_proof = solution.proof;
    if (m_shrink_proofs) 
    {
        ProofUtil::ShrinkProof(solution.proof, *m_stoneboard, loser, 
                               brd.ICE());
        bitset_t pruned;
        pruned  = BoardUtils::ReachableOnBitset(brd.Const(), solution.proof, 
                                 EMPTY_BITSET,
                                 HexPointUtil::colorEdge1(winner));
        pruned &= BoardUtils::ReachableOnBitset(brd.Const(), solution.proof, 
                                 EMPTY_BITSET,
                                 HexPointUtil::colorEdge2(winner));
        solution.proof = pruned;

        if (solution.proof.count() < old_proof.count()) 
        {
            solution.stats.shrunk++;
            solution.stats.cells_removed 
                += old_proof.count() - solution.proof.count();
        }
    }
    // Verify proof touches both of winner's edges
    if (!BoardUtils::ConnectedOnBitset(brd.Const(), solution.proof, 
                                       HexPointUtil::colorEdge1(winner),
                                       HexPointUtil::colorEdge2(winner)))
        throw BenzeneException()
            << "DfsSolver::handle_proof:\n"
            << "Proof does not touch both edges!\n"
            << brd.Write(solution.proof) << '\n'
            << "Original proof:\n"
            << brd.Write(old_proof) << '\n'
            << brd << '\n'
            << color << " to play.\n"
            << "PV: " << HexPointUtil::ToString(variation) << '\n';

    /** @todo HANDLE BEST MOVES PROPERLY! 
        This can only happen if the mustplay goes empty in an internal
        state that wasn't determined initially, or in a decomp state
        where the fillin causes a terminal state. 
     */
    if (solution.pv.empty())
        solution.pv.push_back(INVALID_POINT);

    StoreState(DfsData(winning_state, solution.stats.total_states, 
                       solution.moves_to_connection, solution.pv[0]), 
               solution.proof);
}

//----------------------------------------------------------------------------

/** Plays the move; updates the board.  */
void DfsSolver::PlayMove(HexBoard& brd, HexPoint cell, HexColor color) 
{
    m_statistics.played++;
    m_stoneboard->PlayMove(color, cell);
    brd.PlayMove(color, cell);
}

/** Takes back the move played. */
void DfsSolver::UndoMove(HexBoard& brd, HexPoint cell)
{
    m_stoneboard->UndoMove(cell);
    brd.UndoMove();
}

//----------------------------------------------------------------------------

/** Orders the moves in mustplay using several heuristics.  Aborts
    move ordering early if it finds a TT win: winning move is put to
    the front.

    Will shrink the mustplay if it encounters TT losses: losing moves
    are not added to the list of sorted moves.

    Returns true if it found a TT win, false otherwise.
*/
bool DfsSolver::OrderMoves(HexBoard& brd, HexColor color, bitset_t& mustplay, 
                           DfsSolutionSet& solution,
                           std::vector<HexMoveValue>& moves)
{        
    LogFine() << "OrderMoves\n";
    HexColor other = !color;

    // union and intersection of proofs for all losing moves
    bitset_t proof_union;
    bitset_t proof_intersection;
    proof_intersection.set();

    /** The TT/DB checks are done as a single 1-ply sweep prior to any
        move ordering, since computing the VCs for any solved states
        is pointless, plus these may resolve the current state immediately.
    */
    bool found_win = false;
    bitset_t losingMoves;
    LogFine() << "STARTING!\n";
    for (BitsetIterator it(mustplay); !found_win && it; ++it)
    {
	brd.GetState().PlayMove(color, *it);
	m_stoneboard->PlayMove(color, *it);

	DfsData state;
	if (CheckTransposition(state))
	{
	    solution.stats.explored_states += 1;
	    solution.stats.minimal_explored++;
	    solution.stats.total_states += state.m_numStates;

	    if (!state.m_win)
	    {
		found_win = true;
		moves.clear();
		moves.push_back(HexMoveValue(*it, 0));
		
		// this state plus the child winning state
		// (which is a leaf).
		solution.stats.minimal_explored = 2;
                solution.proof = DefaultProofForWinner(brd, color);

		solution.moves_to_connection = state.m_numMoves + 1;
		solution.pv.clear();
		solution.pv.push_back(*it);
	    } 
	    else 
	    {
		// prune this losing move from the mustplay
		losingMoves.set(*it);
		if (state.m_numMoves + 1 > solution.moves_to_connection) 
                {
		    solution.moves_to_connection = state.m_numMoves + 1;
		    solution.pv.clear();
		    solution.pv.push_back(*it);
		}
		// will prune the mustplay later on with the proof
                bitset_t proof = DefaultProofForWinner(brd, !color);
		proof_intersection &= proof;
		proof_union |= proof;
	    }
	}
	brd.GetState().UndoMove(*it);
	m_stoneboard->UndoMove(*it);
    }
    
    if (found_win)
    {
	HexAssert(moves.size() == 1);
	LogFine() << "Found winning move; aborted ordering.\n";
	return true;
    }

    // We need to actually order moves now :)
    boost::scoped_ptr<Resistance> resist;
    bool with_ordering = m_move_ordering;
    bool with_resist = m_move_ordering & DfsMoveOrderFlags::WITH_RESIST;
    bool with_center = m_move_ordering & DfsMoveOrderFlags::FROM_CENTER;
    bool with_mustplay = m_move_ordering & DfsMoveOrderFlags::WITH_MUSTPLAY;
    
    if (with_resist && with_ordering)
    {
        resist.reset(new Resistance());
        resist->Evaluate(brd);
    }
    
    moves.clear();
    for (BitsetIterator it(mustplay); !found_win && it; ++it)
    {
        bool skip_this_move = false;
        double score = 0.0;

	// Skip losing moves found in DB/TT
        if (losingMoves.test(*it))
            continue;

        if (with_ordering) 
        {
            double mustplay_size = 0.0;
            double fromcenter = 0.0;
            double rscore = 0.0;
            double tiebreaker = 0.0;
            bool exact_score = false;
            bool winning_semi_exists = false;

            // Do mustplay move-ordering.  This entails playing each
            // move, computing the vcs, storing the mustplay size,
            // then undoing the move. This gives pretty good move
            // ordering: 7x7 is much slower without this method and
	    // 8x8 is no longer solvable. However, it is very expensive!
            // 
            // We try to reduce the number of PlayMove/UndoMove pairs
            // we perform by checking the VC/DB/TT here, instead of in
            // solve_state().  Any move leading to a VC/DB/TT hit is removed
            // from the mustplay and handled as it would be in
            // solve_state().
            if (with_mustplay)
	    {
                PlayMove(brd, *it, color);

                DfsData state;
                bitset_t proof;
		// no need to check DB/TT since did this above
		if (HandleTerminalNode(brd, other, state, proof))
		{
                    exact_score = true;

                    solution.stats.explored_states += 1;
                    solution.stats.minimal_explored++;
                    solution.stats.total_states += state.m_numStates;

                    if (!state.m_win)
		    {
                        found_win = true;
                        moves.clear();
                        
                        // this state plus the child winning state
                        // (which is a leaf).
                        solution.stats.minimal_explored = 2;
                        solution.proof = proof;
                        solution.moves_to_connection = state.m_numMoves + 1;
                        solution.pv.clear();
                        solution.pv.push_back(*it);
                    }
		    else
		    {
                        skip_this_move = true;
                        if (state.m_numMoves + 1 > solution.moves_to_connection)
			{
                            solution.moves_to_connection = state.m_numMoves + 1;
                            solution.pv.clear();
                            solution.pv.push_back(*it);
                        }
                        // will prune the mustplay with the proof below
                        proof_intersection &= proof;
                        proof_union |= proof;
                    }
                }
		else
		{
                    // Not a leaf node. 
                    // Do we force a mustplay on our opponent?
                    HexPoint edge1 = HexPointUtil::colorEdge1(color);
                    HexPoint edge2 = HexPointUtil::colorEdge2(color);
                    if (brd.Cons(color).Exists(edge1, edge2, VC::SEMI))
                        winning_semi_exists = true;
                    bitset_t mp = VCUtils::GetMustplay(brd, other);
                    mustplay_size = mp.count();
                } 
                
                UndoMove(brd, *it);
            } // end of mustplay move ordering

            // Perform move ordering 
            if (!exact_score)
	    {
                if (with_center)
		{
                    fromcenter 
                        += DfsSolverUtil::DistanceFromCenter(brd.Const(), *it);
                }
                if (with_resist)
		{
                    rscore = resist->Score(*it);
                    HexAssert(rscore < 100.0);
                }
                tiebreaker = (with_resist) ? 100.0 - rscore : fromcenter;
                
                if (winning_semi_exists)
                    score = 1000.0*mustplay_size + tiebreaker;
                else
                    score = 1000000.0*tiebreaker;
            }
        }
        if (!skip_this_move) 
            moves.push_back(HexMoveValue(*it, score));
    }

    /** @note sort() is not stable, so multiple runs can produce
        different move orders in the same state unless stable_sort() is
        used. */
    stable_sort(moves.begin(), moves.end());
    HexAssert(!found_win || moves.size()==1);

    // For a win: nothing to do
    if (found_win)
        LogFine() << "Found winning move; aborted ordering.\n";
    // For a loss: recompute mustplay because backed-up ice info could
    // shrink it. Then prune with the intersection of all losing
    // proofs, and add in the union of all losing proofs to the
    // current proof.
    else
    {
        if (m_backup_ice_info)
	{
            bitset_t new_initial_proof 
                = DfsSolverUtil::InitialProof(brd, color);
            bitset_t new_mustplay = 
                DfsSolverUtil::MovesToConsider(brd, color, new_initial_proof);
            HexAssert(BitsetUtil::IsSubsetOf(new_mustplay, mustplay));
            
            if (new_mustplay.count() < mustplay.count())
	    {
                LogFine() << "Pruned mustplay with backing-up info."
			  << brd.Write(mustplay)
			  << brd.Write(new_mustplay) << '\n';
                mustplay = new_mustplay;
                solution.proof = new_initial_proof;
            }
        }
        mustplay &= proof_intersection;
        solution.proof |= proof_union;
    }
    return found_win;
}

//----------------------------------------------------------------------------

std::string DfsSolver::Histogram::Dump()
{
    std::ostringstream os;
    os << std::endl;
    os << "Histogram" << std::endl;
    os << "                         States             ";
    os << "                      Branch Info                    ";
    os << "                                      TT/DB                ";
    os << std::endl;
    os << std::setw(3) << "#" << " "
       << std::setw(12) << "Terminal"
       << std::setw(12) << "Internal"
       << std::setw(12) << "Int. Win"
       << std::setw(12) << "Win Pct"

       << std::setw(12) << "Sz Winning"
       << std::setw(12) << "Sz Losing"
        
       << std::setw(12) << "To Win"
       << std::setw(12) << "Mustplay"
       << std::setw(12) << "U/Losing"
       << std::setw(12) << "Cost"
       << std::setw(12) << "Hits"
       << std::setw(12) << "Pct"
       << std::endl;

    for (int p=0; p<FIRST_INVALID; ++p) {
        if (!states[p] && !terminal[p]) 
            continue;

        double moves_to_find_winning = winning[p] ?
            (double)branches[p]/winning[p] : 0;
        
        double avg_states_under_losing = (branches[p]-winning[p])?
            ((double)states_under_losing[p]/(branches[p]-winning[p])):0;

        os << std::setw(3) << p << ":"
            
           << std::setw(12) << terminal[p] 

           << std::setw(12) << states[p]

           << std::setw(12) << winning[p]
            
           << std::setw(12) << std::fixed << std::setprecision(3) 
           << ((states[p])?((double)winning[p]*100.0/states[p]):0)

           << std::setw(12) << std::fixed << std::setprecision(1) 
           << ((winning[p]) 
               ? ((double)size_of_winning_states[p] / winning[p])
               : 0)

           << std::setw(12) << std::fixed << std::setprecision(1) 
           << ((states[p] - winning[p]) 
               ? ((double)(size_of_losing_states[p] 
                           / (states[p] - winning[p])))
               :0)
           << std::setw(12) << std::fixed << std::setprecision(4)
           << moves_to_find_winning
            
           << std::setw(12) << std::fixed << std::setprecision(2)
           << ((winning[p]) ? ((double)mustplay[p] / winning[p]) : 0)
            
           << std::setw(12) << std::fixed << std::setprecision(1)
           << avg_states_under_losing

           << std::setw(12) << std::fixed << std::setprecision(1)
           << fabs((moves_to_find_winning - 1.0) 
                   * avg_states_under_losing * winning[p])

           << std::setw(12) << tthits[p]
            
           << std::setw(12) << std::fixed << std::setprecision(3)
           << ((states[p]) ? ((double)tthits[p] * 100.0 / states[p]) : 0)
            
           << std::endl;
    }
    return os.str();
}

void DfsSolver::DumpStats(const DfsSolutionSet& solution) const
{
    double total_time = m_end_time - m_start_time;

    LogInfo() << '\n'
	      << "########################################" << '\n'
	      << "         Played: " << m_statistics.played << '\n'
	      << "         Pruned: " << solution.stats.pruned << '\n'
	      << "   Total States: " << solution.stats.total_states << '\n'
	      << "Explored States: " << solution.stats.explored_states 
	      << " (" << solution.stats.minimal_explored << ")" << '\n'
	      << "Expanded States: " << solution.stats.expanded_states << '\n'
	      << " Decompositions: " << solution.stats.decompositions << '\n'
	      << "    Decomps won: " 
	      << solution.stats.decompositions_won << '\n'
	      << "  Shrunk Proofs: " << solution.stats.shrunk << '\n'
	      << "    Avg. Shrink: " 
	      << ((double)solution.stats.cells_removed 
		  / solution.stats.shrunk) << '\n'
	      << "  Branch Factor: " 
	      << ((double)solution.stats.moves_to_consider
		  / solution.stats.expanded_states) << '\n'
	      << "    To Find Win: "
	      << ((double)solution.stats.branches_to_win
		  / solution.stats.winning_expanded) << '\n'
	      << "########################################" << '\n';

    if (m_positions->Database()) 
        LogInfo() << m_positions->Database()->GetStatistics().Write() << '\n';
   
    if (m_positions->HashTable()) 
    {
        LogInfo() << m_positions->HashTable()->Stats()
		  << "########################################" << '\n';
    }
    
    LogInfo() << "States/sec: " 
	      << (solution.stats.explored_states/total_time) << '\n'
	      << "Played/sec: " << (m_statistics.played/total_time) << '\n'
	      << "Total Time: " << Time::Formatted(total_time) << '\n'
	      << "VC in " << solution.moves_to_connection << " moves\n"
	      << "PV: " << HexPointUtil::ToString(solution.pv) << '\n'
	      << m_histogram.Dump() << '\n';
}

//----------------------------------------------------------------------------

int DfsSolverUtil::DistanceFromCenter(const ConstBoard& brd, HexPoint cell)
{
    // Odd boards are easy
    if ((brd.Width() & 1) && (brd.Height() & 1))
        return brd.Distance(BoardUtils::CenterPoint(brd), cell);

    // Make sure we spiral nicely on boards with even
    // dimensions. Take the sum of the distance between
    // the two center cells on the main diagonal.
    return brd.Distance(BoardUtils::CenterPointRight(brd), cell)
        +  brd.Distance(BoardUtils::CenterPointLeft(brd), cell);
}

bool DfsSolverUtil::isWinningState(const HexBoard& brd, HexColor color, 
                                   bitset_t& proof)
{
    if (brd.GetGroups().IsGameOver()) 
    {
        if (brd.GetGroups().GetWinner() == color) 
        {
            // Surprisingly, this situation *can* happen: opponent plays a
            // move in the mustplay causing a sequence of presimplicial-pairs 
            // and captures that result in a win. 
            LogFine() << "#### Solid chain win ####\n";
            proof = brd.GetState().GetColor(color) - brd.GetDead();
            return true;
        }
    } 
    else 
    {
        VC v;
        if (brd.Cons(color).SmallestVC(HexPointUtil::colorEdge1(color), 
                                       HexPointUtil::colorEdge2(color), 
                                       VC::SEMI, v)) 
        {
            LogFine() << "VC win.\n";
            proof = (v.carrier() | brd.GetState().GetColor(color)) 
                - brd.GetDead();
            return true;
        } 
    }
    return false;
}

bool DfsSolverUtil::isLosingState(const HexBoard& brd, HexColor color, 
                               bitset_t& proof)
{
    HexColor other = !color;
    if (brd.GetGroups().IsGameOver()) 
    {
        if (brd.GetGroups().GetWinner() == other) 
        {
            // This occurs very rarely, but definetly cannot be ruled out.
            LogFine() << "#### Solid chain loss ####\n";
            proof = brd.GetState().GetColor(other) - brd.GetDead();
            return true;
        } 
    } 
    else 
    {
        VC vc;
        HexPoint otheredge1 = HexPointUtil::colorEdge1(other);
        HexPoint otheredge2 = HexPointUtil::colorEdge2(other);
        if (brd.Cons(other).SmallestVC(otheredge1, otheredge2, VC::FULL, vc)) 
        {
            LogFine() << "VC loss.\n";
            proof = (vc.carrier() | brd.GetState().GetColor(other)) 
                - brd.GetDead();
            return true;
        } 
    }
    return false;
}

bitset_t DfsSolverUtil::MovesToConsider(const HexBoard& brd, HexColor color,
                                     bitset_t& proof)
{
    bitset_t ret = VCUtils::GetMustplay(brd, color);
    if (ret.none()) 
        throw BenzeneException()<< "DfsSolverUtil::MovesToConsider: "
                                << "EMPTY MUSTPLAY!: " << brd << '\n';
    
    // Take out the dead, dominated, reversible, and vulnerable
    const InferiorCells& inf = brd.GetInferiorCells();
    ret = ret - inf.Dead();
    ret = ret - inf.Dominated();
    ret = ret - inf.Reversible();
    ret = ret - inf.Vulnerable();

    /** Must add reversable reversers to proof.

        The carriers do NOT need to be included in the proof, since
        they are captured by the (losing) player, not his opponent
        (for whom we are building the proof set).
        
        @todo Currently, we just add the first reverser: we should see
        if any reverser is already in the proof, since then we wouldn't
        need to add one.
    */
    for (BitsetIterator p(inf.Reversible()); p; ++p) 
    {
        const std::set<HexPoint>& reversers = inf.Reversers(*p);
        proof.set(*reversers.begin());
    }
    
    /** Must add vulnerable killers (and their carriers) to proof.
        
        @todo Currently, we just add the first killer: we should see
        if any killer is already in the proof, since then we wouldn't
        need to add one.
    */
    for (BitsetIterator p(inf.Vulnerable()); p; ++p) 
    {
        const std::set<VulnerableKiller>& killers = inf.Killers(*p);
        proof.set((*killers.begin()).killer());
        proof |= ((*killers.begin()).carrier());
    }
    return ret;
}

bitset_t DfsSolverUtil::MustplayCarrier(const HexBoard& brd, HexColor color)
{
    HexPoint edge1 = HexPointUtil::colorEdge1(!color);
    HexPoint edge2 = HexPointUtil::colorEdge2(!color);
    const VCList& lst = brd.Cons(!color).GetList(VC::SEMI, edge1, edge2);
    return (brd.Builder().Parameters().use_greedy_union)
        ? lst.getGreedyUnion()
        : lst.getUnion();
}

bitset_t DfsSolverUtil::InitialProof(const HexBoard& brd, HexColor color)
{
    LogFine() << "mustplay-carrier:"
	      << brd.Write(MustplayCarrier(brd, color)) << '\n';

    bitset_t proof = 
        (MustplayCarrier(brd, color) | brd.GetState().GetColor(!color)) 
        - brd.GetDead();

    LogFine() << "Initial mustplay-carrier:" << brd.Write(proof) << '\n';
    if ((proof & brd.GetState().GetColor(color)).any())
        throw BenzeneException() << "Initial mustplay hits toPlay's stones!"
                                 << brd << '\n' << brd.Write(proof) << '\n';
    return proof;
}

//----------------------------------------------------------------------------
