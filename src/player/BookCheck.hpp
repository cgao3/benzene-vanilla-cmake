//----------------------------------------------------------------------------
/** @file BookCheck.hpp
 */
//----------------------------------------------------------------------------

#ifndef BOOKCHECK_HPP
#define BOOKCHECK_HPP

#include "BenzenePlayer.hpp"
#include "OpeningBook.hpp"

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
    virtual HexPoint pre_search(HexBoard& brd, const Game& game_state,
				HexColor color, bitset_t& consider,
                                double max_time, double& score);
    
    bool Enabled() const;
    
    void SetEnabled(bool enable);

    /** Ignore nodes with counts below this. */
    unsigned MinCount() const;

    /** See MinCount() */
    void SetMinCount(int count);

    /** Weight used to choose best move. */
    float CountWeight() const;
    
    /** See CountWeight() */
    void SetCountWeight(float factor);

private:

    boost::scoped_ptr<OpeningBook> m_book;

    bool m_bookLoaded;

    bool m_enabled;
  
    /** See MinCount() */
    unsigned m_min_count;

    /** See CountWeight() */
    float m_count_weight;

    void LoadOpeningBook(const ConstBoard& brd);
};

inline bool BookCheck::Enabled() const
{
    return m_enabled;
}
    
inline void BookCheck::SetEnabled(bool enable)
{
    m_enabled = enable;
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
