//----------------------------------------------------------------------------
/** @file MoHexPriorKnowledge.hpp */
//----------------------------------------------------------------------------

#ifndef MOHEXPRIORKNOWLEDGE_HPP
#define MOHEXPRIORKNOWLEDGE_HPP

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

class MoHexThreadState;

/** Applies knowledge to set of moves. */
class MoHexPriorKnowledge
{
public:
    MoHexPriorKnowledge(const MoHexThreadState& m_state);

    ~MoHexPriorKnowledge();

    void ProcessPosition(std::vector<SgUctMoveInfo>& moves);
    
private:
    const MoHexThreadState& m_state;
};

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // MOHEXPRIORKNOWLEDGE_HPP
