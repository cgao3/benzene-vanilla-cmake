//----------------------------------------------------------------------------
/** @file SequenceHash.hpp
 */
//----------------------------------------------------------------------------

#ifndef SEQUENCEHASH_HPP
#define SEQUENCEHASH_HPP

#include "Hex.hpp"
#include "Move.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** MoveSequence hashing. */
namespace SequenceHash
{
    /** Hashes a sequence of points. */
    hash_t Hash(const PointSequence& seq);
    
    /** Hashes a sequence of moves. 
        @bug Currently ignores color of move.
    */
    hash_t Hash(const MoveSequence& seq);
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif  // SEQUENCEHASH_HPP
