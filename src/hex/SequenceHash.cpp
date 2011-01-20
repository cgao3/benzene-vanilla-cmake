//----------------------------------------------------------------------------
/** @file SequenceHash.cpp */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "SequenceHash.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

namespace
{
    
struct HashData
{
    HashData();
    SgHashCode m_hashes[BITSETSIZE][BITSETSIZE];
    SgHashCode m_colorHash[BLACK_WHITE_EMPTY];
};

HashData::HashData()
{
    for (int i = 0; i < BLACK_WHITE_EMPTY; ++i)
        m_colorHash[i] = SgHash<64>::Random();
    for (int i = 0; i < BITSETSIZE; ++i)
        for (int j = 0; j < BITSETSIZE; ++j)
            m_hashes[i][j] = SgHash<64>::Random();
}

const HashData& GetHashData()
{
    static HashData data;
    return data;
}

} // namespace

//----------------------------------------------------------------------------

SgHashCode SequenceHash::Hash(const PointSequence& seq)
{
    BenzeneAssert(seq.size() < (std::size_t)BITSETSIZE);
    const HashData& data = GetHashData();
    SgHashCode ret;
    for (std::size_t i = 0; i < seq.size(); ++i)
        ret.Xor(data.m_hashes[i][seq[i]]);
    return ret;
}

SgHashCode SequenceHash::Hash(const MoveSequence& seq)
{
    BenzeneAssert(seq.size() < (std::size_t)BITSETSIZE);
    const HashData& data = GetHashData();
    SgHashCode ret;
    for (std::size_t i = 0; i < seq.size(); ++i)
    {
        ret.Xor(data.m_hashes[i][seq[i].Point()]);
        ret.Xor(data.m_colorHash[seq[i].Color()]);
    }
    return ret;
}

//----------------------------------------------------------------------------
