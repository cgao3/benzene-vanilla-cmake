//----------------------------------------------------------------------------
/** @file
 */
//----------------------------------------------------------------------------

#ifndef SEARCHEDSTATE_HPP
#define SEARCHEDSTATE_HPP

#include "Hex.hpp"
#include "HexEval.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** State that has been search with Alpha-Beta. */
struct SearchedState
{
    //------------------------------------------------------------------------
                         
    typedef enum { LOWER_BOUND, UPPER_BOUND, ACCURATE, NOT_DEFINED } Bound;

    //------------------------------------------------------------------------

    SearchedState()
        : hash(0), bound(NOT_DEFINED), score(0), move(INVALID_POINT),
          depth(0)
    { }
    
    SearchedState(hash_t h, int depth, Bound b, HexEval s, HexPoint m)
        : hash(h), bound(b), score(s), move(m), depth(depth)
    { }

    ~SearchedState();
    
    bool Initialized() const;

    hash_t Hash() const;

    void CheckCollision(const SearchedState& other) const;

    bool ReplaceWith(const SearchedState& other) const;

    //------------------------------------------------------------------------

    /** Zobrist Hash for this state. */
    hash_t hash;
   
    /** How the score should be interpreted. */
    Bound bound;

    /** Score for this state. */
    HexEval score;

    /** Best move found. */
    HexPoint move;

    /** Depth state was searched. */
    int depth;

};

inline SearchedState::~SearchedState()
{
}

inline bool SearchedState::Initialized() const
{
    return (move != INVALID_POINT);
}

inline hash_t SearchedState::Hash() const
{
    return hash;
}

inline 
void SearchedState::CheckCollision(const SearchedState& other) const
{
    UNUSED(other);
}

inline bool SearchedState::ReplaceWith(const SearchedState& other) const
{
    /** @todo check for better bounds/scores? */

    // replace this state only with a deeper state
    return (other.depth > depth);
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // SEARCHEDSTATE_HPP
