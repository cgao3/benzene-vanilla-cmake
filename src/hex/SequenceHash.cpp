//----------------------------------------------------------------------------
/** @file SequenceHash.cpp
 */
//----------------------------------------------------------------------------

#include "SequenceHash.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

namespace
{
    
struct HashData
{
    HashData();
    hash_t hashes[BITSETSIZE][BITSETSIZE];
};

HashData::HashData()
{
    for (int i = 0; i < BITSETSIZE; ++i)
        for (int j = 0; j < BITSETSIZE; ++j)
            hashes[i][j] = HashUtil::RandomHash();
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
    HexAssert((int)seq.size() < BITSETSIZE);

    const HashData& data = GetHashData();

    hash_t ret = 0;
    for (std::size_t i = 0; i < seq.size(); ++i) 
        ret ^= data.hashes[i][seq[i]];

    return ret;
}


hash_t SequenceHash::Hash(const MoveSequence& seq)
{
    HexAssert((int)seq.size() < BITSETSIZE);

    const HashData& data = GetHashData();

    hash_t ret = 0;
    for (std::size_t i = 0; i < seq.size(); ++i) 
        ret ^= data.hashes[i][seq[i].point()];

    return ret;
}

//----------------------------------------------------------------------------
