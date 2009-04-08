//----------------------------------------------------------------------------
// $Id: Resistance.cpp 1994 2009-04-06 00:57:12Z broderic $
//----------------------------------------------------------------------------

/** @file 
    
    Resistance/energy calculatiosn based very closely on Six's circuit
    implementation.
*/

#include <cmath>

#include "Hex.hpp"
#include "BitsetIterator.hpp"
#include "Connections.hpp"
#include "Resistance.hpp"
#include "Pattern.hpp"
#include "HashedPatternSet.hpp"

#include "lssolve.h"

//----------------------------------------------------------------------------

Resistance::Resistance()
    : m_score(0),
      m_simulate_and_over_edge(false)
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

    // augment connection graph if option is on
    if (m_simulate_and_over_edge)
        ResistanceUtil::SimulateAndOverEdge(brd, graph);

    for (BWIterator c; c; ++c) 
        ComputeScores(*c, brd, graph[*c], values, m_scores[*c]);
    ComputeScore();
}

//----------------------------------------------------------------------------

namespace
{
    /** Sets all cell scores to an explicitly undefined value. */
    void SetAllToInfinity(const StoneBoard& brd, HexEval* out)
    {
        for (BoardIterator it(brd.Interior()); it; ++it) 
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
    
        HexColor ac = brd.getColor(a);
        HexColor bc = brd.getColor(b);

        if (ac == EMPTY && bc == EMPTY)
            return values.empty_to_empty;
        
        if (ac == color && bc == color)
            return values.color_to_color;
        
        return values.color_to_empty;
    }
}

void Resistance::ComputeScores(HexColor color, const GroupBoard& brd,
                               const AdjacencyGraph& graph, 
                               const ConductanceValues& values, 
                               HexEval* out)
{
    SetAllToInfinity(brd, out);

    int ic,jc;
    HexColorSet not_other = HexColorSetUtil::ColorOrEmpty(color);
    HexPoint source = HexPointUtil::colorEdge1(color);
    HexPoint sink = HexPointUtil::colorEdge2(color);
    int n = brd.NumGroups(not_other);

    Mat<double> G(n, n);   // conductances
    Vec<double> sinkG(n);  // conductances from sink to each group
    Vec<double> I(n);      // initial currents

    G = 0.0;
    sinkG = 0.0;
    I = 0.0;
    
    // put some current on the source
    I[brd.GroupIndex(not_other, source)] = 1.0;

    // compute conductance between groups
    ic=0;
    for (BoardIterator i(brd.Groups(not_other)); i; ++i, ++ic) 
    {
        jc=0;
        for (BoardIterator j(brd.Groups(not_other)); *j != *i; ++j, ++jc) 
        {
            double conductance = Conductance(brd, color, *i, *j, 
                                             graph[*i][*j], values);
            
            if (*i != sink) 
                G(ic, ic) += conductance;
            else 
                sinkG(jc) += conductance;

            if (*j != sink)
                G(jc, jc) += conductance;
            else 
                sinkG(ic) += conductance;

            if (*i != sink && *j != sink) 
            {
                G(ic, jc) -= conductance;
                G(jc, ic) -= conductance;
            }
        }
    }

    // solve for voltages
    const Vec<double>& V = lsSolve(G, I);

    m_resistance[color] = fabs(V[brd.GroupIndex(not_other, source)]);

    // compute energy for each cell
    ic=0;
    for (BoardIterator i(brd.Groups(not_other)); i; ++i, ++ic) 
    {
        jc=0;
        double sum = fabs(sinkG[ic] * V[ic]);
        for (BoardIterator j(brd.Groups(not_other)); j; ++j, ++jc) 
        {
            sum += fabs(G(ic, jc) * (V[ic] - V[jc]));
        }
        out[*i] = sum;
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
    for (BoardIterator x(brd.Stones(not_other)); x; ++x) 
    {
        for (BoardIterator y(brd.Stones(not_other)); *y != *x; ++y) 
        {
            HexPoint cx = brd.getCaptain(*x);
            HexPoint cy = brd.getCaptain(*y);
            if ((cx==cy) || brd.Cons(color).Exists(cx, cy, VC::FULL))
            {
                graph[*x][*y] = true;
                graph[*y][*x] = true;
            }
        }
    }
}

bool g_ResistanceUtilInitialized = false;

PatternSet s_capmiai[BLACK_AND_WHITE];
HashedPatternSet* s_hash_capmiai[BLACK_AND_WHITE];

void InitializeCapMiai()
{
    if (g_ResistanceUtilInitialized) return;

    LogFine() << "--InitializeCapMiai" << '\n';

    //
    // The center 'B' is marked so the group can be determined. 
    //
    //              *
    //             . .
    //            B B B                          [capmiai/0]
    // m:0,0,0,0,0;0,0,0,0,0;0,0,0,0,0;0,0,0,0,0;7,6,0,4,0;3,2,0,0,0;1
    //
    std::string capmiai = "m:0,0,0,0,0;0,0,0,0,0;0,0,0,0,0;0,0,0,0,0;7,6,0,4,0;3,2,0,0,0;1";

    Pattern pattern;
    if (!pattern.unserialize(capmiai)) {
        HexAssert(false);
    }
    pattern.setName("capmiai");

    s_capmiai[BLACK].push_back(pattern);
    s_hash_capmiai[BLACK] = new HashedPatternSet();
    s_hash_capmiai[BLACK]->hash(s_capmiai[BLACK]);

    pattern.flipColors();
    s_capmiai[WHITE].push_back(pattern);
    s_hash_capmiai[WHITE] = new HashedPatternSet();
    s_hash_capmiai[WHITE]->hash(s_capmiai[WHITE]);
}

/** Simulates and'ing over the edge for the given color and edge. All
    empty neighbours of an edge get all the connections the edge
    has. All cells that are a captured-miai away from the edge get all
    connections the edge has. */
void SimulateAndOverEdge1(const HexBoard& brd, HexColor color, 
                         HexPoint edge, AdjacencyGraph& graph)
{
    bitset_t augment = brd.Nbs(edge, EMPTY);

    // add in miai-captured empty cells adjacent to edge
    HexAssert(g_ResistanceUtilInitialized);

    bitset_t adjToEdge;
    for (BitsetIterator p(brd.getEmpty()); p; ++p) {
        PatternHits hits;
        brd.matchPatternsOnCell(*s_hash_capmiai[color], *p, 
                                PatternBoard::MATCH_ALL, hits);
        for (unsigned j=0; j<hits.size(); ++j) {
            HexPoint cj = brd.getCaptain(hits[j].moves1()[0]);
            if (cj == edge) {
                augment.set(*p);
                break;
            }
        }
    }

    // add the edge's connections to all cells in augment
    for (BitsetIterator p(augment); p; ++p) {
        for (BoardIterator q(brd.EdgesAndInterior()); q; ++q) {
            graph[*p][*q] = graph[*p][*q] || graph[edge][*q];
            graph[*q][*p] = graph[*q][*p] || graph[edge][*q];
        }
    }
}

} // annonymous namespace

void ResistanceUtil::Initialize()
{
    InitializeCapMiai();
    g_ResistanceUtilInitialized = true;
}

void ResistanceUtil::AddAdjacencies(const HexBoard& brd,
                                    AdjacencyGraph graph[BLACK_AND_WHITE])
{
    for (BWIterator c; c; ++c) 
        AddAdjacent(*c, brd, graph[*c]);
}

void ResistanceUtil::SimulateAndOverEdge(const HexBoard& brd,
                                         AdjacencyGraph g[BLACK_AND_WHITE])
{
    if (brd.Builder().Parameters().and_over_edge)
    {
        LogWarning() 
                 << "**** Simulating and-over-edge "
                 << "while vcs are computed over edge!! ****" 
                 << '\n';
    }

    SimulateAndOverEdge1(brd,BLACK, HexPointUtil::colorEdge1(BLACK), g[BLACK]);
    SimulateAndOverEdge1(brd,BLACK, HexPointUtil::colorEdge2(BLACK), g[BLACK]);
    SimulateAndOverEdge1(brd,WHITE, HexPointUtil::colorEdge1(WHITE), g[WHITE]);
    SimulateAndOverEdge1(brd,WHITE, HexPointUtil::colorEdge2(WHITE), g[WHITE]);
}

//----------------------------------------------------------------------------
