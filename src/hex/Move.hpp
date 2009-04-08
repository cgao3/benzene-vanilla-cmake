//----------------------------------------------------------------------------
// $Id: Move.hpp 1426 2008-06-04 21:18:22Z broderic $
//----------------------------------------------------------------------------

#ifndef MOVE_HPP
#define MOVE_HPP

#include "Hex.hpp"

//----------------------------------------------------------------------------

/** A (HexColor, HexPoint) pair. */
class Move
{
public:

    /** Creates the move (color, point). */
    Move(HexColor color, HexPoint point);
    
    /** Returns color of move. */
    HexColor color() const;

    /** Returns point of move. */
    HexPoint point() const;
    
private:
    HexColor m_color;
    HexPoint m_point;
};

inline Move::Move(HexColor color, HexPoint point)
    : m_color(color), m_point(point)
{
}

inline HexColor Move::color() const
{
    return m_color;
}

inline HexPoint Move::point() const
{
    return m_point;
}

//----------------------------------------------------------------------------

#endif // MOVE_HPP
