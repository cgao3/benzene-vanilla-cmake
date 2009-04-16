//----------------------------------------------------------------------------
/** @file BookCheck.hpp
 */
//----------------------------------------------------------------------------

#ifndef BOOKCHECK_HPP
#define BOOKCHECK_HPP

#include "BenzenePlayer.hpp"
#include "OpeningBook.hpp"

//----------------------------------------------------------------------------

/** Checks book before search. */
class BookCheck : public BenzenePlayerFunctionality
{
public:

    /** Adds book check to the given player. */
    BookCheck(BenzenePlayer* player);

    /** Destructor. */
    virtual ~BookCheck();

    /** Checks the book for the current state if "player-use-book" is
        true. If state found in book, returns book move. Otherwise
        calls player's pre_search() and returns the move it returns.
    */
    virtual HexPoint pre_search(HexBoard& brd, const Game& game_state,
				HexColor color, bitset_t& consider,
                                double time_remaining, double& score);
    
    bool Enabled() const;
    
    void SetEnabled(bool enable);

    int MinDepth() const;

    void SetMinDepth(int depth);

    int MaxDepth() const;
    
    void SetMaxDepth(int depth);
    
    float DepthValueAdjustment() const;
    
    void SetDepthValueAdjustment(float value);

private:

    /** Stores the names and alpha-values of all opening books. */
    void LoadOpeningBooks();
    
    /** Uses the opening books to determine a response (if possible).
        @return INVALID_POINT on failure, valid move on success.
    */
    HexPoint BookResponse(HexBoard& brd, HexColor color);
    
    void ComputeBestChild(StoneBoard& brd, HexColor color,
			  const OpeningBook& book, HexPoint& move,
			  float& score);
    
    std::vector<std::string> m_openingBookFiles;

    bool m_openingBookLoaded;

    bool m_enabled;

    int m_min_depth;
    
    int m_max_depth;

    float m_depth_adjustment;
};

inline bool BookCheck::Enabled() const
{
    return m_enabled;
}
    
inline void BookCheck::SetEnabled(bool enable)
{
    m_enabled = enable;
}

inline int BookCheck::MinDepth() const
{
    return m_min_depth;
}

inline void BookCheck::SetMinDepth(int depth)
{
    m_min_depth = depth;
}

inline int BookCheck::MaxDepth() const
{
    return m_max_depth;
}
    
inline void BookCheck::SetMaxDepth(int depth)
{
    m_max_depth = depth;
}

inline float BookCheck::DepthValueAdjustment() const
{
    return m_depth_adjustment;
}
    
inline void BookCheck::SetDepthValueAdjustment(float value)
{
    m_depth_adjustment = value;
}

//----------------------------------------------------------------------------

#endif // BOOKCHECK_HPP
