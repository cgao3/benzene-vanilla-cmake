//----------------------------------------------------------------------------
/** @file VCUtils.hpp
 */
//----------------------------------------------------------------------------

#ifndef VCUTILS_HPP
#define VCUTILS_HPP

#include "Hex.hpp"
#include "HexBoard.hpp"
#include "StoneBoard.hpp"
#include "VC.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Utilities on VCs. */
namespace VCUtils
{
    /** Returns mustplay for color to move. 
        @todo Document what the mustplay is! 
     */
    bitset_t GetMustplay(const HexBoard& brd, HexColor color);

    /** Returns true if carrier defines a valid bridge to the
        edge. Edge and the other endpoint are stored if it is valid.
    */
    bool ValidEdgeBridge(const StoneBoard& brd, 
                         const bitset_t& carrier, 
                         HexPoint& endpoint,
                         HexPoint& edge);
};

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // VCUTILS_HPP
