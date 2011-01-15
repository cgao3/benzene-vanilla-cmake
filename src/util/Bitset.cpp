//----------------------------------------------------------------------------
/** @file Bitset.cpp */
//----------------------------------------------------------------------------

#include <sstream>
#include <cstdio>
#include "Bitset.hpp"
#include "BenzeneAssert.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

void BitsetUtil::BitsetToBytes(const bitset_t& b, byte* out, int numbits)
{
    numbits = ((numbits + 7) / 8) * 8;
    BenzeneAssert(numbits <= BITSETSIZE);
    for (int i = 0; i < numbits; i += 8) 
    {
        int c = 0;
        for (int j = 0; j < 8; j++) 
            c += (b.test(i + j)) ? 1 << j : 0;
        out[i / 8] = static_cast<byte>(c);
    }
}

bitset_t BitsetUtil::BytesToBitset(const byte* bytes, int numbits)
{
    bitset_t ret;
    int numbytes = (numbits + 7) / 8;
    for (int i = 0; i < numbytes; i++) 
    {
        for (int j = 0; j < 8; j++)
            if (bytes[i] & (1 << j))
                ret.set(i * 8 + j);
    }
    return ret;
}

std::string BitsetUtil::BitsetToHex(const bitset_t& b, int numbits)
{
    numbits = ((numbits + 3) / 4) * 4;
    BenzeneAssert(numbits <= BITSETSIZE);
    std::string out;
    for (int i = 0; i < numbits; i += 4) 
    {
        int c = 0;
        for (int j = 0; j < 4; j++) 
            c += (b.test(i + j)) ? (1 << j) : 0;
        char buff[4];
        sprintf(buff, "%x", c);
        out += buff;
    }
    return out;
}

bitset_t BitsetUtil::HexToBitset(const std::string& str)
{
    bitset_t out;
    for (std::size_t i = 0; i < str.size(); i++) 
    {
        unsigned c;
        char buff[4];
        buff[0] = str[i];
        buff[1] = 0;
        sscanf(buff, "%x", &c);
        for (int j = 0; j < 4; j++)
            if (c & (1 << j))
                out.set(i * 4 + j);
    }
    return out;
}

//----------------------------------------------------------------------------

bitset_t BitsetUtil::Subtract(const bitset_t& b1, const bitset_t& b2)
{
    return (b1 ^ (b1 & b2));
}

bool BitsetUtil::SubtractIfLeavesAny(bitset_t& removeFrom,
                                     const bitset_t& remove)
{
    bitset_t leftover = removeFrom - remove;
    if (leftover.any()) 
    {
        removeFrom = leftover;
        return true;
    } 
    return false;
}

int BitsetUtil::FindSetBit(const bitset_t& b)
{
    BenzeneAssert(b.count() == 1);
    int ret;
    for (ret = 0; ret < BITSETSIZE; ret++) 
        if (b.test(ret))
            break;
    return ret;
}

/** @todo Merge with FindSetBit()? */
int BitsetUtil::FirstSetBit(const bitset_t& b)
{
    BenzeneAssert(b.any());
    int ret;
    for (ret = 0; ret < BITSETSIZE; ret++)
        if (b.test(ret))
            break;
    return ret;
}
//----------------------------------------------------------------------------

/** @note Must specify the namespace here for some reason. */
bitset_t benzene::operator-(const bitset_t& b1, const bitset_t& b2)
{
    return BitsetUtil::Subtract(b1, b2);
}

//----------------------------------------------------------------------------
