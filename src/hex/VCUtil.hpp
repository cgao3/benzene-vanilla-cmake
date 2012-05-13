//----------------------------------------------------------------------------
/** @file VCUtil.hpp */
//----------------------------------------------------------------------------

#ifndef VCUTIL_HPP
#define VCUTIL_HPP

#include "Hex.hpp"
#include "HexBoard.hpp"
#include "StoneBoard.hpp"
#include "VCS.hpp"

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

    /** Computes responses to probe.
        For each connected group xy, tries to restore connection xy
        after probe p by playing key of semi not touched by p. Also
        finds flaring moves around p. */
    void RespondToProbe(const HexBoard& vcbrd, const HexColor toPlay,
                        const HexPoint probe, bitset_t& responses);
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // VCUTIL_HPP
