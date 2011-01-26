//----------------------------------------------------------------------------
/** @file VCUtil.hpp */
//----------------------------------------------------------------------------

#ifndef VCUTIL_HPP
#define VCUTIL_HPP

#include "Hex.hpp"
#include "HexBoard.hpp"
#include "StoneBoard.hpp"
#include "VC.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Utilities on VCs. */
namespace VCUtil
{
    /** Returns mustplay for color to move. */
    bitset_t GetMustplay(const HexBoard& brd, HexColor color);

    /** Returns true if carrier defines a valid bridge to the
        edge. Edge and the other endpoint are stored if it is
        valid. */
    bool ValidEdgeBridge(const StoneBoard& brd, const bitset_t& carrier, 
                         HexPoint& endpoint, HexPoint& edge);
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // VCUTIL_HPP
