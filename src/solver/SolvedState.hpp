//----------------------------------------------------------------------------
/** @file SolvedState.hpp
 */
//----------------------------------------------------------------------------

#ifndef SOLVEDSTATE_H
#define SOLVEDSTATE_H

#include "Hex.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** A solved state. Stored in a TT or DB.  
    Matches HashTableStateConcept and TransTableStateConcept.
*/
struct SolvedState
{
    /** Marks the proof as that of a transposition of some other
        state. */
    static const int FLAG_TRANSPOSITION = 1;

    /** Marks the proof as a mirror transposition of some other
        state. */
    static const int FLAG_MIRROR_TRANSPOSITION = 2;

    //--------------------------------------------------------------------

    /** True if player to move wins. */
    bool win;

    /** Flags. */
    int flags;
    
    /** Number of states in proof-tree of this result. */
    int numstates;
    
    /** Number of moves losing player can delay until winning
        player has a winning virtual connection. */
    int nummoves;

    /** Best move in this state. 
        Very important in winning states, not so important in losing
        states. That is, in winning states this move *must* be a
        winning move, in losing states this move is "most blocking",
        but the definition is fuzzy. */
    HexPoint bestmove;

    //--------------------------------------------------------------------
    
    /** Carrier of the proof. 
        @todo Take this out of here since the proof computed for a
        state depends on the sequence of moves used to reach it.
    */
    bitset_t proof;
    
    /** Winner's stones inside proof. 
        @todo Take this out.
    */
    bitset_t winners_stones;

    /** Number of stones on board; could be used to determine TT
        priority. */
    int numstones;
    
    /** Zobrist hash. Not always set (if from DB hit). */
    hash_t hash;
    
    /** Black stones; used to check for hash collisions. Not
        always set (if from DB hit). */
    bitset_t black;
    
    /** White stones; used to check for hash collisions. Not
        always set (if from DB hit). */
    bitset_t white;

    //--------------------------------------------------------------------

    /** Contructs state with default values.  Required by
        HashTableStateConcept and TransTableStateConcept. */
    SolvedState();
    
    /** Initializes state to given values. */
    SolvedState(int d, hash_t h, bool w, int nstates, 
                int nmoves, HexPoint bmove, 
                const bitset_t& bp, const bitset_t& ws,
                const bitset_t& bb, const bitset_t& bw);

    //--------------------------------------------------------------------

    /** Returns true if this state is not the same as that built by
        the default constructor. */
    bool Initialized() const;

    /** Returns the hash value of this state. */
    hash_t Hash() const;

    /** If true, then this will give up its TT slot to other.
        @note ALWAYS RETURNS TRUE FOR NOW!  */
    bool ReplaceWith(const SolvedState& other) const;
    
    /** Checks for hash collisions betweent his and the given
        hash/black/white bitsets. */
    void CheckCollision(hash_t hash, const bitset_t& black,
                        const bitset_t& white) const;

    //--------------------------------------------------------------------

    /** @name Methods for PackableConcept (needed byHashDB). */
    // @{

    int PackedSize() const;

    byte* Pack() const;

    void Unpack(const byte* t);

    // @}
};

inline SolvedState::SolvedState()
    : win(false), flags(0), numstates(0), nummoves(0), 
      bestmove(INVALID_POINT), proof(), winners_stones(),
      numstones(9999), hash(0), black(), white()
{ 
}
    
inline SolvedState::SolvedState(int d, hash_t h, bool w, int nstates, 
                                int nmoves, HexPoint bmove, 
                                const bitset_t& bp, const bitset_t& ws,
                                const bitset_t& bb, const bitset_t& bw)
    : win(w), flags(0), numstates(nstates), nummoves(nmoves), 
      bestmove(bmove), proof(bp), winners_stones(ws),
      numstones(d), hash(h), black(bb), white(bw)
{ 
    HexAssert(BitsetUtil::IsSubsetOf(winners_stones, proof)); 
}

inline bool SolvedState::Initialized() const
{
    return (numstones != 9999);
}

inline hash_t SolvedState::Hash() const
{
    return hash;
}

inline bool SolvedState::ReplaceWith(const SolvedState& other) const
{
    UNUSED(other);
    return true;
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // SOLVEDSTATE_H
