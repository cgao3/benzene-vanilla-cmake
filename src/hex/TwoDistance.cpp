//----------------------------------------------------------------------------
/** @file TwoDistance.cpp */
//----------------------------------------------------------------------------

#include "Hex.hpp"
#include "BitsetIterator.hpp"
#include "TwoDistance.hpp"
#include "VCSet.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

TwoDistance::TwoDistance(NeighbourType ntype)
    : m_brd(0),
      m_ntype(ntype),
      m_score(0)
{
}

TwoDistance::~TwoDistance()
{
}

//----------------------------------------------------------------------------

void TwoDistance::Evaluate(const HexBoard& brd)
{
    m_brd = &brd;
    for (BWIterator it; it; ++it)
        ComputeScores(*it, m_scores[*it]);
    ComputeScore();
}

void TwoDistance::ComputeScores(HexColor color, HexEval* out)
{
    HexEval dist[2][BITSETSIZE];
    ComputeDistanceToEdge(color, HexPointUtil::colorEdge1(color), dist[0]);
    ComputeDistanceToEdge(color, HexPointUtil::colorEdge2(color), dist[1]);

    for (BoardIterator it(m_brd->Const().Interior()); it; ++it) 
    {
        if (m_brd->GetPosition().IsOccupied(*it))
            out[*it] = 0;
        else 
            out[*it] = TwoDistUtil::AddDistance(dist[0][*it], dist[1][*it]);
            //out[*it] = dist[0][*it];
    }
}

void TwoDistance::FindBest(HexEval* po, HexPoint& who, int& count)
{
    HexEval best = EVAL_INFINITY;
    who = INVALID_POINT;
    count = 1;

    for (BitsetIterator it(m_brd->GetPosition().GetEmpty()); it; ++it) 
    {
        if (po[*it] < best) 
        {
            best = po[*it];
            who = *it;
            count=1;
        }
        else if (po[*it] == best) 
            count++;
    }

    BenzeneAssert(who != INVALID_POINT);
    BenzeneAssert(best != EVAL_INFINITY);
}

void TwoDistance::ComputeScore()
{
    HexPoint black, white;
    int black_count, white_count;
    int SCALE_FACTOR = 1; // FIXME: what is a good value?

    FindBest(m_scores[BLACK], black, black_count);
    FindBest(m_scores[WHITE], white, white_count);

    m_score = (black - white)*SCALE_FACTOR + (black_count - white_count);
}

//----------------------------------------------------------------------------

bool TwoDistance::IsAdjacent(HexColor color, HexPoint p1, HexPoint p2)
{
    VC vc;
    if (m_brd->Cons(color).SmallestVC(p1, p2, VC::FULL, vc)) {
        switch(m_ntype) {
        case ADJACENT: return vc.IsEmpty(); 
        case  FULL_VC: return true;
        }
    }
    return false;
}

void TwoDistance::ComputeDistanceToEdge(HexColor color,
                                        HexPoint edge, 
                                        HexEval* out)
{
    SetAllToInfinity(out);
    
    //
    // FIXME: how to sort queue by increasing order without using
    // negative priorities?  
    //
    //    q(std::less<std::pair<int, HexPoint> >)
    //
    // doesn't seem to work.
    //

    std::priority_queue<std::pair<int, HexPoint> > q;
    std::set<HexPoint> done;
    std::set<HexPoint> once;

    // add immediate neighbours of edge
    for (BitsetIterator it(m_brd->GetPosition().GetEmpty()); it; ++it) 
    {
        if (IsAdjacent(color, *it, edge)) 
        {
            out[*it] = 1;
            q.push(std::make_pair(-1, *it));
            done.insert(*it);
        }
    }

    while (!q.empty()) {
        std::pair<int, HexPoint> pp = q.top();
        q.pop();

        int dist = -pp.first;        
        HexPoint p = pp.second;
                  
        for (BitsetIterator it(m_brd->GetPosition().GetEmpty()); it; ++it) 
        {
            if (IsAdjacent(color, *it, p)) 
            {
                // it has been seen at least twice before, ignore it.
                if (done.count(*it))
                    continue;
                // it has been seen once before, add it to queue
                // and add to 'done'.
                else if (once.count(*it)) 
                {
                    out[*it] = dist+1;
                    q.push(std::make_pair(-(dist+1), *it));
                    done.insert(*it);
                } 
                // it hasn't been seen before: add it to 'once'.
                else
                    once.insert(*it);
            }
        }
    }
}

void TwoDistance::SetAllToInfinity(HexEval* out)
{
    for (BoardIterator it(m_brd->Const().Interior()); it; ++it)
        out[*it] = EVAL_INFINITY;
}

//----------------------------------------------------------------------------

HexEval TwoDistUtil::AddDistance(HexEval a, HexEval b)
{
    if (a == EVAL_INFINITY || b == EVAL_INFINITY)
        return EVAL_INFINITY;
    return a + b;
}

//----------------------------------------------------------------------------
