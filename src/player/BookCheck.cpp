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
      m_openingBookLoaded(false),
      m_enabled(false),
      m_min_depth(3),
      m_max_depth(16),
      m_depth_adjustment(0.005)
{
}

BookCheck::~BookCheck()
{
}

HexPoint BookCheck::pre_search(HexBoard& brd, const Game& game_state,
			       HexColor color, bitset_t& consider,
                               double time_remaining, double& score)
{
    if (m_enabled && m_max_depth > (int)game_state.History().size()) {
	HexPoint response = BookResponse(brd, color);
        if (response != INVALID_POINT)
            return response;
    }
    return m_player->pre_search(brd, game_state, color, consider,
				time_remaining, score);
}

//----------------------------------------------------------------------------

void BookCheck::LoadOpeningBooks()
{
    // find and open the file with the names of all opening books
    using namespace boost::filesystem;
    path book_path = path(ABS_TOP_SRCDIR) / "share";
    path books_list = book_path / "opening-books.txt";
    books_list.normalize();
    std::string books_file = books_list.native_file_string();

    // open file if exists, else abort
    std::ifstream f;
    f.open(books_file.c_str());
    if (!f) {
        LogWarning() << "Could not open file '" << books_file << "'!" << '\n';
        return;
    }
    
    // extract opening book file names/alpha values
    std::string line;
    while (getline(f, line)) 
    {
	std::istringstream iss;
	iss.str(line);
	std::string name;
	iss >> name;
        path bookfile = book_path / name;
        bookfile.normalize();
	m_openingBookFiles.push_back(bookfile.native_file_string());
    }
    // note: file closes automatically
    
    m_openingBookLoaded = true;
}

HexPoint BookCheck::BookResponse(HexBoard& brd, HexColor color)
{
    if (!m_openingBookLoaded)
	LoadOpeningBooks();
    
    LogInfo() << "BookCheck: Searching books for response"
	     << '\n';
    
    HexPoint bestMove = INVALID_POINT;
    float bestScore = -1e10;
    
    // scan opening book files for the best move beyond min depth
    for (unsigned i=0; i<m_openingBookFiles.size(); i++) 
    {
	OpeningBook book(brd.width(), brd.height(), m_openingBookFiles[i]);
	
	// See if can find a better-rated move in this opening book,
	// checking both the board and its rotation
	float score;
	HexPoint move;
	ComputeBestChild(brd, color, book, move, score);
	if (score > bestScore) 
        {
	    bestScore = score;
	    bestMove = move;
	    LogInfo() << "New best:"
		      << " " << m_openingBookFiles[i]
		      << " " << HexPointUtil::toString(bestMove)
		      << " " << bestScore << '\n';
	}
    }
    
    return bestMove;
}

void BookCheck::ComputeBestChild(StoneBoard& brd, HexColor color,
                                 const OpeningBook& book, HexPoint& move,
                                 float& score)
{
    score = -1e10;
    move = INVALID_POINT;
    
    // if book does not contain state with a minimum depth, just quit
    if (book.GetMainLineDepth(brd) <= m_min_depth) 
        return;

    for (BitsetIterator p(brd.getEmpty()); p; ++p) 
    {
	brd.playMove(color, *p);
	int depth = book.GetMainLineDepth(brd);
	if (depth >= m_min_depth) 
        {
            OpeningBookNode node;
            book.GetNode(brd, node);
	    float curScore = OpeningBook::InverseEval(node.m_value);
	    curScore += depth * m_depth_adjustment;
	    if (curScore > score) 
            {
		score = curScore;
		move = *p;
	    }
	}
        brd.undoMove(*p);
    }
}

//----------------------------------------------------------------------------
