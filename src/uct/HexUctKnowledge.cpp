//----------------------------------------------------------------------------
/** @file HexUctKnowledge.cpp
 */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "HexUctState.hpp"
#include "HexUctKnowledge.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

HexUctKnowledge::HexUctKnowledge(const HexUctState& state) 
    : m_state(state)
{
}

HexUctKnowledge::~HexUctKnowledge()
{
}

void HexUctKnowledge::ProcessPosition(std::vector<SgMoveInfo>& moves)
{
    SG_UNUSED(moves);
}

//----------------------------------------------------------------------------
