//----------------------------------------------------------------------------
/** @file HandCodedPattern.cpp */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "HandCodedPattern.hpp"
#include "BoardUtil.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

HandCodedPattern::HandCodedPattern()
    : m_dominatee(INVALID_POINT),
      m_dominator(INVALID_POINT)
{
}

HandCodedPattern::HandCodedPattern(HexPoint dominatee, HexPoint dominator)
    : m_dominatee(dominatee),
      m_dominator(dominator)
{
}

HandCodedPattern::~HandCodedPattern()
{
}
    
//----------------------------------------------------------------------------

void HandCodedPattern::set(HexColor color, const bitset_t& bs)
{
    BenzeneAssert(HexColorUtil::isBlackWhite(color));
    m_color[color] = bs;
}

void HandCodedPattern::rotate(const ConstBoard& brd)
{
    m_dominatee = BoardUtil::Rotate(brd, m_dominatee);
    m_dominator = BoardUtil::Rotate(brd, m_dominator);
    m_mask = BoardUtil::Rotate(brd, m_mask);
    for (BWIterator it; it; ++it)
        m_color[*it] = BoardUtil::Rotate(brd, m_color[*it]);
}

void HandCodedPattern::mirror(const ConstBoard& brd)
{
    m_dominatee = BoardUtil::Mirror(brd, m_dominatee);
    m_dominator = BoardUtil::Mirror(brd, m_dominator);
    m_mask = BoardUtil::Mirror(brd, m_mask);
    for (BWIterator it; it; ++it)
        m_color[*it] = BoardUtil::Mirror(brd, m_color[*it]);
}

void HandCodedPattern::flipColors()
{
    std::swap(m_color[BLACK], m_color[WHITE]);
}

bool HandCodedPattern::check(const StoneBoard& brd)
{
    for (BWIterator it; it; ++it)
        if (m_color[*it] != (brd.GetColor(*it) & m_mask))
            return false;
    return true;
}    

//----------------------------------------------------------------------------

void HandCodedPattern::CreatePatterns(std::vector<HandCodedPattern>& out)
{
    bitset_t bs;
    HandCodedPattern pat;

    //
    //
    // b3 dominates a3:
    //
    //   A B C D
    //   ----------
    // 1 \ . . . .
    //  2 \ . . .
    //   3 \ * !
    //
    HandCodedPattern pat1(HEX_CELL_A3, HEX_CELL_B3);
    bs.reset();
    bs.set(HEX_CELL_A1);
    bs.set(HEX_CELL_B1);
    bs.set(HEX_CELL_C1);
    bs.set(HEX_CELL_D1);
    bs.set(HEX_CELL_A2);
    bs.set(HEX_CELL_B2);
    bs.set(HEX_CELL_C2);
    bs.set(HEX_CELL_A3);
    bs.set(HEX_CELL_B3);
    pat1.setMask(bs);
    out.push_back(pat1);

    //
    //
    // With white c2, b3 dominates a3 and a4.
    //
    // a3 and a4 are actually vulnerable to b3!!
    //
    //   A B C
    //   ----------
    // 1 \ . . . 
    //  2 \ . . W
    //   3 \ * !
    //    4 \ *
    //
    HandCodedPattern pat3(HEX_CELL_A3, HEX_CELL_B3);
    bs.reset();
    bs.set(HEX_CELL_A1);
    bs.set(HEX_CELL_B1);
    bs.set(HEX_CELL_C1);
    bs.set(HEX_CELL_A2);
    bs.set(HEX_CELL_B2);
    bs.set(HEX_CELL_C2);
    bs.set(HEX_CELL_A3);
    bs.set(HEX_CELL_B3);
    bs.set(HEX_CELL_A4);
    pat3.setMask(bs);
    bs.reset();
    bs.set(HEX_CELL_C2);
    pat3.set(WHITE, bs);

    pat3.setDominatee(HEX_CELL_A3);
    out.push_back(pat3);

    pat3.setDominatee(HEX_CELL_A4);
    out.push_back(pat3);
}

//----------------------------------------------------------------------------
