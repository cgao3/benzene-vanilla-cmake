//----------------------------------------------------------------------------
/** @file ProofUtil.hpp
 */
//----------------------------------------------------------------------------

#ifndef PROOFUTIL_HPP
#define PROOFUTIL_HPP

#include "Hex.hpp"
#include "StoneBoard.hpp"
#include "ICEngine.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Deduce equivalent states, etc. */
namespace ProofUtil 
{
    /** Gives all cells outside of the proof to loser, computes fillin
        using ice, removes any cell in proof that is filled-in. 
        Returns true if proof was shrunk. */
    bool ShrinkProof(bitset_t& proof, const StoneBoard& board,
                     HexColor loser, const ICEngine& ice);

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
