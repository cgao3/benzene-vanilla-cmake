//----------------------------------------------------------------------------
/** @file BenzeneSolver.hpp
 */
//----------------------------------------------------------------------------

#ifndef BENZENESOLVER_H
#define BENZENESOLVER_H

#include "Hex.hpp"
#include "ConstBoard.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

struct SolverDataFlags
{
    /** Marks the proof as that of a transposition of some other
        state. */
    static const int TRANSPOSITION = 1;

    /** Marks the proof as a mirror transposition of some other
        state. */
    static const int MIRROR_TRANSPOSITION = 2;
};

//----------------------------------------------------------------------------

/** Contains a bestmove. */
template<class T>
struct HasBestMoveConcept
{
    void constraints() 
    {
        const T t;
        HexPoint point = t.m_bestMove;
        SG_UNUSED(point);
    }
};

/** Contains a flag. */
template<class T>
struct HasFlagsConcept
{
    void constraints() 
    {
        const T t;
        int flags = t.m_flags;
        SG_UNUSED(flags);
    }
};

/** Object can be mirrored. */
template<class T>
struct HasMirrorConcept
{
    void constraints() 
    {
        const ConstBoard& brd = ConstBoard::Get(1, 1);
        T t;
        t.Mirror(brd);
    }
};

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // BENZENESOLVER_H
