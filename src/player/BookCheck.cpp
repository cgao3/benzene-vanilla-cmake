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
      m_bookName("book.db"),
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
                               double max_time, double& score)
{
    if (m_enabled) 
    {
        HexPoint response = INVALID_POINT;
        if (!m_bookLoaded)
            LoadOpeningBook(m_bookName);
        if (m_bookLoaded)
            response = BookUtil::BestMove(*m_book, brd.GetState(), m_min_count,
                                          m_count_weight);
        if (response != INVALID_POINT)
            return response;
    }
    return m_player->pre_search(brd, game_state, color, consider,
				max_time, score);
}

//----------------------------------------------------------------------------

void BookCheck::SetBookName(const std::string& name)
{
    m_bookName = name;
    if (m_bookLoaded)
    {
        m_book.reset(0);
        m_bookLoaded = false;
        LoadOpeningBook(name);
    }
}

void BookCheck::LoadOpeningBook(const std::string& name)
{
    using namespace boost::filesystem;
    path book_path = path(ABS_TOP_SRCDIR) / "share";
    path books_list = book_path / name;
    books_list.normalize();
    std::string book_file = books_list.native_file_string();
    try {
        LogInfo() << "Loading book: '" << book_file << "'...\n";
        m_book.reset(new Book(book_file));
        m_bookLoaded = true;
    }
    catch (HexException& e) {
        LogWarning() << "BookCheck: could not open book!" << '\n';
    }
}

//----------------------------------------------------------------------------
