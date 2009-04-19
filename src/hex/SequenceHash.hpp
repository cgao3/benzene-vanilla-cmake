//----------------------------------------------------------------------------
/** @file SequenceHash.hpp
 */
//----------------------------------------------------------------------------

#ifndef SEQUENCEHASH_HPP
#define SEQUENCEHASH_HPP

#include "Hex.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** MoveSequence hashing. */
namespace SequenceHash
{
    /** Hashes a move sequence. */
    hash_t Hash(const PointSequence& seq);
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif  // SEQUENCEHASH_HPP
