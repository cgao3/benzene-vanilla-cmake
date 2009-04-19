//----------------------------------------------------------------------------
/** @file
 */
//----------------------------------------------------------------------------

#ifndef TWODISTANCE_HPP
#define TWODISTANCE_HPP

#include "Hex.hpp"
#include "HexEval.hpp"
#include "HexBoard.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** TwoDistance Evaluation Function.
    
    Computes the two distance from each cell to each of the four
    edges.  The two distance is the second shortest distance between
    two cells (1 if they are adjacent, and infinity if fewer than two
    connecting paths exist).
  
    This evaluation function requires that the VCs be precalculated
    for the given board state.  This calcuation runs in O(n^2) time,
    where n is the number of cells on the board.

    @bug if NeighbourType is FULL_VC then the distance returned is not
    accurate since one cell we have a vc with could affect another
    cell we have a vc with.
*/
class TwoDistance
{
public:

    /** Two types of cell neighbourhoods: ADJACENT and FULL_VC.
        ADJACENT: standard adjacency, going through stones of like
                  color. 
         FULL_VC: two cells are adjacent if a FULL vc exists
                  between them. */
    typedef enum { ADJACENT, FULL_VC } NeighbourType;

    /** Compute the TwoDistance on the given HexBoard (with up-to-date
        VCs) and NeighbourType.  */
    explicit TwoDistance(NeighbourType ntype = ADJACENT);

    /** Destructor. */
    virtual ~TwoDistance();

    /** Computes the evaluation. */
    virtual void Evaluate(const HexBoard& brd);

    /** Returns the computed score for BLACK, that is: 

             Score = SCALE*(B_m - W_m) + (B_mc - W_mc);
        
        Where SCALE is an arbitrary scaling factor (hex setting
        "twod-scale-factor"), B_m and W_m are the minimum BLACK and WHITE
        cell scores, and B_mc and W_mc are equal to
        the number of times B_m and W_m appear on the board.

        B_mc and W_mc are tie breaking values.  The intuition here is
        that a position with many cells with minimum potential is
        better than a position with only a single cell with minimum
        potential. */
    virtual HexEval Score() const;

    /** Returns the sum of the BLACK and WHITE scores for this
        cell. */
    virtual HexEval Score(HexPoint cell) const;

    /** Returns the score for cell and color. This is the sum of
        the twodistances between both edges for cell. */
    virtual HexEval Score(HexPoint cell, HexColor color) const;

private:

    void ComputeScores(HexColor color, HexEval* out);

    void FindBest(HexEval* po, HexPoint& who, int& count);

    void ComputeScore();

    void ComputeDistanceToEdge(HexColor color, HexPoint edge1, HexEval* out);

    bool IsAdjacent(HexColor color, HexPoint p1, HexPoint p2);

    void SetAllToInfinity(HexEval* out);

    const HexBoard* m_brd;
    NeighbourType m_ntype;

    HexEval m_score;
    HexEval m_scores[BLACK_AND_WHITE][BITSETSIZE];
};

inline HexEval TwoDistance::Score() const
{
    return m_score;
}

inline HexEval TwoDistance::Score(HexPoint cell, HexColor color) const
{
    // @todo How to handle bad cells?
    return m_scores[color][cell];
}

inline HexEval TwoDistance::Score(HexPoint cell) const
{
    // @todo How to handle bad cells?
    return m_scores[BLACK][cell] + m_scores[WHITE][cell];
}

//----------------------------------------------------------------------------

namespace TwoDistUtil
{
    /** Add two distances without mangling infinites. */
    HexEval AddDistance(HexEval a, HexEval b);
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // TWODISTANCE_HPP
