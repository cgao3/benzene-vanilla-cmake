//----------------------------------------------------------------------------
/** @file HexUctKnowledge.hpp
 */
//----------------------------------------------------------------------------

#ifndef HEXUCTKNOWLEDGE_HPP
#define HEXUCTKNOWLEDGE_HPP

//#include "SgSystem.h"
//#include "SgUctSearch.h"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

class HexUctState;

/** Applies knowledge to set of moves. */
class HexUctKnowledge
{
public:
    HexUctKnowledge(const HexUctState& m_state);

    ~HexUctKnowledge();

    void ProcessPosition(std::vector<SgMoveInfo>& moves);
    
private:
    const HexUctState& m_state;
};

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // HEXUCTKNOWLEDGE_HPP
