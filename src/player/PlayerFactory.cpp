//----------------------------------------------------------------------------
/** @file PlayerFactory.cpp
    Various factory methods for creating players. 
*/
//----------------------------------------------------------------------------

#include "PlayerFactory.hpp"
#include "BookCheck.hpp"
#include "EndgameCheck.hpp"
#include "HandBookCheck.hpp"
#include "LadderCheck.hpp"
#include "SwapCheck.hpp"
#include "VulPreCheck.hpp"

//----------------------------------------------------------------------------

BenzenePlayer* PlayerFactory::CreatePlayer(BenzenePlayer* player)
{
    return 
        new SwapCheck
        (new EndgameCheck
	  (new LadderCheck(player)));
}

BenzenePlayer* PlayerFactory::CreatePlayerWithBook(BenzenePlayer* player)
{
    return 
        new SwapCheck
        (new EndgameCheck
	 (new HandBookCheck
          (new BookCheck
	    (new LadderCheck(player)))));
}

BenzenePlayer* PlayerFactory::CreateTheoryPlayer(BenzenePlayer* player)
{
    return new VulPreCheck(player);
}

//----------------------------------------------------------------------------
