//----------------------------------------------------------------------------
/** @file ProofUtil.hpp
 */
//----------------------------------------------------------------------------

#ifndef PROOFUTIL_HPP
#define PROOFUTIL_HPP

#include "Hex.hpp"
#include "StoneBoard.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Deduce equivalent states, etc. */
namespace ProofUtil 
{
    /** Computes and stores in db the transpostions of this proof on
        the given boardstate. Returns number of db entries
        successfully added or updated. */
    int StoreTranspositions(DfsDB& db, const StoneBoard& brd, 
                            const DfsData& state, const bitset_t& proof);

    /** Computes and stores in db the flipped transpostions of this
        proof on the given boardstate. Returns number of db entries
        successfully added or updated. */
    int StoreFlippedStates(DfsDB& db, const StoneBoard& brd, 
                           const DfsData& state, const bitset_t& proof);
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // PROOFUTIL_HPP
