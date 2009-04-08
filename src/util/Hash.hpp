//----------------------------------------------------------------------------
/** @file
 */
//----------------------------------------------------------------------------

#ifndef HASH_HPP
#define HASH_HPP

#include "SgSystem.h"
#include "SgRandom.h"

#include <sstream>
#include <vector>

//----------------------------------------------------------------------------

/** 64-bit hash value. */
typedef unsigned long long hash_t;

//----------------------------------------------------------------------------

/** Turn on hash collision detection. */
#define CHECK_HASH_COLLISION   1

//----------------------------------------------------------------------------

/** Utilities on hash values. */
namespace HashUtil
{

/** Converts a hash value into a hex string. */
inline std::string toString(hash_t hash)
{
    std::ostringstream os;
    os << "0x" << std::hex << hash;
    return os.str();
}

/** Returns a random hash value. */
inline hash_t RandomHash()
{
    hash_t a = SgRandom::Global().Int();
    hash_t b = SgRandom::Global().Int();
    a <<= 32;
    return a | b;
}

}

//----------------------------------------------------------------------------

#endif // HASH_HPP
