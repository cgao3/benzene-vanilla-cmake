//----------------------------------------------------------------------------
/** @file
 */
//----------------------------------------------------------------------------

#ifndef HEXEVAL_H
#define HEXEVAL_H

#include "Hex.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Data type for storing evaluation scores. */
typedef double HexEval;

//----------------------------------------------------------------------------

/** (HexPoint, value) pairs; use for move ordering. */
class HexMoveValue
{
public:
    HexMoveValue()
	: m_point(INVALID_POINT),
	  m_value(0.0)
    { }
    HexMoveValue(HexPoint point, double value)
        : m_point(point),
          m_value(value)
    { }
    
    HexPoint point() const { return m_point; }
    double value() const { return m_value; }

    bool operator< (const HexMoveValue& other) const
    {
        return m_value < other.m_value;
    }

    bool operator> (const HexMoveValue& other) const
    {
        return m_value > other.m_value;
    }
    
private:
    HexPoint m_point;
    double m_value;
};

//----------------------------------------------------------------------------

/** Scores >= WIN_THRESHOLD are wins and scores <= LOSS_THRESHOLD are
    losses.  The difference between a score and IMMEDIATE_WIN or
    IMMEDIATE_LOSS should correspond directly with the number of ply
    to win or lose.  For example, a win in 5 moves should have score
    IMMEDIATE_WIN - 5 >= WIN_THRESHOLD.
*/
static const HexEval  IMMEDIATE_WIN =  10000.0;
static const HexEval  WIN_THRESHOLD =   9000.0; 
static const HexEval LOSS_THRESHOLD =  -9000.0;
static const HexEval IMMEDIATE_LOSS = -10000.0;

static const HexEval EVAL_INFINITY = 1000000.0;

namespace HexEvalUtil
{
    bool IsValidEval(HexEval ev);

    bool IsWin(HexEval ev);
    int PlyToWin(HexEval ev);

    bool IsLoss(HexEval ev);
    int PlyToLoss(HexEval ev);

    bool IsWinOrLoss(HexEval ev);
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // HEXEVAL_H
