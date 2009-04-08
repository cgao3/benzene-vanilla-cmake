//----------------------------------------------------------------------------
// $Id: HexProp.cpp 1842 2009-01-18 00:05:04Z broderic $
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "SgProp.h"
#include "HexProp.hpp"

//----------------------------------------------------------------------------

void HexInitProp()
{
    SgProp* moveProp = new SgPropMove(0);
    SG_PROP_MOVE_BLACK 
        = SgProp::Register(moveProp, "B", 
                           SG_PROPCLASS_MOVE + SG_PROPCLASS_BLACK);
    SG_PROP_MOVE_WHITE
        = SgProp::Register(moveProp, "W", 
                           SG_PROPCLASS_MOVE + SG_PROPCLASS_WHITE);
}

//----------------------------------------------------------------------------

SgProp* HexPropUtil::AddMoveProp(SgNode* node, SgMove move, 
                                 SgBlackWhite player)
{
    SG_ASSERT_BW(player);
    SgProp* moveProp = node->AddMoveProp(move, player);
    return moveProp;
}

//----------------------------------------------------------------------------
