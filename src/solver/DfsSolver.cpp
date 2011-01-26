//----------------------------------------------------------------------------
/** @file DfsSolver.cpp
 
    @todo Finish converting over to using HexStates.
 */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "SgWrite.h"

#include "BitsetIterator.hpp"
#include "BoardUtil.hpp"
#include "Decompositions.hpp"
#include "DfsSolver.hpp"
#include "EndgameUtil.hpp"
#include "GraphUtil.hpp"
#include "HexBoard.hpp"
#include "ProofUtil.hpp"
#include "Resistance.hpp"
#include "VCSet.hpp"
#include "VCUtil.hpp"

#include <boost/scoped_ptr.hpp>

using namespace benzene;

//----------------------------------------------------------------------------

/** Current version of dfs database. 
    Update this if DfsData is changes to prevent old out-of-data
    databases from being loaded. */
const std::string DfsDB::DFS_DB_VERSION("BENZENE_DFS_DB_VER_0001");

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

HexColor DfsSolver::Solve(const HexState& state, HexBoard& brd,
                          DfsSolutionSet& solution, DfsStates& positions,
                          int depthLimit, double timeLimit)
{
    m_positions = &positions;
    m_depthLimit = depthLimit;
    m_timeLimit = timeLimit;
    
    m_aborted = false;
    m_timer.Start();
    m_histogram = DfsHistogram();
    m_last_histogram_dump = 0;
    m_statistics = GlobalStatistics();
    m_state.reset(new HexState(state));
    m_workBrd = &brd;

    // DfsSolver currently cannot handle permanently inferior cells.
    if (m_workBrd->ICE().FindPermanentlyInferior())
        throw BenzeneException("Permanently Inferior not supported "
                               "in DfsSolver!");
    // Check if state is already solved
    DfsData data;
    bool win = false;
    HexColor toPlay = state.ToPlay();
    if (CheckTransposition(data))
    {
        LogInfo() << "DfsSolver: Found cached result!\n";
        win = data.m_win;
        solution.m_numMoves = data.m_numMoves;
        solution.pv.clear();
        SolverDBUtil::GetVariation(*m_state, positions, solution.pv);
        solution.proof = ProofUtil::MaximumProofSet(brd, data.m_win ? 
                                                    toPlay : !toPlay);
    }
    else
    {
        m_workBrd->ComputeAll(toPlay);
        m_completed.resize(BITSETSIZE);
        PointSequence variation;
        win = SolveState(variation, solution);
    }
    solution.proof &= m_state->Position().GetEmpty();
    m_timer.Stop();
    if (m_aborted) 
        return EMPTY;
    return win ? toPlay : !toPlay;
}

//----------------------------------------------------------------------------

bool DfsSolver::CheckTransposition(DfsData& data) const
{
    return m_positions->Get(*m_state, data);
}

void DfsSolver::StoreState(const DfsData& data, const bitset_t& proof)
{
    m_positions->Put(*m_state, data);
    const SolverDBParameters& param = m_positions->Parameters();
    if (m_state->Position().NumStones() <= param.m_transStones)
    {
        HexColor toPlay = m_state->ToPlay();
        if (param.m_useProofTranspositions)
            ProofUtil::StoreTranspositions(*m_positions, data, 
                                           *m_state, proof, 
                                           data.m_win ? toPlay : !toPlay);
        if (param.m_useFlippedStates)
            ProofUtil::StoreFlippedStates(*m_positions, data,
                                          *m_state, proof,
                                          data.m_win ? toPlay : !toPlay);
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
        else if ((m_timeLimit > 0) && (m_timer.GetTime() > m_timeLimit))
        {
            m_aborted = true;
            LogInfo() << "DfsSolver::CheckAbort(): Timelimit!\n";
        }
    }
    return m_aborted;
}

/** Returns true if node is terminal. Fills in data if terminal.
    Data's bestmove field is not specified here.*/
bool DfsSolver::HandleTerminalNode(DfsData& data, bitset_t& proof) const
{
    std::size_t numstones = m_state->Position().NumStones();
    if (EndgameUtil::IsWonGame(*m_workBrd, m_state->ToPlay(), proof)) 
    {
        data.m_win = true;
        data.m_numMoves = 0;
        data.m_numStates = 1;
        m_histogram.terminal[numstones]++;
        return true;
    } 
    else if (EndgameUtil::IsLostGame(*m_workBrd, m_state->ToPlay(), proof)) 
    {
        data.m_win = false;
        data.m_numMoves = 0;
        data.m_numStates = 1;
        m_histogram.terminal[numstones]++;
        return true;
    } 
    return false;
}

/** Returns true if current data is a terminal node (win/loss), or a
    DB/TT hit. If so, info is stored in data. */
bool DfsSolver::HandleLeafNode(DfsData& data, bitset_t& proof) const
{
    if (HandleTerminalNode(data, proof))
        return true;
    if (CheckTransposition(data))
    {
        HexColor color = m_state->ToPlay();
        proof = ProofUtil::MaximumProofSet(*m_workBrd, 
                                           data.m_win ? color : !color);
    }
    return false;
}

//----------------------------------------------------------------------------

/** Solves the current state. Handles decompositions if option is
    turned on. */
bool DfsSolver::SolveState(PointSequence& variation, DfsSolutionSet& solution)
{
    if (CheckAbort()) 
        return false;

    // Check for VC/DB/TT states
    {
        DfsData data;
        bitset_t proof;
        if (HandleLeafNode(data, proof)) 
        {
            solution.pv.clear();
            solution.m_numMoves = data.m_numMoves;
            solution.proof = proof;
            solution.stats.explored_states = 1;
            solution.stats.minimal_explored = 1;
            solution.stats.total_states += data.m_numStates;
            return data.m_win;
        }
    }

    // Solve decompositions if they exist, otherwise solve the data
    // normally.
    bool winning_state = false;
    {
        HexColor color = m_state->ToPlay();
        HexPoint group;
        if (m_use_decompositions
            && Decompositions::FindSplitting(*m_workBrd, !color, group))
        {
            winning_state = SolveDecomposition(variation, solution, group);
        } 
        else 
        {
            winning_state = SolveInteriorState(variation, solution);
        }
    }

    // Shrink, verify, and store proof in DB/TT.
    HandleProof(variation, winning_state, solution);

    // Dump histogram every 1M moves
    if ((m_statistics.played / 1000000) > (m_last_histogram_dump)) 
    {
        LogInfo() << m_histogram.Write() << '\n';
        m_last_histogram_dump = m_statistics.played / 1000000;
    }
    return winning_state;
}

/** Solves each side of the decompsosition; combines proofs if
    necessary. */
bool DfsSolver::SolveDecomposition(PointSequence& variation,
                                   DfsSolutionSet& solution, HexPoint group)
{
    HexColor color = m_state->ToPlay();
    solution.stats.decompositions++;

    // Compute the carriers for each side 
    PointToBitset nbs;
    GraphUtil::ComputeDigraph(m_workBrd->GetGroups(), !color, nbs);
    bitset_t stopset = nbs[group];

    bitset_t carrier[2];
    carrier[0] = 
        GraphUtil::BFS(HexPointUtil::colorEdge1(!color), nbs, stopset);
    carrier[1] = 
        GraphUtil::BFS(HexPointUtil::colorEdge2(!color), nbs, stopset);

    if ((carrier[0] & carrier[1]).any())
        throw BenzeneException()
            << "DfsSolver::SolveDecomposition:\n"
            << "Side0:" << m_workBrd->Write(carrier[0]) << '\n'
            << "Side1:" << m_workBrd->Write(carrier[1]) << '\n';
        
    DfsData data;
    DfsSolutionSet dsolution[2];
    for (int s = 0; s < 2; ++s) 
    {
        bool win = false;
        m_workBrd->PlayStones(!color, 
                              carrier[s^1] & m_workBrd->Const().GetCells(), 
                              color);

        bitset_t proof;
        if (HandleTerminalNode(data, proof)) 
        {
            win = data.m_win;
            dsolution[s].proof = proof;
            dsolution[s].m_numMoves = data.m_numMoves;
            dsolution[s].pv.clear();
            dsolution[s].stats.expanded_states = 0;
            dsolution[s].stats.explored_states = 1;
            dsolution[s].stats.minimal_explored = 1;
            dsolution[s].stats.total_states = 1;
        } 
        else 
            win = SolveInteriorState(variation, dsolution[s]);

        m_workBrd->UndoMove();

        if (win) 
        {
            solution.pv = dsolution[s].pv;
            solution.proof = dsolution[s].proof;
            solution.m_numMoves = dsolution[s].m_numMoves;
            solution.stats += dsolution[s].stats;
            solution.stats.decompositions_won++;
            return true;
        } 
    }
        
    // Combine the two losing proofs
    solution.pv = dsolution[0].pv;
    solution.m_numMoves = dsolution[0].m_numMoves + dsolution[1].m_numMoves;
    solution.pv.insert(solution.pv.end(), dsolution[1].pv.begin(),
                       dsolution[1].pv.end());
    
    solution.proof = (dsolution[0].proof & carrier[0]) 
        | (dsolution[1].proof & carrier[1]) 
        | m_workBrd->GetPosition().GetColor(!color);
    solution.proof = solution.proof - m_workBrd->GetDead();
    return false;
}

/** Does the recursive mustplay search. */
bool DfsSolver::SolveInteriorState(PointSequence& variation,
                                   DfsSolutionSet& solution)
{
    HexColor color = m_state->ToPlay();
    std::size_t numStones = m_state->Position().NumStones();
    // Set initial proof for this state to be the union of all
    // opponent winning semis.  We need to do this because we use the
    // semis to restrict the search (ie, the mustplay).
    // Proof will also include all opponent stones. 
    //
    // Basically, we are assuming the opponent will win from this state;
    // if we win instead, we use the proof generated from that state,
    // not this one. 
    solution.proof = ProofUtil::InitialProofForOpponent(*m_workBrd, color);
    bitset_t mustplay = EndgameUtil::MovesToConsider(*m_workBrd, color);
    BenzeneAssert(mustplay.any());

    if (m_use_guifx && variation.size() == m_update_depth)
    {
        std::ostringstream os;
        os << "gogui-gfx:\n";
        os << "solver\n";
        os << "VAR";
        HexColor toplay = (variation.size() & 1) ? !color : color;
        for (std::size_t i = 0; i < variation.size(); ++i) 
        {
            os << ' ' << (toplay == BLACK ? 'B' : 'W');
            os << ' ' << variation[i];
            toplay = !toplay;
        }
        os << '\n';
        os << "LABEL ";
        const InferiorCells& inf = m_workBrd->GetInferiorCells();
        os << inf.GuiOutput();
        os << BoardUtil::GuiDumpOutsideConsiderSet(m_workBrd->GetPosition(), 
                                                    mustplay, inf.All());
        os << '\n';
        os << "TEXT";
        for (std::size_t i = 0; i < variation.size(); ++i) 
            os << ' ' << m_completed[i].first << '/' << m_completed[i].second;
        os << '\n';
        os << '\n';
        std::cout << os.str();
        std::cout.flush();
    } 

    bitset_t original_mustplay = mustplay;
    solution.stats.total_states = 1;
    solution.stats.explored_states = 1;
    solution.stats.minimal_explored = 1;
    solution.stats.expanded_states = 1;
    solution.stats.moves_to_consider = mustplay.count();
    m_histogram.states[numStones]++;

    // Order moves in the mustplay.
    //
    // NOTE: If we want to find all winning moves then we need to stop
    // OrderMoves() from aborting on a win.
    //
    // NOTE: OrderMoves() will handle VC/DB/TT hits, and remove them
    // from consideration. It is possible that there are no moves, in
    // which case we fall through the loop below with no problem (the
    // state is a loss).
    solution.m_numMoves = -1;
    std::vector<HexMoveValue> moves;
    bool winning_state = OrderMoves(mustplay, solution, moves);

    //----------------------------------------------------------------------
    // Expand all moves in mustplay that were not leaf states.
    //----------------------------------------------------------------------
    std::size_t states_under_losing = 0;

    for (unsigned index = 0; 
         !winning_state && index < moves.size(); 
         ++index) 
    {
        HexPoint cell = moves[index].point();
        m_completed[variation.size()] = std::make_pair(index, moves.size());
        if (!mustplay.test(cell)) 
        {
            solution.stats.pruned++;
            continue;
        }

        DfsSolutionSet child;
        PlayMove(cell);
        variation.push_back(cell);
        bool win = !SolveState(variation, child);
        variation.pop_back();
        UndoMove(cell);
        solution.stats += child.stats;

        if (win) 
        {
            // Win: copy proof over, copy pv, abort!
            winning_state = true;
            solution.proof = child.proof;
            solution.SetPV(cell, child.pv);
            solution.m_numMoves = child.m_numMoves + 1;
            solution.stats.winning_expanded++;
            solution.stats.minimal_explored = child.stats.minimal_explored + 1;
            solution.stats.branches_to_win += index + 1;

            m_histogram.winning[numStones]++;
            m_histogram.size_of_winning_states[numStones] 
                += child.stats.explored_states;
            m_histogram.branches[numStones] += index + 1;
            m_histogram.states_under_losing[numStones] += states_under_losing;
            m_histogram.mustplay[numStones] += original_mustplay.count();

	    BenzeneAssert(solution.m_numMoves != -1);	    
        } 
        else 
        {
            // Loss: add returned proof to current proof. Prune
            // mustplay by proof.  Maintain PV to longest loss.
            mustplay &= child.proof;
            solution.proof |= child.proof;
            states_under_losing += child.stats.explored_states;

            m_histogram.size_of_losing_states[numStones] 
                += child.stats.explored_states;

            if (child.m_numMoves + 1 > solution.m_numMoves) 
            {
                solution.m_numMoves = child.m_numMoves + 1;
                solution.SetPV(cell, child.pv);
            }
	    BenzeneAssert(solution.m_numMoves != -1);
        }
    }
    BenzeneAssert(solution.m_numMoves != -1);
    return winning_state;
}

/** Shrinks/verifies proof then stores it. */
void DfsSolver::HandleProof(const PointSequence& variation,
                            bool winning_state, DfsSolutionSet& solution)
{
    if (m_aborted)
        return;
    HexColor color = m_state->ToPlay();
    HexColor winner = (winning_state) ? color : !color;
    HexColor loser = !winner;
    // Verify loser's stones do not intersect proof
    if ((m_workBrd->GetPosition().GetColor(loser) & solution.proof).any()) 
        throw BenzeneException()
            << "DfsSolver::HandleProof:\n"
            << color << " to play.\n"
            << loser << " loses.\n"
            << "Losing stones hit proof:\n"
            << m_workBrd->Write(solution.proof) << '\n'
            << *m_workBrd << '\n'
            << "PV: " << HexPointUtil::ToString(variation) << '\n';
    // Verify dead cells do not intersect proof
    if ((m_workBrd->GetDead() & solution.proof).any()) 
        throw BenzeneException()
            << "DfsSolver::HandleProof:\n"
            << color << " to play.\n"
            << loser << " loses.\n"
            << "Dead cells hit proof:\n"
            << m_workBrd->Write(solution.proof) << '\n'
            << *m_workBrd << '\n'
            << "PV: " << HexPointUtil::ToString(variation) << '\n';
    // Shrink proof
    bitset_t old_proof = solution.proof;
    if (m_shrink_proofs) 
    {
        ProofUtil::ShrinkProof(solution.proof, m_state->Position(), loser, 
                               m_workBrd->ICE());
        bitset_t pruned;
        pruned  = BoardUtil::ReachableOnBitset(m_workBrd->Const(), 
                                                solution.proof, 
                                                EMPTY_BITSET,
                                 HexPointUtil::colorEdge1(winner));
        pruned &= BoardUtil::ReachableOnBitset(m_workBrd->Const(), 
                                                solution.proof, 
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
    if (!BoardUtil::ConnectedOnBitset(m_workBrd->Const(), solution.proof, 
                                       HexPointUtil::colorEdge1(winner),
                                       HexPointUtil::colorEdge2(winner)))
        throw BenzeneException()
            << "DfsSolver::HandleProof:\n"
            << "Proof does not touch both edges!\n"
            << m_workBrd->Write(solution.proof) << '\n'
            << "Original proof:\n"
            << m_workBrd->Write(old_proof) << '\n'
            << *m_workBrd << '\n'
            << color << " to play.\n"
            << "PV: " << HexPointUtil::ToString(variation) << '\n';

    /** @todo HANDLE BEST MOVES PROPERLY! 
        This can only happen if the mustplay goes empty in an internal
        state that wasn't determined initially, or in a decomp state
        where the fillin causes a terminal state. 
     */
    if (solution.pv.empty())
        solution.pv.push_back(INVALID_POINT);

    StoreState(DfsData(winning_state, int(solution.stats.total_states), 
                       int(solution.m_numMoves), solution.pv[0]), 
               solution.proof);
}

//----------------------------------------------------------------------------

/** Plays the move; updates the board.  */
void DfsSolver::PlayMove(HexPoint cell) 
{
    m_statistics.played++;
    m_workBrd->PlayMove(m_state->ToPlay(), cell);
    m_state->PlayMove(cell);

}

/** Takes back the move played. */
void DfsSolver::UndoMove(HexPoint cell)
{
    m_state->UndoMove(cell);
    m_workBrd->UndoMove();
}

//----------------------------------------------------------------------------

/** Orders the moves in mustplay using several heuristics.  Aborts
    move ordering early if it finds a TT win: winning move is put to
    the front.

    Will shrink the mustplay if it encounters TT losses: losing moves
    are not added to the list of sorted moves.

    Returns true if it found a TT win, false otherwise.
*/
bool DfsSolver::OrderMoves(bitset_t& mustplay, DfsSolutionSet& solution,
                           std::vector<HexMoveValue>& moves)
{        
    LogFine() << "OrderMoves\n";
    HexColor color = m_state->ToPlay();
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
    for (BitsetIterator it(mustplay); !found_win && it; ++it)
    {
	m_workBrd->GetPosition().PlayMove(color, *it);
	m_state->PlayMove(*it);

	DfsData data;
	if (CheckTransposition(data))
	{
	    solution.stats.explored_states += 1;
	    solution.stats.minimal_explored++;
	    solution.stats.total_states += data.m_numStates;

	    if (!data.m_win)
	    {
		found_win = true;
		moves.clear();
		moves.push_back(HexMoveValue(*it, 0));
		
		// This state plus the child winning state (which is a leaf).
		solution.stats.minimal_explored = 2;
                solution.proof = ProofUtil::MaximumProofSet(*m_workBrd, color);
		solution.m_numMoves = data.m_numMoves + 1;
                solution.SetPV(*it);
	    } 
	    else 
	    {
		// Prune this losing move from the mustplay
		losingMoves.set(*it);
		if (data.m_numMoves + 1 > solution.m_numMoves) 
                {
		    solution.m_numMoves = data.m_numMoves + 1;
                    solution.SetPV(*it);
		}
		// Will prune the mustplay later on with the proof
                bitset_t proof = ProofUtil::MaximumProofSet(*m_workBrd, !color);
		proof_intersection &= proof;
		proof_union |= proof;
	    }
	}
	m_workBrd->GetPosition().UndoMove(*it);
	m_state->UndoMove(*it);
    }
    
    if (found_win)
    {
	BenzeneAssert(moves.size() == 1);
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
        resist->Evaluate(*m_workBrd);
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
            if (with_mustplay)
	    {
                PlayMove(*it);
                
                DfsData data;
                bitset_t proof;
		// No need to check DB/TT since did this above
		if (HandleTerminalNode(data, proof))
		{
                    exact_score = true;
                    solution.stats.minimal_explored++;
                    solution.stats.explored_states++;
                    solution.stats.total_states += data.m_numStates;
                    if (!data.m_win)
		    {
                        found_win = true;
                        moves.clear();
                        
                        // This state plus the child winning state
                        // (which is a leaf).
                        solution.stats.minimal_explored = 2;
                        solution.proof = proof;
                        solution.m_numMoves = data.m_numMoves + 1;
                        solution.SetPV(*it);
                    }
		    else
		    {
                        skip_this_move = true;
                        if (data.m_numMoves + 1 > solution.m_numMoves)
			{
                            solution.m_numMoves = data.m_numMoves + 1;
                            solution.SetPV(*it);
                        }
                        // Will prune the mustplay with the proof below
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
                    if (m_workBrd->Cons(color).Exists(edge1, edge2, VC::SEMI))
                        winning_semi_exists = true;
                    bitset_t mp = VCUtil::GetMustplay(*m_workBrd, other);
                    mustplay_size = static_cast<double>(mp.count());
                } 
                
                UndoMove(*it);
            } // end of mustplay move ordering

            // Perform move ordering 
            if (!exact_score)
	    {
                if (with_center)
		{
                    fromcenter 
                        += DfsSolverUtil::DistanceFromCenter(m_workBrd->Const(), *it);
                }
                if (with_resist)
		{
                    rscore = resist->Score(*it);
                    BenzeneAssert(rscore < 100.0);
                }
                tiebreaker = (with_resist) ? 100.0 - rscore : fromcenter;
                
                if (winning_semi_exists)
                    score = 1000.0 * mustplay_size + tiebreaker;
                else
                    score = 1000000.0 * tiebreaker;
            }
        }
        if (!skip_this_move) 
            moves.push_back(HexMoveValue(*it, score));
    }
    BenzeneAssert(!found_win || moves.size() == 1);
    // NOTE: sort() is not stable, so multiple runs can produce
    // different move orders in the same state unless stable_sort() is
    // used.
    stable_sort(moves.begin(), moves.end());

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
                = ProofUtil::InitialProofForOpponent(*m_workBrd, color);
            bitset_t new_mustplay 
                = EndgameUtil::MovesToConsider(*m_workBrd, color);
            BenzeneAssert(BitsetUtil::IsSubsetOf(new_mustplay, mustplay));
            
            if (new_mustplay.count() < mustplay.count())
	    {
                LogFine() << "Pruned mustplay with backing-up info."
			  << m_workBrd->Write(mustplay)
			  << m_workBrd->Write(new_mustplay) << '\n';
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

std::string DfsHistogram::Write()
{
    std::ostringstream os;
    os << '\n'
       << "Histogram\n"
       << "                         States             "
       << "                      Branch Info                    "
       << "                                      TT/DB                "
       << '\n'
       << std::setw(3) << "#" << " "
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
       << '\n';

    for (int p = 0; p < FIRST_INVALID; ++p) 
    {
        if (!states[p] && !terminal[p]) 
            continue;
        double moves_to_find_winning = winning[p] 
            ? double(branches[p]) / double(winning[p]) : 0;
        double avg_states_under_losing = (branches[p] - winning[p])
            ?(double(states_under_losing[p]) 
              / double(branches[p] - winning[p])):0;
        os << std::setw(3) << p << ":"
           << std::setw(12) << terminal[p] 
           << std::setw(12) << states[p]
           << std::setw(12) << winning[p]
           << std::setw(12) << std::fixed << std::setprecision(3) 
           << (states[p] 
               ? double(winning[p]) * 100.0 / double(states[p])
               : 0)
           << std::setw(12) << std::fixed << std::setprecision(1) 
           << (winning[p]
               ? double(size_of_winning_states[p]) / double(winning[p])
               : 0)
           << std::setw(12) << std::fixed << std::setprecision(1) 
           << (states[p] - winning[p] 
               ? double(size_of_losing_states[p]) 
                 / double(states[p] - winning[p])
               : 0)
           << std::setw(12) << std::fixed << std::setprecision(4)
           << moves_to_find_winning
           << std::setw(12) << std::fixed << std::setprecision(2)
           << (winning[p] ? double(mustplay[p]) / double(winning[p]) : 0)
           << std::setw(12) << std::fixed << std::setprecision(1)
           << avg_states_under_losing
           << std::setw(12) << std::fixed << std::setprecision(1)
           << fabs((double(moves_to_find_winning) - 1.0) 
                   * double(avg_states_under_losing) * double(winning[p]))
           << std::setw(12) << tthits[p]
           << std::setw(12) << std::fixed << std::setprecision(3)
           << (states[p] ? double(tthits[p]) * 100.0 / double(states[p]) : 0)
           << '\n';
    }
    return os.str();
}

void DfsSolver::DumpStats(const DfsSolutionSet& solution) const
{
    const double total_time = m_timer.GetTime();
    std::ostringstream os;
    os << '\n'
       << SgWriteLabel("Played") << m_statistics.played << '\n'
       << SgWriteLabel("Pruned") << solution.stats.pruned << '\n'
       << SgWriteLabel("Total States") << solution.stats.total_states << '\n'
       << SgWriteLabel("Explored") << solution.stats.explored_states 
       << " (" << solution.stats.minimal_explored << ")" << '\n'
       << SgWriteLabel("Expanded") 
       << solution.stats.expanded_states << '\n'
       << SgWriteLabel("Decompositions") 
       << solution.stats.decompositions << '\n'
       << SgWriteLabel("Decomps Won")
       << solution.stats.decompositions_won << '\n'
       << SgWriteLabel("Shrunk Proofs") << solution.stats.shrunk << '\n'
       << SgWriteLabel("Avg. Shrink")
       << (double(solution.stats.cells_removed)
           / double(solution.stats.shrunk)) << '\n'
       << SgWriteLabel("Branch Factor")
       << (double(solution.stats.moves_to_consider)
           / double(solution.stats.expanded_states)) << '\n'
       << SgWriteLabel("To Find Win")
       << (double(solution.stats.branches_to_win)
           / double(solution.stats.winning_expanded)) << '\n'
       << SgWriteLabel("States/sec")
       << (double(solution.stats.explored_states) / total_time) << '\n'
       << SgWriteLabel("Played/sec") << 
        (double(m_statistics.played) / total_time) << '\n'
       << SgWriteLabel("Total Time") << total_time << "s\n"
       << SgWriteLabel("Moves to W/L") << solution.m_numMoves << " moves\n"
       << SgWriteLabel("PV") << HexPointUtil::ToString(solution.pv) << '\n';
    if (m_positions->Database()) 
        os << '\n' << m_positions->Database()->GetStatistics().Write() << '\n';
    if (m_positions->HashTable()) 
        os << '\n' << *m_positions->HashTable() << '\n';
    LogInfo() << os.str();
}

//----------------------------------------------------------------------------

int DfsSolverUtil::DistanceFromCenter(const ConstBoard& brd, HexPoint cell)
{
    // Odd boards are easy
    if ((brd.Width() & 1) && (brd.Height() & 1))
        return brd.Distance(BoardUtil::CenterPoint(brd), cell);
    // Make sure we spiral nicely on boards with even
    // dimensions. Take the sum of the distance between
    // the two center cells on the main diagonal.
    return brd.Distance(BoardUtil::CenterPointRight(brd), cell)
        +  brd.Distance(BoardUtil::CenterPointLeft(brd), cell);
}

//----------------------------------------------------------------------------
