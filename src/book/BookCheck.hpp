//----------------------------------------------------------------------------
/** @file BookCheck.hpp */
//----------------------------------------------------------------------------

#ifndef BOOKCHECK_HPP
#define BOOKCHECK_HPP

#include "Book.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Returns best move from book. */
class BookCheck
{
public:

    BookCheck(boost::scoped_ptr<Book>& book);

    ~BookCheck();

    /** Returns best move from the book. Returns INVALID_POINT of no
        book or position not found in book. */
    HexPoint BestMove(const HexState& state);
    
    /** Ignore nodes with counts below this. */
    unsigned MinCount() const;

    /** See MinCount() */
    void SetMinCount(unsigned count);

    /** Weight used to choose best move. */
    float CountWeight() const;
    
    /** See CountWeight() */
    void SetCountWeight(float factor);

private:
    boost::scoped_ptr<Book>& m_book;

    /** See MinCount() */
    unsigned m_minCount;

    /** See CountWeight() */
    float m_countWeight;
};

inline unsigned BookCheck::MinCount() const
{
    return m_minCount;
}

inline void BookCheck::SetMinCount(unsigned count)
{
    m_minCount = count;
}

inline float BookCheck::CountWeight() const
{
    return m_countWeight;
}
    
inline void BookCheck::SetCountWeight(float weight)
{
    m_countWeight= weight;
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // BOOKCHECK_HPP
