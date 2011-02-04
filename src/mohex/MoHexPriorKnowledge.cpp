//----------------------------------------------------------------------------
/** @file MoHexPriorKnowledge.cpp */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "MoHexThreadState.hpp"
#include "MoHexPriorKnowledge.hpp"

using namespace benzene;

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
    SG_UNUSED(moves);
}

//----------------------------------------------------------------------------
