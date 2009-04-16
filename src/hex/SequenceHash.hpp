//----------------------------------------------------------------------------
/** @file SequenceHash.hpp
 */
//----------------------------------------------------------------------------

#ifndef SEQUENCEHASH_HPP
#define SEQUENCEHASH_HPP

#include "Hex.hpp"

//----------------------------------------------------------------------------

/** MoveSequence hashing. */
namespace SequenceHash
{

    /** Hashes a move sequence. */
    hash_t Hash(const PointSequence& seq);

}

//----------------------------------------------------------------------------

#endif  // SEQUENCEHASH_HPP
