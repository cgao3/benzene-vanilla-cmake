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
				   double time_remaining, double& score)
{
    if (m_enabled) 
    {
	HexPoint response = HandBookResponse(brd, color);
        if (response != INVALID_POINT)
            return response;
    }
    return m_player->pre_search(brd, game_state, color, consider,
				time_remaining, score);
}

//----------------------------------------------------------------------------

void HandBookCheck::LoadHandBook()
{
    LogInfo() << "Loading hand book" << '\n';
    
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
    
    // Extract board ID/response pairs from hand book
    std::string line;
    while (getline(f, line)) 
    {
	std::istringstream iss;
	iss.str(line);
	std::string hash;
	iss >> hash;
	// Comment lines should be ignored
	if (hash[0] != '#') 
        {
	    m_id.push_back(hash);
	    std::string response;
	    iss >> response;
            
	    HexPoint p = HexPointUtil::fromString(response);
	    HexAssert(p != INVALID_POINT);
	    m_response.push_back(p);
	}
    }
    HexAssert(m_id.size() == m_response.size());
    // note: file closes automatically
    
    m_handBookLoaded = true;
}

HexPoint HandBookCheck::HandBookResponse(const StoneBoard& brd, 
                                         HexColor color)
{
    UNUSED(color);

    if (!m_handBookLoaded)
	LoadHandBook();
    
    LogInfo() << "HandBookCheck: Seeking response" << '\n'
	      << "Hash: " << HashUtil::toString(brd.Hash()) << '\n';
    
    for (std::size_t i = 0; i < m_id.size(); ++i) 
    {
	if (HashUtil::toString(brd.Hash()) == m_id[i]) 
        {
	    LogInfo() << "Found hand book response!" << '\n';
	    HexAssert(m_response[i] != INVALID_POINT);
	    HexAssert(brd.isEmpty(m_response[i]));
	    return m_response[i];
	}
    }
    
    LogInfo() << "HandBookCheck: No response found." << '\n';
    return INVALID_POINT;
}

//----------------------------------------------------------------------------
