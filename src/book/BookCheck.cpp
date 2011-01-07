//----------------------------------------------------------------------------
/** @file BookCheck.cpp
 */
//----------------------------------------------------------------------------

#include "BookCheck.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

BookCheck::BookCheck(boost::scoped_ptr<Book>& book)
    : m_book(book),
      m_minCount(1),
      m_countWeight(0.02f)
{
}

BookCheck::~BookCheck()
{
}

HexPoint BookCheck::BestMove(const HexState& state)
{
    if (m_book.get() == 0)
        return INVALID_POINT;
    HexPoint bookMove = BookUtil::BestMove(*m_book, state, m_minCount, 
                                           m_countWeight);
    if (bookMove != INVALID_POINT)
        LogInfo() << "BookCheck: playing move " << bookMove << '\n';
    return bookMove;
}

//----------------------------------------------------------------------------
