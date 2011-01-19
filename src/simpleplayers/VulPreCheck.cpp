//----------------------------------------------------------------------------
/** VulPreCheck.cpp */
//----------------------------------------------------------------------------

#include "VulPreCheck.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

VulPreCheck::VulPreCheck()
    : m_killedOpptStones()
{
}

VulPreCheck::~VulPreCheck()
{
}

HexPoint VulPreCheck::KillLastMove(HexBoard& brd, const Game& game_state,
                                   HexColor color)
{
    LogWarning() << "Performing vulnerable pre-check...\n";
    if (!game_state.History().empty()) 
    {
	// Setup the board as it was prior to the opponent's last move.
	MoveSequence gh = game_state.History();
        StoneBoard b(brd.Width(), brd.Height());
	PatternState pastate(b);
	for (std::size_t i = 0; i + 1 < gh.size(); ++i) 
        {
	    HexPoint p = gh[i].Point();
	    HexColor c = gh[i].Color();
	    
	    /** If we've killed this opponent stone, give it to ourselves.
		This often helps to find more vulnerable opponent moves. 
            
                @todo Make this both colors (ie, dead) once
                PatternState supports stones of both colors.
            */
	    if (m_killedOpptStones.test(p)) 
            {
		BenzeneAssert(c == !color);
		c = !c;
	    }
	    b.PlayMove(c, p);
	}
	pastate.Update();
	LogWarning() << "Board before last move:" << b << '\n';
	
	// Check if last move played (by opponent) was vulnerable.
	HexPoint lastCell = gh.back().Point();
	bitset_t lastMoveOnly;
	lastMoveOnly.set(lastCell);
	LogWarning() << "Last move on this board:"
                     << b.Write(lastMoveOnly) << '\n';
	BenzeneAssert(gh.back().Color() == !color);
	InferiorCells inf;
	brd.ICE().FindVulnerable(pastate, !color, lastMoveOnly, inf);
	LogWarning() << "Inferior cells:" << inf.GuiOutput() << '\n';
	
	// If it was, simply return the killer.
	if (inf.Vulnerable().test(lastCell)) 
        {
	    LogWarning() << "Opponent's last move was vulnerable - killing it!"
                         << '\n';
	    std::set<VulnerableKiller> killers = inf.Killers(lastCell);
	    BenzeneAssert(!killers.empty());
	    
	    /** If opponent's last move can be made unconditionally dead,
		this is preferable since we can treat it as such in the
		future, thereby finding more opponent vulnerable cells. */
	    std::set<VulnerableKiller>::iterator i;
	    for (i = killers.begin(); i != killers.end(); ++i) 
            {
		if (i->carrier().none()) 
                {
		    m_killedOpptStones.set(lastCell);
		    return i->killer();
		}
	    }
	    // Otherwise, just kill it any which way.
	    return killers.begin()->killer();
	}
    }
    return INVALID_POINT;
}

//----------------------------------------------------------------------------
