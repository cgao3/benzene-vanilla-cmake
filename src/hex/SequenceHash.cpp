//----------------------------------------------------------------------------
// $Id: SequenceHash.cpp 1657 2008-09-15 23:32:09Z broderic $
//----------------------------------------------------------------------------

#include "SequenceHash.hpp"

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
    for (int i=0; i<BITSETSIZE; ++i) {
        for (int j=0; j<BITSETSIZE; ++j) {
            hashes[i][j] = HashUtil::RandomHash();
        }
    }
}

const HashData& GetHashData()
{
    static HashData data;
    return data;
}

} 

//----------------------------------------------------------------------------

hash_t SequenceHash::Hash(const MoveSequence& seq)
{
    HexAssert((int)seq.size() < BITSETSIZE);

    const HashData& data = GetHashData();

    hash_t ret = 0;
    for (std::size_t i=0; i<seq.size(); ++i) {
        ret ^= data.hashes[i][seq[i]];
    }

    return ret;
}

//----------------------------------------------------------------------------
