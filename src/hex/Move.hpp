//----------------------------------------------------------------------------
/** @file Move.hpp */
//----------------------------------------------------------------------------

#ifndef MOVE_HPP
#define MOVE_HPP

#include "Hex.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** A (HexColor, HexPoint) pair. */
class Move
{
public:

    /** Creates the move (color, point). */
    Move(HexColor color, HexPoint point);
    
    /** Returns color of move. */
    HexColor Color() const;

    /** Returns point of move. */
    HexPoint Point() const;

    /** Outputs a [color, move] string. */
    std::string ToString() const;

    /** Returns true if point and color are equal. */
    bool operator==(const Move& other) const;

    /** Returns true !operator==(other). */
    bool operator!=(const Move& other) const;

private:
    HexColor m_color;

    HexPoint m_point;
};

inline Move::Move(HexColor color, HexPoint point)
    : m_color(color),
      m_point(point)
{
}

inline HexColor Move::Color() const
{
    return m_color;
}

inline HexPoint Move::Point() const
{
    return m_point;
}

inline std::string Move::ToString() const
{
    std::ostringstream os;
    os << '[' << m_color << ", " << m_point << ']';
    return os.str();
}

inline bool Move::operator==(const Move& other) const
{
    return m_color == other.m_color
        && m_point == other.m_point;
}

inline bool Move::operator!=(const Move& other) const
{
    return !operator==(other);
}

//----------------------------------------------------------------------------

/** Extends standard output operator to Moves. */
inline std::ostream& operator<<(std::ostream& os, const Move& move)
{
    return os << move.ToString();
}

//----------------------------------------------------------------------------

/** Sequence of moves. */
typedef std::vector<Move> MoveSequence;

/** Extends standard output operator for MoveSequences. */
inline std::ostream& operator<<(std::ostream& os, const MoveSequence& move)
{
    for (std::size_t i = 0; i < move.size(); ++i)
        os << move[i] << ' ';
    return os;
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // MOVE_HPP
