//----------------------------------------------------------------------------
/** @file SequenceHash.cpp */
//----------------------------------------------------------------------------

#include "SequenceHash.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

namespace
{
    
struct HashData
{
    HashData();
    hash_t m_hashes[BITSETSIZE][BITSETSIZE];
    hash_t m_colorHash[BLACK_WHITE_EMPTY];
};

HashData::HashData()
{
    for (int i = 0; i < BLACK_WHITE_EMPTY; ++i)
        m_colorHash[i] = HashUtil::RandomHash();
    for (int i = 0; i < BITSETSIZE; ++i)
        for (int j = 0; j < BITSETSIZE; ++j)
            m_hashes[i][j] = HashUtil::RandomHash();
}

const HashData& GetHashData()
{
    static HashData data;
    return data;
}

} // namespace

//----------------------------------------------------------------------------

hash_t SequenceHash::Hash(const PointSequence& seq)
{
    BenzeneAssert(seq.size() < (std::size_t)BITSETSIZE);
    const HashData& data = GetHashData();
    hash_t ret = 0;
    for (std::size_t i = 0; i < seq.size(); ++i)
        ret ^= data.m_hashes[i][seq[i]];
    return ret;
}

hash_t SequenceHash::Hash(const MoveSequence& seq)
{
    BenzeneAssert(seq.size() < (std::size_t)BITSETSIZE);
    const HashData& data = GetHashData();
    hash_t ret = 0;
    for (std::size_t i = 0; i < seq.size(); ++i)
    {
        ret ^= data.m_hashes[i][seq[i].Point()];
        ret ^= data.m_colorHash[seq[i].Color()];
    }
    return ret;
}

//----------------------------------------------------------------------------
