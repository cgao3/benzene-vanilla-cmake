//----------------------------------------------------------------------------
/** @file SearchedState.hpp */
//----------------------------------------------------------------------------

#ifndef SEARCHEDSTATE_HPP
#define SEARCHEDSTATE_HPP

#include "SgHash.h"
#include "Hex.hpp"
#include "HexEval.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** State that has been searched with Alpha-Beta. */
struct SearchedState
{
    //------------------------------------------------------------------------
                         
    typedef enum { LOWER_BOUND, UPPER_BOUND, ACCURATE, NOT_DEFINED } Bound;

    //------------------------------------------------------------------------

    SearchedState()
        : hash(0), bound(NOT_DEFINED), score(0), move(INVALID_POINT),
          depth(0)
    { }
    
    SearchedState(SgHashCode h, int depth, Bound b, HexEval s, HexPoint m)
        : hash(h), bound(b), score(s), move(m), depth(depth)
    { }

    ~SearchedState();
    
    bool Initialized() const;

    SgHashCode Hash() const;

    void CheckCollision(const SearchedState& other) const;

    bool ReplaceWith(const SearchedState& other) const;

    //------------------------------------------------------------------------

    /** Zobrist Hash for this state. */
    SgHashCode hash;
   
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

inline SgHashCode SearchedState::Hash() const
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
