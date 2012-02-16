//----------------------------------------------------------------------------
/** @file HexColor.hpp */
//----------------------------------------------------------------------------

#ifndef HEXCOLOR_HPP
#define HEXCOLOR_HPP

#include <string>
#include "Benzene.hpp"
#include "BenzeneAssert.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Available colors of a cell on a Hex board. 
    
    @note BLACK=0 and WHITE=1 is currently assumed by many pieces of code. 
*/
typedef enum { BLACK=0, WHITE=1, EMPTY=2 } HexColor;

//----------------------------------------------------------------------------

/** Color of player to move first in a game of Hex. */
static const HexColor FIRST_TO_PLAY = BLACK;

//----------------------------------------------------------------------------

/** Color of player who is trying to form a vertical chain (i.e. joining
    NORTH to SOUTH). */
static const HexColor VERTICAL_COLOR = BLACK;

//----------------------------------------------------------------------------

/** Color that all dead cells are set to. */
static const HexColor DEAD_COLOR = BLACK;

//----------------------------------------------------------------------------

/** Constant to denote an array to be indexed only by BLACK and
    WHITE. */
static const int BLACK_AND_WHITE = 2;

/** Constant to denote an array to be indexed by BLACK, WHITE, and
    EMPTY. */
static const int BLACK_WHITE_EMPTY = 3;

//----------------------------------------------------------------------------

/** Iterator over BLACK and WHITE. */
class BWIterator
{
public:
    BWIterator()
        : m_color(BLACK)
    { }

    /** Advance the state of the iteration to the next element. */
    void operator++()
    {
        ++m_color;
    }

    /** Return the value of the current element. */
    HexColor operator*() const
    {
        return static_cast<HexColor>(m_color);
    }

    /** Return true if iteration is valid, otherwise false. */
    operator bool() const
    {
        return m_color <= WHITE;
    }

private:
    int m_color;

    /** Not implemented */
    BWIterator(const BWIterator&);

    /** Not implemented */
    BWIterator& operator=(const BWIterator&);
};

/** Iterator over BLACK and WHITE and EMPTY. */
class ColorIterator
{
public:
    ColorIterator()
        : m_color(BLACK)
    { }

    /** Advance the state of the iteration to the next element. */
    void operator++()
    {
        ++m_color;
    }

    /** Return the value of the current element. */
    HexColor operator*() const
    {
        return static_cast<HexColor>(m_color);
    }

    /** Return true if iteration is valid, otherwise false. */
    operator bool() const
    {
        return m_color <= EMPTY;
    }

private:
    int m_color;

    /** Not implemented */
    ColorIterator(const ColorIterator&);

    /** Not implemented */
    ColorIterator& operator=(const ColorIterator&);
};

//----------------------------------------------------------------------------

/** Basic HexColor utilities. */
namespace HexColorUtil 
{

/** Returns true if color is one of BLACK, WHITE, or EMPTY. */
inline bool isValidColor(HexColor color)
{
    return (color == BLACK || color == WHITE) || (color == EMPTY);
}

/** Returns true if color is BLACK or WHITE. */
inline bool isBlackWhite(HexColor color)
{
    return (color == BLACK || color == WHITE);
}

/** Returns a string representation of the given HexColor. */
inline std::string toString(HexColor color)
{
    BenzeneAssert(isValidColor(color));
    if (color == BLACK) return "black";
    if (color == WHITE) return "white";
    return "empty";
}

/** Returns the opposite color for BLACK and WHITE, EMPTY for EMPTY. */
inline HexColor otherColor(HexColor color)
{
    BenzeneAssert(isValidColor(color));
    if (color == EMPTY) return EMPTY;
    return (color == WHITE) ? BLACK : WHITE;
}

} // namespace HexColorUtil

//----------------------------------------------------------------------------

/** Overrides the standard "<<" operator; same as
    HexColorUtil::toString(). */
inline std::ostream& operator<<(std::ostream& os, HexColor color)
{
    os << HexColorUtil::toString(color);
    return os;
}

/** Overrides the standard "!" operator to be the same as
    HexColorUtil::otherColor(). */
inline HexColor operator!(HexColor color)
{
    return HexColorUtil::otherColor(color);
}

//----------------------------------------------------------------------------

/** All possible sets of available colors. */
typedef enum { BLACK_ONLY, WHITE_ONLY, EMPTY_ONLY, 
               NOT_BLACK, NOT_WHITE, NOT_EMPTY, 
               ALL_COLORS, NUM_COLOR_SETS } HexColorSet;

/** Utilities on HexColorSets. */
namespace HexColorSetUtil
{

/** Returns true if colorset is a valid HexColorSet. */
inline bool isValid(HexColorSet colorset)
{
    return (colorset >= BLACK_ONLY && colorset <= ALL_COLORS);
}

/** Converts a HexColorSet to a string. */
inline std::string toString(HexColorSet colorset)
{
    BenzeneAssert(isValid(colorset));
    if (colorset == BLACK_ONLY) return "black_only";
    if (colorset == WHITE_ONLY) return "white_only";
    if (colorset == EMPTY_ONLY) return "empty_only";
    if (colorset == NOT_BLACK) return "not_black";
    if (colorset == NOT_WHITE) return "not_white";
    if (colorset == NOT_EMPTY) return "not_empty";
    return "all_colors";
}

/** Converts a string into a HexColorSet. */
inline HexColorSet fromString(std::string str)
{
    if (str == "black_only") return BLACK_ONLY;
    if (str == "white_only") return WHITE_ONLY;
    if (str == "empty_only") return EMPTY_ONLY;
    if (str == "not_black") return NOT_BLACK;
    if (str == "not_white") return NOT_WHITE;
    if (str == "not_empty") return NOT_EMPTY;
    if (str == "all_colors") return ALL_COLORS;
    BenzeneAssert(false);
    return ALL_COLORS;
}
    
/** Returns true if color is in colorset. */
inline bool InSet(HexColor color, HexColorSet colorset)
{
    BenzeneAssert(HexColorUtil::isValidColor(color));
    BenzeneAssert(HexColorSetUtil::isValid(colorset));
 
    switch(colorset) {
    case BLACK_ONLY: return (color == BLACK);
    case WHITE_ONLY: return (color == WHITE);
    case EMPTY_ONLY: return (color == EMPTY);
    case  NOT_BLACK: return (color != BLACK);
    case  NOT_WHITE: return (color != WHITE);
    case  NOT_EMPTY: return (color != EMPTY);
    case ALL_COLORS: return true;
    default:
        BenzeneAssert(false);
    }
    return true;
}

/** Returns the HexColorSet composed only of color. */
inline HexColorSet Only(HexColor color)
{
    BenzeneAssert(HexColorUtil::isValidColor(color));
    if (color == BLACK) 
        return BLACK_ONLY;
    else if (color == WHITE)
        return WHITE_ONLY;
    return EMPTY_ONLY;
}

/** Returns the HexColorSet containing all but color. */
inline HexColorSet NotColor(HexColor color)
{
    BenzeneAssert(HexColorUtil::isValidColor(color));
    if (color == BLACK) 
        return NOT_BLACK;
    else if (color == WHITE)
        return NOT_WHITE;
    return NOT_EMPTY;
}

/** Returns the HexColorSet containing color or empty; equivalent to:
    @code NotColor(HexColorUtil::otherColor(color)). @endcode */
inline HexColorSet ColorOrEmpty(HexColor color)
{
    BenzeneAssert(HexColorUtil::isValidColor(color));
    if (color == BLACK)
        return NOT_WHITE;
    else if (color == WHITE)
        return NOT_BLACK;
    return EMPTY_ONLY;
}

} // namespace HexColorSetUtil

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // HEXCOLOR_HPP
