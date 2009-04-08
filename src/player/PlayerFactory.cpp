//----------------------------------------------------------------------------
// $Id: PlayerFactory.cpp 1877 2009-01-29 00:57:27Z broderic $
//----------------------------------------------------------------------------

#include "PlayerFactory.hpp"
#include "BookCheck.hpp"
#include "EndgameCheck.hpp"
#include "HandBookCheck.hpp"
#include "LadderCheck.hpp"
#include "SolverCheck.hpp"
#include "SwapCheck.hpp"
#include "VulPreCheck.hpp"

/** @file

    Various factory methods for creating players. 
*/

//----------------------------------------------------------------------------

BenzenePlayer* PlayerFactory::CreatePlayer(BenzenePlayer* player)
{
    return 
        new SwapCheck
        (new EndgameCheck
	 (new SolverCheck 
	  (new LadderCheck(player))));
}

BenzenePlayer* PlayerFactory::CreatePlayerWithBook(BenzenePlayer* player)
{
    return 
        new SwapCheck
        (new EndgameCheck
	 (new HandBookCheck
          (new BookCheck
	   (new SolverCheck
	    (new LadderCheck(player))))));
}

BenzenePlayer* PlayerFactory::CreateTheoryPlayer(BenzenePlayer* player)
{
    return new VulPreCheck(player);
}

//----------------------------------------------------------------------------
