//----------------------------------------------------------------------------
/** @file Bitset.hpp */
//----------------------------------------------------------------------------

#ifndef BITSET_HPP
#define BITSET_HPP

#include <cassert>
#include <set>
#include <string>
#include <vector>

#include "Benzene.hpp"
#include "BenzeneAssert.hpp"
#include "Types.hpp"

/** If true, uses our own bitset, otherwise std::bitset.
    Homebrewed bitset is a copy of std::bitset with subset and
    less-than operations built in. Using the homebrewed bitset should
    improve performance slightly.
    If you are getting compile errors, switch to the stl bitset.
 */
#define USE_HOMEBREWED_BITSET   1

#if USE_HOMEBREWED_BITSET
#include "BenzeneBitset.hpp"
#else
#include <bitset>
#endif

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Maximum size of a bitset. 
    Very important. Only mess with this if you know what you are
    doing! */
#if defined(SUPPORT_19x19)

/** Actually need only 361+7 for 19x19. */
static const int BITSETSIZE = 384;

#elif defined(SUPPORT_14x14)

/** Actually need only 196+7 for 14x14. */
static const int BITSETSIZE = 224;

#elif defined(SUPPORT_13x13)

/** Actually need only 169+7 for 13x13. */
static const int BITSETSIZE = 192;

#else

/** Fits 11x11 exactly. */
static const int BITSETSIZE = 128;

#endif

//----------------------------------------------------------------------------

/** Standard-sized bitset. */
typedef benzene_bitset<BITSETSIZE> bitset_t;

/** Global empty bitset. */
static const bitset_t EMPTY_BITSET;

//----------------------------------------------------------------------------

/** Utilities on bitsets. */
namespace BitsetUtil 
{
    /** Converts the bottom numbits of b into a byte stream. */
    void BitsetToBytes(const bitset_t& b, byte* out, int numbits);
    
    /** Converts a byte stream into a bitset. */
    bitset_t BytesToBitset(const byte* bytes, int numbits);

    /** Converts a bitset into a string of hex symbols. */
    std::string BitsetToHex(const bitset_t& b, int numbits);

    /** Converts a string of hex symbols into a bitset. */
    bitset_t HexToBitset(const std::string& str);

    //------------------------------------------------------------------------

    /** Subtracts b2 from b1. */
    bitset_t Subtract(const bitset_t& b1, const bitset_t& b2);

    /** If removeFrom - remove is not empty, stores that value in
        removeFrom and returns true.  Otherwise, removeFrom is not
        changed and returns false. */
    bool SubtractIfLeavesAny(bitset_t& removeFrom, const bitset_t& remove);

    /** Returns true if b1 is a subset of b2. */
    bool IsSubsetOf(const bitset_t& b1, const bitset_t& b2);
    
    /** Returns true if b1 comes before b2 in some consistent order
        (any well defined ordering, not necessarily lexicographic). */
    bool IsLessThan(const bitset_t& b1, const bitset_t& b2);

    //------------------------------------------------------------------------

    /** Stores indices of set bits in b in indices. 
        @note INT must be convertible to an int.
    */
    template<typename INT>
    void BitsetToVector(const bitset_t& b, std::vector<INT>& indices);

    /** Converts of set of indices into a bitset with those bits set. 
        @note INT must be convertible to an int. 
     */
    template<typename INT>
    bitset_t SetToBitset(const std::set<INT>& indices);
    
    //------------------------------------------------------------------------

    /** Returns the bit that is set in b. */
    int FindSetBit(const bitset_t& b);

    /** Returns least-significant set bit in b. */
    int FirstSetBit(const bitset_t& b);
}

//----------------------------------------------------------------------------

template<typename INT>
void BitsetUtil::BitsetToVector(const bitset_t& b, 
                                std::vector<INT>& indices)
{
    indices.clear();
    for (int i = 0; i < BITSETSIZE; i++)
        if (b.test(i))
            indices.push_back(static_cast<INT>(i));
    BenzeneAssert(b.count() == indices.size());
}

template<typename INT>
bitset_t BitsetUtil::SetToBitset(const std::set<INT>& indices)
{
    bitset_t ret;
    typedef typename std::set<INT>::const_iterator const_iterator;
    for (const_iterator it = indices.begin(); 
         it != indices.end();
         ++it) 
    {
        ret.set(static_cast<int>(*it));
    }
    return ret;
}

//----------------------------------------------------------------------------

inline bool BitsetUtil::IsSubsetOf(const bitset_t& b1, const bitset_t& b2)
{
#if USE_HOMEBREWED_BITSET
    return b1.is_subset_of(b2);
#else
    return ((b1 & b2) == b1);
#endif
}

inline bool BitsetUtil::IsLessThan(const bitset_t& b1, const bitset_t& b2)
{
#if USE_HOMEBREWED_BITSET
    return b1.is_less_than(b2);
#else
    // holy crap this is sloooow!
    for (int i = 0; i < BITSETSIZE; ++i) 
    {
        if (b1[i] != b2[i]) 
            return !b1[i];
    }
#endif
}

//----------------------------------------------------------------------------

/** Extends the standard binary '-' operator for bitsets. */
bitset_t operator-(const bitset_t& b1, const bitset_t& b2);

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // BITSET_HPP
