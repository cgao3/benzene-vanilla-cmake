//----------------------------------------------------------------------------
/** @file SequenceHash.hpp */
//----------------------------------------------------------------------------

#ifndef SEQUENCEHASH_HPP
#define SEQUENCEHASH_HPP

#include "SgHash.h"
#include "Move.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** MoveSequence hashing. */
namespace SequenceHash
{
    /** Hashes a sequence of points. */
    SgHashCode Hash(const PointSequence& seq);
    
    /** Hashes a sequence of moves. */
    SgHashCode Hash(const MoveSequence& seq);
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif  // SEQUENCEHASH_HPP
