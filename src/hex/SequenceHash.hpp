//----------------------------------------------------------------------------
// $Id: SequenceHash.hpp 1657 2008-09-15 23:32:09Z broderic $
//----------------------------------------------------------------------------

#ifndef SEQUENCEHASH_HPP
#define SEQUENCEHASH_HPP

#include "Hex.hpp"

//----------------------------------------------------------------------------

/** MoveSequence hashing. */
namespace SequenceHash
{

    /** Hashes a move sequence. */
    hash_t Hash(const MoveSequence& seq);

}

//----------------------------------------------------------------------------

#endif  // SEQUENCEHASH_HPP
