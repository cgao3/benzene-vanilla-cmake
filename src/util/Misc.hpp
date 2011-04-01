//----------------------------------------------------------------------------
/** @file Misc.hpp */
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

    /** Get directory of the executable from arguments to main().
        If this function is called in main(), the directory of the executable
        will be extracted from the path to the executable in argv[0] (if
        possible) and added to the data directories searched in
        MiscUtil::OpenFile(). */
    void FindProgramDir(int argc, char* argv[]);

    std::string OpenFile(std::string name, std::ifstream& f);
}

/** Prints a vector with a space between elements. */
template<typename TYPE>
std::string MiscUtil::PrintVector(const std::vector<TYPE>& v)
{
    std::ostringstream is;
    for (std::size_t i = 0; i < v.size(); ++i) 
    {
        if (i) is << ' ';
        is << v[i];
    }
    return is.str();
} 

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // MISC_HPP
