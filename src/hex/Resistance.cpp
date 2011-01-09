//----------------------------------------------------------------------------
/** @file Resistance.cpp

    Resistance/energy calculation based very closely on Six's circuit
    implementation.

    We use the same open source code to solve the linear system that
    Six uses. We actually tried linking with two different external
    linear algebra libraries (GNU Scientific library and
    boost::numeric::ublas) but both fail in some instances that our
    current code handles without complaint.

    These instances seem to include a linear dependence among the rows
    of our G matrix. In this simplest case, this can happen if one
    group's set of connections is a superset of another group's. We
    haven't been able to find a way around this, as any fix seems more
    expensive than worthwhile.

    We also hoped these external libraries would be faster; again,
    this does not appear to be the case. 

    It is possible that a more powerful library like LAPACK would
    address both of these issues, but the hassle of adding this
    external dependency to benzene probably outweighs the gain.    
*/
//----------------------------------------------------------------------------

#include <cmath>

#include "Hex.hpp"
#include "BitsetIterator.hpp"
#include "VCSet.hpp"
#include "Resistance.hpp"
#include "Pattern.hpp"
#include "HashedPatternSet.hpp"

#include "lssolve.h"

using namespace benzene;

//----------------------------------------------------------------------------

Resistance::Resistance()
    : m_score(0)
{
}

Resistance::~Resistance()
{
}

//----------------------------------------------------------------------------

void Resistance::Evaluate(const HexBoard& brd)
{
    AdjacencyGraph graph[BLACK_AND_WHITE];
    ResistanceUtil::AddAdjacencies(brd, graph);
    Evaluate(brd, graph);
}

void Resistance::Evaluate(const HexBoard& brd, 
                          AdjacencyGraph graph[BLACK_AND_WHITE])
{
    ConductanceValues values;
    for (BWIterator c; c; ++c) 
        ComputeScores(*c, brd.GetGroups(), graph[*c], values, m_scores[*c]);
    ComputeScore();
}

void Resistance::Evaluate(const Groups& groups, 
                          AdjacencyGraph graph[BLACK_AND_WHITE])
{
    ConductanceValues values;
    for (BWIterator c; c; ++c) 
        ComputeScores(*c, groups, graph[*c], values, m_scores[*c]);
    ComputeScore();
}

//----------------------------------------------------------------------------

namespace
{
    /** Sets all cell scores to an explicitly undefined value. */
    void SetAllToInfinity(const StoneBoard& brd, HexEval* out)
    {
        for (BoardIterator it(brd.Const().Interior()); it; ++it) 
            out[*it] = EVAL_INFINITY;
    }

    /** Returns the conductance between two cells by comparing their
        colors and whether they are connected or not. */
    double Conductance(const StoneBoard& brd, HexColor color, 
                       HexPoint a, HexPoint b, bool connected,
                       const ConductanceValues& values)
    {
        if (!connected)
            return values.no_connection;
    
        HexColor ac = brd.GetColor(a);
        HexColor bc = brd.GetColor(b);

        if (ac == EMPTY && bc == EMPTY)
            return values.empty_to_empty;
        
        if (ac == color && bc == color)
            return values.color_to_color;
        
        return values.color_to_empty;
    }
}

void Resistance::ComputeScores(HexColor color, const Groups& groups,
                               const AdjacencyGraph& graph, 
                               const ConductanceValues& values, 
                               HexEval* out)
{
    const StoneBoard& brd = groups.Board();
    SetAllToInfinity(brd, out);

    const HexColorSet not_other = HexColorSetUtil::ColorOrEmpty(color);
    const HexPoint source = HexPointUtil::colorEdge1(color);
    const HexPoint sink = HexPointUtil::colorEdge2(color);
    const int n = static_cast<int>(groups.NumGroups(not_other) - 1);

    // Compute index that does not contain the sink
    int pointToIndex[BITSETSIZE];
    HexPoint indexToPoint[BITSETSIZE];
    {
        int index = 0;
        for (GroupIterator i(groups, not_other); i; ++i)
        {
            if (i->Captain() == sink)
                continue;
            pointToIndex[i->Captain()] = index;
            indexToPoint[index] = i->Captain();
            index++;
        }
    }

    // Compute conductances between groups
    Mat<double> G(n, n);
    std::vector<double> sinkG(n, 0.0);
    G = 0.0;
    for (int i = 0; i < n; ++i)
    {
        HexPoint ip = indexToPoint[i];
        for (int j = 0; j < i; ++j)
        {
            HexPoint jp = indexToPoint[j];
            double c = Conductance(brd, color, ip, jp, graph[ip][jp], values);
            G(i, i) += c;
            G(j, j) += c;
            G(i, j) -= c;
            G(j, i) -= c;                
        }
        double c = Conductance(brd, color, ip, sink, graph[ip][sink], values);
        G(i, i) += c;
        sinkG[i] += c;
    }

    // Put some current on the source
    Vec<double> I(n);
    I = 0.0;
    I[pointToIndex[source]] = 1.0;

    // Solve for voltages
    const Vec<double>& V = lsSolve(G, I);
    m_resistance[color] = fabs(V[pointToIndex[source]]);

    // Compute energy
    for (int i = 0; i < n; ++i)
    {
        double sum = fabs(sinkG[i] * V[i]);
        for (int j = 0; j < n; ++j)
            sum += fabs(G(i,j) * (V[i] - V[j]));
        out[indexToPoint[i]] = sum;
    }
}

void Resistance::ComputeScore()
{
    double r = m_resistance[WHITE] / m_resistance[BLACK];
    m_score = log(r);
}

double Resistance::Resist(HexColor color) const
{
    return log(m_resistance[color]);
}

//---------------------------------------------------------------------------
// AdjacencyGraph computations

namespace
{

/** Computes a AdjacencyGraph for color on the given board. */
void AddAdjacent(HexColor color, const HexBoard& brd,
                 AdjacencyGraph& graph)
{
    HexColorSet not_other = HexColorSetUtil::ColorOrEmpty(color);
    for (BoardIterator x(brd.GetPosition().Stones(not_other)); x; ++x) 
    {
        for (BoardIterator y(brd.GetPosition().Stones(not_other)); *y!=*x; ++y) 
        {
            HexPoint cx = brd.GetGroups().CaptainOf(*x);
            HexPoint cy = brd.GetGroups().CaptainOf(*y);
            if ((cx == cy) || brd.Cons(color).Exists(cx, cy, VC::FULL))
            {
                graph[*x][*y] = true;
                graph[*y][*x] = true;
            }
        }
    }
}

} // annonymous namespace

void ResistanceUtil::AddAdjacencies(const HexBoard& brd,
                                    AdjacencyGraph graph[BLACK_AND_WHITE])
{
    for (BWIterator c; c; ++c) 
        AddAdjacent(*c, brd, graph[*c]);
}
//----------------------------------------------------------------------------
