//----------------------------------------------------------------------------
/** @file MoHexPriorKnowledge.cpp */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "MoHexThreadState.hpp"
#include "MoHexPriorKnowledge.hpp"

using namespace benzene;

extern double GlobalPatterns_GetGammaFromBoard(const StoneBoard& board, 
                                               HexPoint point, HexColor toPlay,
                                               bool *isBadPattern);

//----------------------------------------------------------------------------

MoHexPriorKnowledge::MoHexPriorKnowledge(const MoHexThreadState& state) 
    : m_state(state)
{
}

MoHexPriorKnowledge::~MoHexPriorKnowledge()
{
}

void MoHexPriorKnowledge::ProcessPosition(std::vector<SgUctMoveInfo>& moves)
{
    double TotalGamma = 0;
    double MoveGamma[BITSETSIZE];
    for (std::size_t i = 0; i < moves.size(); ++i)
    {
        bool isBadPattern;
        double gamma 
            = GlobalPatterns_GetGammaFromBoard(m_state.Board(), 
                                               HexPoint(moves[i].m_move),
                                               m_state.GetColorToPlay(), 
                                               &isBadPattern);
	MoveGamma[(int)moves[i].m_move] = gamma;
        TotalGamma += gamma;
    }
    if (TotalGamma == 0)
        return;
    for (std::size_t i = 0; i < moves.size(); ++i)
    {
        double gamma = MoveGamma[(int)moves[i].m_move];
        double prob = gamma / TotalGamma;
        moves[i].m_prior = prob;
	moves[i].m_raveValue = 0.5f;
	moves[i].m_raveCount = 8;
    }
}

//----------------------------------------------------------------------------
