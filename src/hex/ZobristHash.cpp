//----------------------------------------------------------------------------
/** @file ZobristHash.cpp */
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
{
    SetPointers();
    GetHashes();
}

void ZobristHash::GlobalData::SetPointers()
{
    m_black_hashes = &m_hashes[1024];
    m_white_hashes = &m_hashes[2048];
    m_color_hashes[BLACK] = m_black_hashes;
    m_color_hashes[WHITE] = m_white_hashes;
    m_toPlay_hashes[BLACK] = &m_hashes[3072];
    m_toPlay_hashes[WHITE] = &m_hashes[3073];
    m_toPlay_hashes[EMPTY] = &m_hashes[3074];
}

#if USE_PREDEFINED_HASHES

namespace
{
#include "ZobristHashes.hpp"
}

void ZobristHash::GlobalData::GetHashes()
{
    for (int i = 0; i < NUM_HASHES; ++i)
        m_hashes[i].FromString(s_predefined_hashes[i]);
}

#else

void ZobristHash::GlobalData::GetHashes()
{
    int old_seed = SgRandom::Global().Seed();
    SgRandom::Global().SetSeed(1);
    for (int i = 0; i < NUM_HASHES; ++i)
        m_hashes[i] = SgHash::Random();
    SgRandom::SetSeed(old_seed);
}

#endif

//----------------------------------------------------------------------------

ZobristHash::ZobristHash(int width, int height)
    : m_base(GetGlobalData().m_hashes[30 * width + height])
{
    BenzeneAssert(30 * width + height < 1024);
    Reset();
}

ZobristHash::GlobalData& ZobristHash::GetGlobalData()
{
    static GlobalData data;
    return data;
}

void ZobristHash::Compute(const bitset_t& black, const bitset_t& white)
{
    Reset();
    for (int p = 0; p < BITSETSIZE; ++p) 
    {
        if (black.test(p)) 
            m_hash.Xor(GetGlobalData().m_black_hashes[p]);
        if (white.test(p)) 
            m_hash.Xor(GetGlobalData().m_white_hashes[p]);
    }
}

//----------------------------------------------------------------------------
