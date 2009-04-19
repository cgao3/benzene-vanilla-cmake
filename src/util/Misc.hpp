//----------------------------------------------------------------------------
/** @file Misc.hpp
 */
//----------------------------------------------------------------------------

#ifndef MISC_HPP
#define MISC_HPP

#include <sstream>
#include <string>
#include <vector>
#include "Benzene.hpp"
#include "Types.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Misc. Utilities. */
namespace MiscUtil
{
    /** Converts a word to an array of bytes. */
    void WordToBytes(unsigned word, byte* out);

    /** Converts an array of four bytes into a word. */
    unsigned BytesToWord(const byte* bytes);

    /** Returns the number of bytes need to hold the given number of
        bits. Equal to (bits + 7)/8.
     */
    int NumBytesToHoldBits(int bits);

    /** Prints a vector with a space between elements. */
    template<typename TYPE>
    std::string PrintVector(const std::vector<TYPE>& v);
};

/** Prints a vector with a space between elements. */
template<typename TYPE>
std::string MiscUtil::PrintVector(const std::vector<TYPE>& v)
{
    std::ostringstream is;
    for (std::size_t i=0; i<v.size(); ++i) {
        if (i) is << ' ';
        is << v[i];
    }
    return is.str();
} 

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // MISC_HPP
