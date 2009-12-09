//----------------------------------------------------------------------------
/** @file BookCheck.hpp
 */
//----------------------------------------------------------------------------

#ifndef BOOKCHECK_HPP
#define BOOKCHECK_HPP

#include "BenzenePlayer.hpp"
#include "Book.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Checks book before search. */
class BookCheck : public BenzenePlayerFunctionality
{
public:

    /** Adds book check to the given player. */
    BookCheck(BenzenePlayer* player);

    /** Destructor. */
    virtual ~BookCheck();

    /** Checks the book for the current state if Enabled() is
        true. If state is found in book, returns best book move. Otherwise
        calls player's pre_search() and returns the move it returns.
    */
    virtual HexPoint PreSearch(HexBoard& brd, const Game& game_state,
                               HexColor color, bitset_t& consider,
                               double max_time, double& score);
    
    bool Enabled() const;
    
    void SetEnabled(bool enable);

    /** Name of book to open. */
    std::string BookName() const;

    /** See BookName(). Will set name of book that will be opened if
        no book is currently opened, or close and attempt to open a
        new book if one is already open.  */
    void SetBookName(const std::string& name);

    /** Ignore nodes with counts below this. */
    unsigned MinCount() const;

    /** See MinCount() */
    void SetMinCount(int count);

    /** Weight used to choose best move. */
    float CountWeight() const;
    
    /** See CountWeight() */
    void SetCountWeight(float factor);

private:

    std::string m_bookName;

    boost::scoped_ptr<Book> m_book;

    bool m_bookLoaded;

    bool m_enabled;
  
    /** See MinCount() */
    unsigned m_min_count;

    /** See CountWeight() */
    float m_count_weight;

    void LoadOpeningBook(const std::string& name);
};

inline bool BookCheck::Enabled() const
{
    return m_enabled;
}
    
inline void BookCheck::SetEnabled(bool enable)
{
    m_enabled = enable;
}

inline std::string BookCheck::BookName() const
{
    return m_bookName;
}

inline unsigned BookCheck::MinCount() const
{
    return m_min_count;
}

inline void BookCheck::SetMinCount(int count)
{
    m_min_count = count;
}

inline float BookCheck::CountWeight() const
{
    return m_count_weight;
}
    
inline void BookCheck::SetCountWeight(float weight)
{
    m_count_weight= weight;
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // BOOKCHECK_HPP
