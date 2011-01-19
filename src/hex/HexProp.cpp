//----------------------------------------------------------------------------
/** @file HexProp.cpp */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "SgProp.h"
#include "HexProp.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

void HexProp::Init()
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

SgProp* HexProp::AddMoveProp(SgNode* node, SgMove move, SgBlackWhite player)
{
    SG_ASSERT_BW(player);
    SgProp* moveProp = node->AddMoveProp(move, player);
    return moveProp;
}

//----------------------------------------------------------------------------
