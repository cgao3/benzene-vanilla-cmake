//----------------------------------------------------------------------------
/** @file HexStateAssertRestored.hpp */
//----------------------------------------------------------------------------

#ifndef HEXSTATEASSERTRESTORED_HPP
#define HEXSTATEASSERTRESTORED_HPP

#include "HexState.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Assert that the state has been restored to previous state. */
class HexStateAssertRestored
{
public:
    /** Constructor for later explicit call of Init() */
    HexStateAssertRestored();

    /** Constructor, calls Init(). */
    HexStateAssertRestored(const HexState& state);

    /** Destructor, calls CheckRestored(). */
    ~HexStateAssertRestored();

    /** Checks with assertions that the state is the same
        as it was at the last call to Init() or the constructor.
    */
    void AssertRestored();

    void Init(const HexState& state);

    /** Set to a state in which the destructor does not call
        AssertRestored() anymore. */
    void Clear();

private:
#ifndef NDEBUG
    HexState* m_state;

    HexState m_origState;
#endif // NDEBUG

    /** Not implemented. */
    HexStateAssertRestored(const HexStateAssertRestored&);

    /** Not implemented. */
    HexStateAssertRestored& operator=(const HexStateAssertRestored&);
};

inline HexStateAssertRestored::HexStateAssertRestored()
{
#ifndef NDEBUG
    m_state = 0;
#endif
}

inline HexStateAssertRestored::HexStateAssertRestored(const HexState& state)
{
    SG_DEBUG_ONLY(state);
#ifndef NDEBUG
    Init(state);
#endif
}

inline HexStateAssertRestored::~HexStateAssertRestored()
{
#ifndef NDEBUG
    AssertRestored();
#endif
}

inline void HexStateAssertRestored::AssertRestored()
{
#ifndef NDEBUG
    if (m_state == 0)
        return;
    SG_ASSERT(m_state->ToPlay() == m_origState.ToPlay());
    SG_ASSERT(m_state->Position() == m_origState.Position());
#endif
}

inline void HexStateAssertRestored::Clear()
{
#ifndef NDEBUG
    m_state = 0;
#endif
}

inline void HexStateAssertRestored::Init(const HexState& state)
{
    SG_DEBUG_ONLY(state);
#ifndef NDEBUG
    m_state = const_cast<HexState*>(&state);
    m_origState = state;
#endif
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // HEXSTATEASSERTRESTORED_HPP
