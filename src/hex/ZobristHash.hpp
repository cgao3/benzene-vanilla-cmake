//----------------------------------------------------------------------------
/** @file ZobristHash.hpp */
//----------------------------------------------------------------------------

#ifndef ZOBRISTHASH_HPP
#define ZOBRISTHASH_HPP

#include "SgHash.h"
#include "HexColor.hpp"
#include "HexPoint.hpp"
#include "Bitset.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Zobrist Hashing.

    Hash values are shared amoung all instances of ZobristHash.

    Each unique boardsize has its own base hash, so hashes of
    positions on different boardsizes should never collide.
*/
class ZobristHash
{
public:
    /** Constructs a ZobristHash object for the given boardsize. */
    ZobristHash(int width, int height);

    /** Returns the current hash value for the color to play. */
    SgHashCode Hash(HexColor toPlay) const;

    /** Helper function: same as Hash(EMPTY). */
    SgHashCode Hash() const;
    
    /** Reset hash to the base hash value. */
    void Reset();

    /** Sets the hash to base hash value updated with the played
        stones in black and white. */
    void Compute(const bitset_t& black, const bitset_t& white);

    /** Incrementally updates the hash value with the given move. */
    void Update(HexColor color, HexPoint cell);

private:
    /** Hash for the current state. */
    SgHashCode m_hash;

    /** Base hash. */
    SgHashCode m_base;

    //----------------------------------------------------------------------

    /** Data shared amoungst all instances of ZobristHash. */
    struct GlobalData
    {
        static const int NUM_HASHES = 4096;

        SgHashCode m_hashes[NUM_HASHES];

        SgHashCode* m_black_hashes;

        SgHashCode* m_white_hashes;

        SgHashCode* m_color_hashes[BLACK_AND_WHITE];

        SgHashCode* m_toPlay_hashes[BLACK_WHITE_EMPTY];

        GlobalData();

        void SetPointers();

        void GetHashes();
    };

    /** Returns a reference to a static local variable so that it is
        initialized on first use if some global variable is
        initialized. */
    static GlobalData& GetGlobalData(); 
};

inline SgHashCode ZobristHash::Hash(HexColor toPlay) const
{
    SgHashCode ret(m_hash);
    ret.Xor(*GetGlobalData().m_toPlay_hashes[toPlay]);
    return ret;
}

inline SgHashCode ZobristHash::Hash() const
{
    return Hash(EMPTY);
}

inline void ZobristHash::Reset()
{
    m_hash = m_base;
}

inline void ZobristHash::Update(HexColor color, HexPoint cell)
{
    BenzeneAssert(HexColorUtil::isBlackWhite(color));
    BenzeneAssert(0 <= cell && cell < BITSETSIZE);
    m_hash.Xor(GetGlobalData().m_color_hashes[color][cell]);
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // ZOBRISTHASH_HPP
