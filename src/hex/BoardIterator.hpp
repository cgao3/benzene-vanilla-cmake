//----------------------------------------------------------------------------
/** @file
 */
//----------------------------------------------------------------------------

#ifndef BOARD_ITERATOR_HPP
#define BOARD_ITERATOR_HPP

#include "Benzene.hpp"
#include "Hex.hpp"
#include "SafeBool.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------

/**
 *  Iterates on an array of HexPoints.  The array must end with
 *  INVALID_POINT, otherwise BoardIterator will just keep on truckin'.
 *
 *  BoardIterator implements the bool operator, so we can do stuff 
 *  like this:
 *
 *    @code for (BoardIterator i(brd.cells()); i; ++i) {...} @endcode
 *
 *  (note the "i" test for validity).  operator bool() returns false
 *  if the iterator is currently pointing at INVALID_POINT and true
 *  otherwise. 
 *
 *  Because BoardIterator uses operator bool(), comparisons like @code
 *  if (x == y) @endcode will do very different things that you would
 *  think. To disallow iterator comparisons like the above (instead
 *  use @code if (*x == *y) @endcode ), we have employed the Safe Bool
 *  Idiom.  Comparing two BoardIterators will result in a compilation
 *  error.
 */
class BoardIterator : public SafeBool<BoardIterator>
{
public:

    /** Empty iterator. */
    BoardIterator();

    /** Destructor. */
    ~BoardIterator();

    /** Iterates over the vector of points begining with start. */
    BoardIterator(const HexPoint* start);

    /** Iterates over the vector of points. */
    BoardIterator(const std::vector<HexPoint>& start);

    /** Returns the HexPoint at the current location. */
    HexPoint operator*() const;

    /** Move to the next point in the list.  Incrementing past
        an INVALID_POINT gives undefined behavoir. */
    BoardIterator& operator++();

    /** Used by the SafeBool Idiom. */
    bool boolean_test() const;

protected:
    const HexPoint* m_point;
};

inline BoardIterator::BoardIterator() 
    : m_point(NULL)
{
}

inline BoardIterator::BoardIterator(const HexPoint* start) 
    : m_point(start)
{
}

inline BoardIterator::BoardIterator(const std::vector<HexPoint>& start) 
    : m_point(&start[0])
{
}

inline BoardIterator::~BoardIterator()
{
}

inline HexPoint BoardIterator::operator*() const
{
    return *m_point;
}

inline bool BoardIterator::boolean_test() const
{
    return (*m_point != INVALID_POINT);
}

inline BoardIterator& BoardIterator::operator++() 
{
    ++m_point;
    return (*this);
}

//----------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // BOARD_ITERATOR_HPP
