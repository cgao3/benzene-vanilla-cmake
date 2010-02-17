//----------------------------------------------------------------------------
/** @file ZobristHash.hpp
 */
//----------------------------------------------------------------------------

#ifndef ZOBRISTHASH_HPP
#define ZOBRISTHASH_HPP

#include "Hex.hpp"

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
    hash_t Hash(HexColor toPlay) const;

    /** Helper function: same as Hash(EMPTY). */
    hash_t Hash() const;
    
    /** Reset hash to the base hash value. */
    void Reset();

    /** Sets the hash to base hash value updated with the played
        stones in black and white. */
    void Compute(const bitset_t& black, const bitset_t& white);

    /** Incrementally updates the hash value with the given move. */
    void Update(HexColor color, HexPoint cell);

private:

    /** Hash for the current state. */
    hash_t m_hash;

    /** Base hash. */
    hash_t m_base;

    //----------------------------------------------------------------------

    /** Data shared amoungst all instances of ZobristHash. */
    struct GlobalData
    {
        static const int NUM_HASHES = 4096;

        hash_t m_hashes[NUM_HASHES];

        hash_t* m_black_hashes;

        hash_t* m_white_hashes;

        hash_t* m_color_hashes[BLACK_AND_WHITE];

        hash_t* m_toPlay_hashes[BLACK_WHITE_EMPTY];

        GlobalData();

        void SetPointers();

        void GetHashes();
    };

    /** Returns a reference to a static local variable so that it is
        initialized on first use if some global variable is
        initialized. */
    static GlobalData& GetGlobalData(); 
};

inline hash_t ZobristHash::Hash(HexColor toPlay) const
{
    return m_hash ^ *GetGlobalData().m_toPlay_hashes[toPlay];
}

inline hash_t ZobristHash::Hash() const
{
    return Hash(EMPTY);
}

inline void ZobristHash::Reset()
{
    m_hash = m_base;
}

inline void ZobristHash::Update(HexColor color, HexPoint cell)
{
    HexAssert(HexColorUtil::isBlackWhite(color));
    HexAssert(0 <= cell && cell < BITSETSIZE);
    m_hash ^= GetGlobalData().m_color_hashes[color][cell];
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // ZOBRISTHASH_HPP
