//----------------------------------------------------------------------------
/** @file HexProp.hpp */
//----------------------------------------------------------------------------

#ifndef HEXPROP_HPP
#define HEXPROP_HPP

#include "SgSystem.h"
#include "SgBlackWhite.h"
#include "SgPoint.h"
#include "SgProp.h"
#include "SgNode.h"

#include "Benzene.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Properties for sgfs. */
namespace HexProp
{
    void Init();

    SgProp* AddMoveProp(SgNode* node, SgMove move, SgBlackWhite player);
}
  
//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // HEXPROP_HPP


