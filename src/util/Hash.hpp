//----------------------------------------------------------------------------
/** @file Hash.hpp
 */
//----------------------------------------------------------------------------

#ifndef HASH_HPP
#define HASH_HPP

#include "SgSystem.h"
#include "SgRandom.h"
#include <sstream>
#include <iomanip>

#include "Benzene.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** 64-bit hash value. */
typedef unsigned long long hash_t;

//----------------------------------------------------------------------------

/** Turn on hash collision detection. */
#define CHECK_HASH_COLLISION   0

//----------------------------------------------------------------------------

/** Utilities on hash values. */
namespace HashUtil
{
    /** Converts a hash value into a hex string. */
    std::string toString(hash_t hash);

    /** Returns a random hash value. */
    hash_t RandomHash();
}

//----------------------------------------------------------------------------

inline std::string HashUtil::toString(hash_t hash)
{
    std::ostringstream os;
    os.fill('0');
    os << "0x" << std::hex << std::setw(16) << hash;
    return os.str();
}

inline hash_t HashUtil::RandomHash()
{
    hash_t a = SgRandom::Global().Int();
    hash_t b = SgRandom::Global().Int();
    a <<= 32;
    return a | b;
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // HASH_HPP
