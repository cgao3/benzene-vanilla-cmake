//----------------------------------------------------------------------------
/** @file HandCodedPattern.hpp
 */
//----------------------------------------------------------------------------

#ifndef HAND_CODED_PATTERN_HPP
#define HAND_CODED_PATTERN_HPP

#include "Hex.hpp"
#include "StoneBoard.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Special patterns that are too big to check with a PatternBoard (to
    do so would mean increasing Pattern::MAX_EXTENSION, thus slowing
    down PatternBoard::update()).  These are static patterns -- so
    they are not translated.

    @todo Want hand-coded vulnerable patterns, not just dominated
*/
class HandCodedPattern
{        
public: 

    /** Constructor, sets dominator and dominatee to
        INVALID_POINT.  */
    HandCodedPattern();

    /** Constructor. */
    explicit HandCodedPattern(HexPoint dominatee, HexPoint dominator);

    /** Destructor. */
    ~HandCodedPattern();

    //----------------------------------------------------------------------

    /** Adds all HandCodedPatterns into out. */
    static void CreatePatterns(std::vector<HandCodedPattern>& out);

    //----------------------------------------------------------------------

    /** Sets the dominated cell. */
    void setDominatee(HexPoint dominatee);

    /** Returns the dominated cell. */
    HexPoint dominatee() const;
    
    /** Sets the dominator. */
    void setDominator(HexPoint dominator);

    /** Returns the dominator. */
    HexPoint dominator() const;

    /** Sets the mask for the pattern. */
    void setMask(const bitset_t& bs);
    
    /** Sets the bitset for the given color. */
    void set(HexColor color, const bitset_t& bs);

    /** Rotates the pattern on the given board. */
    void rotate(const ConstBoard& brd);

    /** Mirrors the pattern on the given board. */
    void mirror(const ConstBoard& brd);

    /** Flips black to white and vice versa. */
    void flipColors();

    /** True if pattern matches board. */
    bool check(const StoneBoard& brd);
    
private:
    HexPoint m_dominatee, m_dominator;
    bitset_t m_mask;
    bitset_t m_color[BLACK_AND_WHITE];
};

inline void HandCodedPattern::setDominatee(HexPoint dominatee)
{
    m_dominatee = dominatee;
}

inline HexPoint HandCodedPattern::dominatee() const
{
    return m_dominatee;
}

inline void HandCodedPattern::setDominator(HexPoint dominator)
{
    m_dominator = dominator;
}

inline HexPoint HandCodedPattern::dominator() const
{
    return m_dominator;
}

inline void HandCodedPattern::setMask(const bitset_t& mask) 
{
    m_mask = mask;
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif  // HAND_CODED_PATTERN_HPP
