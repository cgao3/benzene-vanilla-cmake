//----------------------------------------------------------------------------
// $Id: ZobristHash.hpp 1657 2008-09-15 23:32:09Z broderic $
//----------------------------------------------------------------------------

#ifndef ZOBRISTHASH_HPP
#define ZOBRISTHASH_HPP

#include "Hex.hpp"

//----------------------------------------------------------------------------

/** Computes hash for two bitsets (used to compute hashes for
    boardstates where each bitset represents the stones for each
    color).
    
    Hash values are shared amoung all instances of ZobristHash, and
    are created using the generator SgRandom::Global() when the first
    ZobristHash object is created.
*/
class ZobristHash
{
public:
    /** Constructs a ZobristHash object with the current hash set to
        the base hash value. */
    ZobristHash();

    /** Constructs a ZobristHash object with the current hash set to
        the base hash value updated with the cells in black and
        white. */
    ZobristHash(const bitset_t& black, const bitset_t& white);

    /** Returns the current hash value. */
    hash_t hash() const;
    
    /** Reset hash to the base hash value. */
    void reset();

    /** Sets the hash to base hash value updated with the played
        stones in black and white. */
    void compute(const bitset_t& black, const bitset_t& white);

    /** Incrementally updates the hash value with the given move. */
    void update(HexColor color, HexPoint cell);

    //-----------------------------------------------------------------------

    /** Initialize the set of hashes. */
    static void Initialize();

private:

    /** Hash for the current state. */
    hash_t m_hash;

    //----------------------------------------------------------------------

    /** Data shared amoungst all instances of ZobristHash. */
    struct GlobalData
    {
        bool initialized;
        hash_t zkey_base;
        hash_t zkey_color[BLACK_AND_WHITE][BITSETSIZE];

        GlobalData();
    };

    /** Returns a reference to a static local variable, so that it is
        initialized on first use if some global variable is
        initialized. */
    static GlobalData& GetGlobalData(); 
};

inline hash_t ZobristHash::hash() const
{
    return m_hash;
}

inline void ZobristHash::reset()
{
    m_hash = GetGlobalData().zkey_base;
}

inline void ZobristHash::update(HexColor color, HexPoint cell)
{
    HexAssert(HexColorUtil::isBlackWhite(color));
    HexAssert(0 <= cell && cell < BITSETSIZE);
    m_hash ^= GetGlobalData().zkey_color[color][cell];
}

//----------------------------------------------------------------------------

#endif // ZOBRISTHASH_HPP
