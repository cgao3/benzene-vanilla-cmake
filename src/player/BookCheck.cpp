//----------------------------------------------------------------------------
/** @file BookCheck.cpp
 */
//----------------------------------------------------------------------------

#include "BoardUtils.hpp"
#include "BookCheck.hpp"
#include "BitsetIterator.hpp"
#include "BenzenePlayer.hpp"

#include <boost/filesystem/path.hpp>

using namespace benzene;

//----------------------------------------------------------------------------

BookCheck::BookCheck(BenzenePlayer* player)
    : BenzenePlayerFunctionality(player),
      m_book(0),
      m_bookLoaded(false),
      m_enabled(false),
      m_min_count(1),
      m_count_weight(0.02)
{
}

BookCheck::~BookCheck()
{
}

HexPoint BookCheck::pre_search(HexBoard& brd, const Game& game_state,
			       HexColor color, bitset_t& consider,
                               double time_remaining, double& score)
{
    if (m_enabled) 
    {
        HexPoint response = INVALID_POINT;
        LoadOpeningBook(brd.Const());
        if (m_bookLoaded)
            response = OpeningBookUtil::BestMove(*m_book, brd, m_min_count,
                                                 m_count_weight);
        if (response != INVALID_POINT)
            return response;
    }
    return m_player->pre_search(brd, game_state, color, consider,
				time_remaining, score);
}

//----------------------------------------------------------------------------

void BookCheck::LoadOpeningBook(const ConstBoard& brd)
{
    if (m_bookLoaded)
        return;
    using namespace boost::filesystem;
    path book_path = path(ABS_TOP_SRCDIR) / "share";
    path books_list = book_path / "book.db";
    books_list.normalize();
    std::string book_file = books_list.native_file_string();
    try {
        m_book.reset(new OpeningBook(brd.width(), brd.height(), book_file));
        m_bookLoaded = true;
    }
    catch (HexException& e) {
        LogWarning() << "BookCheck: could not open book!" << '\n';
    }
}

//----------------------------------------------------------------------------
