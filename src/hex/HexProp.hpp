//----------------------------------------------------------------------------
// $Id: HexProp.hpp 1470 2008-06-17 22:04:32Z broderic $
//----------------------------------------------------------------------------

#ifndef HEXPROP_HPP
#define HEXPROP_HPP

#include "SgSystem.h"
#include "SgBlackWhite.h"
#include "SgPoint.h"
#include "SgProp.h"
#include "SgNode.h"

void HexInitProp();

namespace HexPropUtil
{
    SgProp* AddMoveProp(SgNode* node, SgMove move, SgBlackWhite player);
}
  
//----------------------------------------------------------------------------

#endif // HEXPROP_HPP


