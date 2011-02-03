//----------------------------------------------------------------------------
/** @file HexState.hpp */
//----------------------------------------------------------------------------

#ifndef HEXSTATE_H
#define HEXSTATE_H

#include "StoneBoard.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Board position paired with a color to play. */
class HexState
{
public:
    HexState();

    explicit HexState(unsigned size);

    HexState(const StoneBoard& brd, HexColor toPlay);

    StoneBoard& Position();

    const StoneBoard& Position() const;

    void SetToPlay(HexColor color);

    HexColor ToPlay() const;

    SgHashCode Hash() const;

    void PlayMove(HexPoint move);
    
    void UndoMove(HexPoint move);

    bool operator==(const HexState& other) const;

    bool operator!=(const HexState& other) const;

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

inline SgHashCode HexState::Hash() const
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

inline bool HexState::operator==(const HexState& other) const
{
    return m_toPlay == other.m_toPlay
        && m_brd == other.m_brd;
}

inline bool HexState::operator!=(const HexState& other) const
{
    return !operator==(other);
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // HEXSTATE_H
