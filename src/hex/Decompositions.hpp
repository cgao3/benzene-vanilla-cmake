//----------------------------------------------------------------------------
/** @file Decompositions.hpp */
//----------------------------------------------------------------------------

#ifndef DECOMPOSITIONS_HPP
#define DECOMPOSITIONS_HPP

#include "Hex.hpp"
#include "HexBoard.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Functions for finding combinatorial decompositions. */
namespace Decompositions
{
    /** Must be called before any decomposition related function is
        called. */
    void Initialize();

    /** Returns true if there is a combinatorial decomposition for
        color where the opposing color edges are VC-connected. The
        VC's carrier will be stored in captured.
        InitializeDecompositions() must be called once before this can
        be used. */
    bool Find(const HexBoard& brd, HexColor color, bitset_t& captured);

    /** Returns true if there is a combinatorial decomposition for
        color that splits the board (i.e. touches both edges of the
        opposite color).  Group that splits the board is stored in
        group. InitializeDecompositions() must be called once before
        this can be used. */
    bool FindSplitting(const HexBoard& brd, HexColor color, HexPoint& group);
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // DECOMPOSITIONS_HPP
