//----------------------------------------------------------------------------
// $Id: SolverCheck.cpp 1994 2009-04-06 00:57:12Z broderic $
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "SgTimer.h"

#include <boost/scoped_ptr.hpp>

#include "Connections.hpp"
#include "PlayerUtils.hpp"
#include "Solver.hpp"
#include "SolverCheck.hpp"
#include "Time.hpp"

//----------------------------------------------------------------------------

SolverCheck::SolverCheck(BenzenePlayer* player)
    : BenzenePlayerFunctionality(player),
      m_enabled(false),
      m_threshold(15),
      m_timelimit(15.0)
{
}

SolverCheck::~SolverCheck()
{
}

HexPoint SolverCheck::pre_search(HexBoard& brd, const Game& game_state,
				 HexColor color, bitset_t& consider,
				 double time_remaining, double& score)
{
    if (m_enabled && m_threshold < (int)game_state.History().size())
    {
        LogInfo() << "SolverCheck: Trying to solve in " 
		 << FormattedTime(m_timelimit) << "." << '\n';

        /** @todo Copy the board's settings! */
	boost::scoped_ptr<HexBoard> bd(new HexBoard(brd.width(), 
						    brd.height(),
						    brd.ICE(),
                                                brd.Builder().Parameters()));
        bd->startNewGame();
        bd->setColor(BLACK, brd.getBlack());
        bd->setColor(WHITE, brd.getWhite());
	bd->setPlayed(brd.getPlayed());
        int sf = brd.Cons(BLACK).SoftLimit(VC::FULL);
        int ss = brd.Cons(BLACK).SoftLimit(VC::SEMI);
        bd->Cons(BLACK).SetSoftLimit(VC::FULL, sf);
        bd->Cons(BLACK).SetSoftLimit(VC::SEMI, ss);
        bd->Cons(WHITE).SetSoftLimit(VC::FULL, sf);
        bd->Cons(WHITE).SetSoftLimit(VC::SEMI, ss);
	
        SgTimer timer;
	Solver solver;
	Solver::Result result;
	Solver::SolutionSet solution;

	timer.Start();
	result = solver.Solve(*bd, color, solution, 
			      Solver::NO_DEPTH_LIMIT, m_timelimit);
        timer.Stop();
      
        if (result == Solver::WIN && !solution.pv.empty()) {

	  LogInfo() << "******* FOUND WIN!!! ******" 
		   << '\n' 
		   << "PV: " << HexPointUtil::ToPointListString(solution.pv)
		   << '\n'
		   << "Elapsed: " << timer.GetTime()
		   << '\n';
	  return solution.pv[0];

        } else if (result == Solver::LOSS) {

	  LogInfo() << "** Found loss!! **" << '\n';

	}

	LogInfo() << "No win found." << '\n';
        time_remaining -= timer.GetTime();
    }

    return m_player->pre_search(brd, game_state, color, consider,
				time_remaining, score);
}

//----------------------------------------------------------------------------
