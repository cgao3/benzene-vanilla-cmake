//----------------------------------------------------------------------------
/** @file HexState.hpp
 */
//----------------------------------------------------------------------------

#ifndef HEXSTATE_H
#define HEXSTATE_H

#include "Hex.hpp"
#include "StoneBoard.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Board position paired with a color to play. */
class HexState
{
public:
    HexState();

    HexState(unsigned size);

    HexState(const StoneBoard& brd, HexColor toPlay);

    StoneBoard& Position();

    const StoneBoard& Position() const;

    void SetToPlay(HexColor color);

    HexColor ToPlay() const;

    hash_t Hash() const;

    void PlayMove(HexPoint move);
    
    void UndoMove(HexPoint move);

private:
    StoneBoard m_brd;

    HexColor m_toPlay;

    void FlipColorToPlay();
};

inline HexState::HexState()
{
}

inline HexState::HexState(unsigned size)
    : m_brd(size),
      m_toPlay(m_brd.WhoseTurn())
{
}

inline HexState::HexState(const StoneBoard& brd, HexColor toPlay)
    : m_brd(brd),
      m_toPlay(toPlay)
{
}

inline StoneBoard& HexState::Position()
{
    return m_brd;
}

inline const StoneBoard& HexState::Position() const
{
    return m_brd;
}

inline void HexState::SetToPlay(HexColor toPlay)
{
    m_toPlay = toPlay;
}

inline HexColor HexState::ToPlay() const
{
    return m_toPlay;
}

inline hash_t HexState::Hash() const
{
    return m_brd.Hash(m_toPlay);
}

inline void HexState::PlayMove(HexPoint move)
{
    m_brd.PlayMove(m_toPlay, move);
    FlipColorToPlay();
}

inline void HexState::UndoMove(HexPoint move)
{
    m_brd.UndoMove(move);
    FlipColorToPlay();
}

inline void HexState::FlipColorToPlay()
{
    m_toPlay = !m_toPlay;
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // HEXSTATE_H
