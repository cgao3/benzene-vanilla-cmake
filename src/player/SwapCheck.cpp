//----------------------------------------------------------------------------
/** @file SwapCheck.cpp
 */
//----------------------------------------------------------------------------

#include "BoardUtils.hpp"
#include "SwapCheck.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

SwapCheck::SwapCheck(BenzenePlayer* player)
    : BenzenePlayerFunctionality(player),
      m_player(player)
{
}

SwapCheck::~SwapCheck()
{
}

HexPoint SwapCheck::PreSearch(HexBoard& brd, const Game& game_state,
                              HexColor color, bitset_t& consider,
                              double max_time, double& score)
{
    if (game_state.AllowSwap()
        && (1 == game_state.History().size())
        && (!FIRST_TO_PLAY == color))
    {
	HexAssert(1 == brd.GetState().NumStones());
	LogInfo() << "Performing swap pre-check...\n";
	
	// If board has unequal dimensions, we want to traverse the
	// shorter distance.
	if (brd.Width() != brd.Height()) 
        {
	    if ((brd.Width() > brd.Height() && color == !VERTICAL_COLOR) ||
		(brd.Width() < brd.Height() && color == VERTICAL_COLOR))
            {
                LogInfo() << "Non-square board: " 
                          << "Swapping to obtain shorter side!\n";
		return SWAP_PIECES;
            }
	}
        else 
        {
            if (!m_swapLoaded)
                LoadSwapMoves("swap-moves.txt");
	    HexPoint firstMove = game_state.History().back().point();
	    if (color == VERTICAL_COLOR)
                // Swap decisions assume VERTICAL_COLOR was FIRST_TO_PLAY,
                // so we mirror the first move if this is not the case
                // (i.e. to consider an equivalent decision).
                firstMove = BoardUtils::Mirror(brd.Const(), firstMove);
            std::ostringstream os;
            os << brd.Width() << 'x' << brd.Height();
            if (m_swapMoves[os.str()].count(firstMove) > 0)
                return SWAP_PIECES;
        }
        LogInfo() << "Opted not to swap.\n";
    }
    return m_player->PreSearch(brd, game_state, color, consider,
                               max_time, score);
}

/** Loads swap moves for each boardsize from the given file.
    Ignores lines begining with '#'. On lines not begining with '#',
    expects a string of the form "nxn" and the name of a HexPoint:
    this pair denotes a move to swap on an nxn board. The remainder of
    the line is not looked at.
*/
void SwapCheck::LoadSwapMoves(const std::string& name)
{
    using namespace boost::filesystem;
    path swap_path = path(ABS_TOP_SRCDIR) / "share";
    path swap_list = swap_path / name;
    swap_list.normalize();
    std::string swap_file = swap_list.native_file_string();
    LogInfo() << "Loading swap moves: '" << swap_file << "'...\n";
    m_swapMoves.clear();
    std::ifstream s(swap_file.c_str());
    if (!s)
        throw HexException("SwapCheck: could not open list!\n");
    std::string line;
    std::size_t lineNumber = 0;
    while (std::getline(s, line))
    {
        lineNumber++;
        if (line[0] == '#')
            continue;
        if (line.size() < 6) // skip (nearly) empty lines
            continue;
        std::string boardSizeStr;
        std::string pointStr;
        std::istringstream ss(line);
        ss >> boardSizeStr;
        ss >> pointStr;
        HexPoint point = HexPointUtil::FromString(pointStr);
        if (point == INVALID_POINT)
            LogWarning() << "SwapCheck: line " << lineNumber 
                         << ": invalid cell!\n";
        else
            m_swapMoves[boardSizeStr].insert(point);
    }
    s.close();
    m_swapLoaded = true;
}

//----------------------------------------------------------------------------
