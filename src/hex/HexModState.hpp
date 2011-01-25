//----------------------------------------------------------------------------
/** @file HexModState.hpp */
//----------------------------------------------------------------------------

#ifndef HEXMODBOARD_HPP
#define HEXMODBOARD_HPP

#include "HexStateAssertRestored.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Make a const board temporarily modifiable.
    Allows functions to use a const state for performing temporary
    operations (e.g. searches), as long as the state is the same state
    after the function is finished. This class facilitates
    const-correctness and encapsulation, because it allows the owner
    of a state, which is the only one who is allowed to do persistent
    changes on the board, to hand out only a const reference to other
    code. The other code can still use the board for temporary
    operations without needing a copy of the board.  HexModState does
    a const_cast from the const reference to a non-const reference in
    its constructor and checks with HexStateAssertRestored in its
    destructor that the board is returned in the same state.

    Example:
    @code
    // myFunction is not supposed to do persistent changes on the board
    // and therefore gets a const-reference. However it wants to use
    // the board temporarily
    void myFunction(const HexState& constState)
    {
        HexModState modState(constState);
        HexState& state = modState.State(); // get a nonconst-reference

        // ... play some moves and undo them

        // end of lifetime for modState, HexStateAssertRestored is
        // automatically called in the destructor of modBoard
    }
    @endcode

    There are also functions that allow to lock and unlock the board
    explicitly, for cases in which the period of temporary modifications
    cannot be mapped to the lifetime of a HexModState instance (e.g. because
    the period starts end ends in different functions).
*/
class HexModState
{
public:
    /** Constructor.
        Remembers the current board state.
        @param state The state
        @param locked Whether to start in locked mode (for explicit usage
        of Lock() and Unlock())
    */
    HexModState(const HexState& state, bool locked = false);

    /** Destructor.
        Checks with assertions that the board state is restored.
    */
    ~HexModState();

    /** Explicit conversion to non-const reference.
        This function triggers an assertion, if the board is currently in
        locked mode.
    */
    HexState& State() const;

    /** Automatic conversion to non-const reference.
        Allows to pass HexModState to functions that expect a non-const GoBoard
        reference without explicitely calling HexModState.Board().
        See State()
    */
    operator HexState&() const;

    /** Explicitly unlock the board. */
    void Unlock();

    /** Explicitly lock the board.
        Checks with assertions that the board state is restored.
        See Lock()
    */
    void Lock();

private:
    bool m_locked;

    HexState& m_state;

    HexStateAssertRestored m_assertRestored;
};

inline HexModState::HexModState(const HexState& state, bool locked)
    : m_locked(locked),
      m_state(const_cast<HexState&>(state)),
      m_assertRestored(state)
{
}

inline HexModState::~HexModState()
{
    // Destructor of m_assertRestored calls AssertRestored()
}

inline HexModState::operator HexState&() const
{
    return State();
}

inline HexState& HexModState::State() const
{
    SG_ASSERT(! m_locked);
    return m_state;
}

inline void HexModState::Unlock()
{
    m_assertRestored.Init(m_state);
    m_locked = false;
}

inline void HexModState::Lock()
{
    m_assertRestored.AssertRestored();
    m_assertRestored.Clear();
    m_locked = true;
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // HEXMODBOARD_HPP
