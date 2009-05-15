//----------------------------------------------------------------------------
/** @file PerfectPlayer.cpp
 */
//----------------------------------------------------------------------------

#include "SgSystem.h"

#include "PerfectPlayer.hpp"
#include "BitsetIterator.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

PerfectPlayer::PerfectPlayer(Solver* solver)
    : BenzenePlayer(),
      m_solver(solver),
      m_db(0)
{
    LogFine() << "--- PerfectPlayer" << '\n';
}

PerfectPlayer::~PerfectPlayer()
{
}

//----------------------------------------------------------------------------

HexPoint PerfectPlayer::search(HexBoard& brd, 
                               const Game& game_state,
			       HexColor color,
                               const bitset_t& consider,
                               double max_time,
                               double& score)
{
    UNUSED(game_state);
    UNUSED(consider); 
    UNUSED(max_time); 

    bool need_to_run_solver = true;

    bitset_t proof;
    HexPoint move_to_play = INVALID_POINT;

    // check db 
    if (find_db_move(brd, color, move_to_play, proof, score)) 
        need_to_run_solver = false;

    // state not currently in db; solve it. 
    if (need_to_run_solver) {
       
        solve_new_state(brd, color, move_to_play, proof, score);

    }

    LogInfo() << brd.printBitset(proof) << '\n';
    if (HexEvalUtil::IsWin(score)) {
        LogInfo() << "Win in " << HexEvalUtil::PlyToWin(score) << "." << '\n';
    } else {
        LogInfo() << "Loss in " << HexEvalUtil::PlyToLoss(score) << "." << '\n';
    }

    return move_to_play;
}

bool PerfectPlayer::find_db_move(StoneBoard& brd, HexColor color, 
                                   HexPoint& move_to_play, bitset_t& proof, 
                                   double& score) const
{
    if (!m_db)
        return false;

    // bail if state doesn't exist in db
    SolvedState root_state;
    if (!m_db->get(brd, root_state)) {
        LogInfo() << "perfect: state not in db." << '\n';
        return false;
    }
    bool winning = root_state.win;
    proof = root_state.proof;

    LogInfo() << "perfect: state in db; finding best move..." << '\n';

    // check all children to find the shortest win/longest loss
    HexPoint best_win  = INVALID_POINT;
    HexPoint best_loss = INVALID_POINT;
    int win_length  = 999999;
    int loss_length =-999999;

    bool found_child = false;
    for (BitsetIterator p(brd.getEmpty()); p; ++p) {
        brd.playMove(color, *p);
        
        SolvedState state;
        if (m_db->get(brd, state)) {
            found_child = true;
            if (state.win) {
                if (state.nummoves > loss_length) {
                    best_loss = *p;
                    loss_length = state.nummoves;
                }
            } else {
                if (state.nummoves < win_length) {
                    best_win = *p;
                    win_length = state.nummoves;
                }
            }
        }
        
        brd.undoMove(*p);
    }
    
    // if no child (ie, a db leaf state), we have to solve it by hand
    if (!found_child) {
        LogInfo() << "perfect: db leaf." << '\n';
        return false;
    }

    if (winning) {
        HexAssert(best_win != INVALID_POINT);
        move_to_play = best_win;
        score = IMMEDIATE_WIN - win_length;
    } else {
        HexAssert(best_loss != INVALID_POINT);
        move_to_play = best_loss;
        score = IMMEDIATE_LOSS + loss_length;
    }

    return true;
}

void PerfectPlayer::solve_new_state(HexBoard& brd, HexColor color, 
                                    HexPoint& move_to_play, bitset_t& proof, 
                                    double& score) const
{
    LogInfo() << "perfect: state not in db; solving from scratch." << '\n';
        
    Solver::SolutionSet solution;
    HexEval result = Solver::UNKNOWN;

    /** @todo Make this an option? */
    m_solver->GetTT()->clear();
    
    // solve the state; try to use the db if possible
    if (m_db) 
    {
        int flags = m_solver->GetFlags();
        m_solver->SetFlags(flags | Solver::SOLVE_ROOT_AGAIN);
        result = m_solver->Solve(brd, color, *m_db, solution);
        m_solver->SetFlags(flags);
    } 
    else 
    {
        LogInfo() << "perfect: solving state without db..." << '\n';
        result = m_solver->Solve(brd, color, solution);
    }
    HexAssert(result != Solver::UNKNOWN);
        
    /** @todo Propagate these values up the tree. This is
        necessary because finding an alternate win can change the
        moves_to_connection value in parent states. This is
        somewhat cosmetic. */

    proof = solution.proof;
    HexAssert(!solution.pv.empty());
    move_to_play = solution.pv[0];
    
    if (solution.result) {
        score = IMMEDIATE_WIN - solution.moves_to_connection;
    } else {
        score = IMMEDIATE_LOSS + solution.moves_to_connection;
    }
}

//----------------------------------------------------------------------------
