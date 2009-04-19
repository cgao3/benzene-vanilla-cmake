//----------------------------------------------------------------------------
/** @file ZobristHash.cpp
 */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "SgRandom.h"
#include "ZobristHash.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

/** Use hashes defined in ZobrishHashes.hpp if this is true.
    @bug Setting this to 0 will break all OpeningBooks and SolverDBs.
    Only do this if you really know what you are doing.   
*/
#define USE_PREDEFINED_HASHES 1

//----------------------------------------------------------------------------

ZobristHash::GlobalData::GlobalData()
    : initialized(false)
{
}

//----------------------------------------------------------------------------

ZobristHash::ZobristHash()
{
    Initialize();
    reset();
}

ZobristHash::ZobristHash(const bitset_t& black, const bitset_t& white)
{
    Initialize();
    compute(black, white);
}

void ZobristHash::compute(const bitset_t& black, const bitset_t& white)
{
    reset();
    for (int p = 0; p < BITSETSIZE; ++p) 
    {
        if (black.test(p)) m_hash ^= GetGlobalData().zkey_color[BLACK][p];
        if (white.test(p)) m_hash ^= GetGlobalData().zkey_color[WHITE][p];
    }
}

//----------------------------------------------------------------------------

ZobristHash::GlobalData& ZobristHash::GetGlobalData()
{
    static GlobalData s_data;
    return s_data;
}


#if USE_PREDEFINED_HASHES

namespace
{
#include "ZobristHashes.hpp"
}

void ZobristHash::Initialize()
{
    if (GetGlobalData().initialized) 
        return;

    GetGlobalData().zkey_base = base_hash;
    int n = 0;
    for (int j=0; j<BITSETSIZE; ++j) 
    {
        HexAssert(n < NUM_HASHES);
        GetGlobalData().zkey_color[BLACK][j] = hashes[n++];
    }
    for (int j=0; j<BITSETSIZE; ++j)
    {
        HexAssert(n < NUM_HASHES);
        GetGlobalData().zkey_color[WHITE][j] = hashes[n++];
    }

    GetGlobalData().initialized = true;
}

#else

void ZobristHash::Initialize()
{
    if (GetGlobalData().initialized) 
        return;

    int old_seed = SgRandom::Global().Seed();
    SgRandom::Global().SetSeed(1);

    GetGlobalData().zkey_base = HashUtil::RandomHash();
    for (BWIterator it; it; ++it) 
    {
        for (int j=0; j<BITSETSIZE; j++) 
        {
            GetGlobalData().zkey_color[*it][j] = HashUtil::RandomHash();
        }
    }
    SgRandom::SetSeed(old_seed);

    GetGlobalData().initialized = true;
}

#endif

//----------------------------------------------------------------------------
