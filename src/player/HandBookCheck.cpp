//----------------------------------------------------------------------------
/** HandBookCheck.cpp
 */
//----------------------------------------------------------------------------

#include "HandBookCheck.hpp"
#include "BenzenePlayer.hpp"

#include <boost/filesystem/path.hpp>

using namespace benzene;

//----------------------------------------------------------------------------

HandBookCheck::HandBookCheck(BenzenePlayer* player)
    : BenzenePlayerFunctionality(player),
      m_enabled(false),
      m_handBookLoaded(false)
{
}

HandBookCheck::~HandBookCheck()
{
}

HexPoint HandBookCheck::pre_search(HexBoard& brd, const Game& game_state,
				   HexColor color, bitset_t& consider,
				   double max_time, double& score)
{
    if (m_enabled) 
    {
	HexPoint response = HandBookResponse(brd, color);
        if (response != INVALID_POINT)
            return response;
    }
    return m_player->pre_search(brd, game_state, color, consider,
				max_time, score);
}

//----------------------------------------------------------------------------

/** Reads in the hand-book from a file. 
    Checks for duplicates and does not add if an entry with that hash
    already exists, ie, the first hash is always used.
*/
void HandBookCheck::LoadHandBook()
{
    LogInfo() << "HandBookCheck: Loading book..." << '\n';
    
    // Find hand book file
    using namespace boost::filesystem;
    path p = path(ABS_TOP_SRCDIR) / "share" / "hand-book.txt";
    p.normalize();
    std::string file = p.native_file_string();
    
    // Open file if exists, else abort
    std::ifstream f;
    f.open(file.c_str());
    if (!f) {
        LogWarning() << "Could not open file '" << file << "'!" << '\n';
        return;
    }
    
    // Extract (hash, response) pairs from hand book
    std::string line;
    while (getline(f, line)) 
    {
	std::istringstream iss;
	iss.str(line);
	std::string hash;
	iss >> hash;
	if (hash[0] != '#') 
        {
	    std::string response;
	    iss >> response;
            
	    HexPoint move = HexPointUtil::fromString(response);
	    HexAssert(move != INVALID_POINT);

            if (m_response.count(hash))
                LogWarning() << "Duplicate entry in book: " << hash << '\n';
            else
                m_response[hash] = move;
	}
    }
    m_handBookLoaded = true;
    LogInfo() << "HandBookCheck: Found " << m_response.size() << " states." << '\n';
}

/** Uses the hand book to determine a response (if possible).
    @return INVALID_POINT on failure, valid move on success.
*/
HexPoint HandBookCheck::HandBookResponse(const StoneBoard& brd, HexColor color)
{
    UNUSED(color);

    if (!m_handBookLoaded)
	LoadHandBook();
    
    LogInfo() << "HandBookCheck: Seeking " 
              << HashUtil::toString(brd.Hash()) << '\n';
    
    std::string key = HashUtil::toString(brd.Hash());
    if (m_response.count(key))
    {
        HexPoint response = m_response[key];
        LogInfo() << "HandBookCheck: response = " << response << '\n';
        HexAssert(response != INVALID_POINT);
        HexAssert(brd.isEmpty(response));
        return response;
    }
    
    LogInfo() << "HandBookCheck: No response found." << '\n';
    return INVALID_POINT;
}

//----------------------------------------------------------------------------
