//----------------------------------------------------------------------------
/** @file Resistance.cpp */
//----------------------------------------------------------------------------

#ifndef RESISTANCE_HPP
#define RESISTANCE_HPP

#include "Hex.hpp"
#include "HexEval.hpp"
#include "HexBoard.hpp"
#include "Groups.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Adjacency between each pair of cells. */
class AdjacencyGraph
{
public:
    
    /** Creates an empty adjacency graph. */
    AdjacencyGraph();

    ~AdjacencyGraph();
    
    std::vector<bool>& operator[](int n);
    
    const std::vector<bool>& operator[](int n) const;

private:
    std::vector< std::vector<bool> > m_adj;
};

inline AdjacencyGraph::AdjacencyGraph()
    : m_adj(BITSETSIZE, std::vector<bool>(BITSETSIZE, false))
{ }

inline AdjacencyGraph::~AdjacencyGraph()
{ }

inline std::vector<bool>& AdjacencyGraph::operator[](int n)
{
    return m_adj[n];
}

inline const std::vector<bool>& AdjacencyGraph::operator[](int n) const
{
    return m_adj[n];
}

//----------------------------------------------------------------------------

/** Conductance between different types of groups. */
struct ConductanceValues
{
    /** Conductance between a pair of groups with no connection. */
    double no_connection;

    /** Conductance between a pair of empty groups. */
    double empty_to_empty;

    /** Conductance between an occupied group and an empty group. */
    double color_to_empty;

    /** Conductance between two occupied groups. */
    double color_to_color;

    /** Initializes conductance defaults. */
    ConductanceValues()
        : no_connection(0.0),
          empty_to_empty(1.0),
          color_to_empty(2.0),
          color_to_color(0.0)
    { }
};

//----------------------------------------------------------------------------

/** Board evaluation based on circuit flow. */
class Resistance
{
public:

    /** Constructor. */
    explicit Resistance();

    /** Destructor. */
    virtual ~Resistance();

    //---------------------------------------------------------------------- 

    /** Computes the evaluation for the given boardstate; uses
        ResistanceUtil::AddAdjacencies() to compute the
        conductance graphs for this board. */
    void Evaluate(const HexBoard& brd);

    /** Computes the evaluation for the given boardstate with the given
        AdjacencyGraphs for each color. 
    
        @note Graphs are not constant because if
        "resist-simulate-and-over-edge" is true, the graphs will be
        augmented.  Augmentation is done here so that it is done only
        one spot, as opposed to having the augmentation done in all
        spots calling Evaluate().
    */
    void Evaluate(const HexBoard& brd, 
                  AdjacencyGraph graph[BLACK_AND_WHITE]);


    /** Evaluate on the given groups with the given adjacency
        graph. */
    void Evaluate(const Groups& groups, AdjacencyGraph graph[BLACK_AND_WHITE]);

    //---------------------------------------------------------------------- 

    /** Returns the log(resistance) for the given color. */
    double Resist(HexColor color) const;

    /** Returns the resistance of the board position from BLACK's
        view. That is, the log of the white resistance over the black
        resistance. */
    HexEval Score() const;

    /** Returns the score for cell and color.  A cells score is equal
        to the current flowing through it. */
    HexEval Score(HexPoint cell, HexColor color) const;

    /** Returns the sum of the BLACK and WHITE scores for this
        cell. */
    HexEval Score(HexPoint cell) const;

private:
    /** Compute the evaluation for a single color. */
    void ComputeScores(HexColor color, const Groups& brd,
                       const AdjacencyGraph& graph, 
                       const ConductanceValues& values, 
                       HexEval* out);

    /** Returns score as a ration of black's score over white's. */
    void ComputeScore();

    HexEval m_score;
    HexEval m_resistance[BLACK_AND_WHITE];
    HexEval m_scores[BLACK_AND_WHITE][BITSETSIZE];
};

inline HexEval Resistance::Score() const
{
    return m_score;
}

inline HexEval Resistance::Score(HexPoint cell) const
{
    /** @todo How to handle bad cells? */
    return m_scores[BLACK][cell] + m_scores[WHITE][cell];
}

inline HexEval Resistance::Score(HexPoint cell, HexColor color) const
{
    /** @todo How to handle bad cells? */
    return m_scores[color][cell];
}

//----------------------------------------------------------------------------

/** Utilities to compute AdjacencyGraphs, etc. */
namespace ResistanceUtil
{
    /** Computes AdjacencyGraphs for this board state using a
        default ConductanceValues object. */
    void AddAdjacencies(const HexBoard& brd, 
                        AdjacencyGraph graph[BLACK_AND_WHITE]);

}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // RESISTANCE_HPP
