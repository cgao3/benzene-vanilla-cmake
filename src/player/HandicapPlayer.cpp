//----------------------------------------------------------------------------
/** @file HandicapPlayer.cpp
 */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "HandicapPlayer.hpp"
#include "BoardUtils.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

HandicapPlayer::HandicapPlayer(ICEngine* ice)
    : BenzenePlayer(),
      m_ice(ice),
      m_assume_added_stones(true)
{
    LogFine() << "--- HandicapPlayer" << '\n';
}

HandicapPlayer::~HandicapPlayer()
{
}

//----------------------------------------------------------------------------

HexPoint HandicapPlayer::search(HexBoard& brd, 
                                const Game& game_state,
				HexColor color, 
                                const bitset_t& consider,
				double max_time, 
                                double& score)
{
    UNUSED(consider);
    UNUSED(max_time); 
    UNUSED(score);
// NOTE: 'color' is currently used only in a few HexAsserts below. 
// Remove this if it will be used outside of the HexAsserts!
#ifdef NDEBUG
    UNUSED(color);
#endif

    HexPoint lastMove, response;
    HexAssert(color == !VERTICAL_COLOR);
    
    m_width = (m_assume_added_stones) ? brd.width()-1 : brd.width();
    if (m_width == brd.height())
        return RESIGN;
    
    /** Handicap player wins playing second, so in this case any random
	move will suffice. */
    if (game_state.History().empty()) {
	HexAssert(color == FIRST_TO_PLAY);
	return BoardUtils::RandomEmptyCell(brd);
    }
    
    lastMove = game_state.History().back().point();
    LogInfo() << "Last move: " 
             << HexPointUtil::toString(lastMove) << '\n';
    /** For future implementation: discard the naive responseMap and
	just do it here. Only build the responseMap for the places on
	the very edge of the board. Possibly edge and second row from
	edge...  Depends on whether the theory player will handle all
	the edge cases.
    */
    buildResponseMap(brd);
    response = m_responseMap[lastMove];
    if (!brd.isPlayed(response) && response != INVALID_POINT)
        return response;

    LogInfo() << "Playing random move" << '\n';
    return BoardUtils::RandomEmptyCell(brd);
}

void HandicapPlayer::buildResponseMap(const StoneBoard& brd)
{
    int x, y, offset;
    m_responseMap.clear();
    offset = (m_width > brd.height()) ? 1 : -1;
    //Naive mirroring. Ignores handicap stones
    for (BoardIterator it = brd.Interior(); it; ++it)
    {
        HexPointUtil::pointToCoords(*it, x, y);
        if (y > x)
            y = y + offset;
        else
            x = x - offset;
        if (y >= m_width || x >= brd.height())
            m_responseMap[*it] = INVALID_POINT;
        else
            m_responseMap[*it] = HexPointUtil::coordsToPoint(y, x);
    }
    //Handicap Stones mirroring
    if (m_assume_added_stones)
    {
        x = brd.width() - 1;
        y = 0;
        makeMiai(HexPointUtil::coordsToPoint(x, y),
                 HexPointUtil::coordsToPoint(x, y+1));
        for (y = 6; y < brd.height() - 1; y = y + 6)
        {
            makeMiai(HexPointUtil::coordsToPoint(x, y),
                     HexPointUtil::coordsToPoint(x, y+1));
            threeToOne(brd,
                       HexPointUtil::coordsToPoint(x-1, y-3),
                       HexPointUtil::coordsToPoint(x-1, y-4),
                       HexPointUtil::coordsToPoint(x, y-4),
                       HexPointUtil::coordsToPoint(x, y-3));
            threeToOne(brd,
                       HexPointUtil::coordsToPoint(x-1, y-1),
                       HexPointUtil::coordsToPoint(x-1, y),
                       HexPointUtil::coordsToPoint(x, y-1),
                       HexPointUtil::coordsToPoint(x, y-2));
        }
        y = y - 6;
        if (y == brd.height() - 6 || y == brd.height() - 7)
        {
            y = y + 2;
            makeMiai(HexPointUtil::coordsToPoint(x, y),
                     HexPointUtil::coordsToPoint(x, y+1));
        }
        if (y + 3 < brd.height())
        {
            threeToOne(brd,
                       HexPointUtil::coordsToPoint(x-1, y+3),
                       HexPointUtil::coordsToPoint(x-1, y+2),
                       HexPointUtil::coordsToPoint(x, y+2),
                       HexPointUtil::coordsToPoint(x, y+3));
        }
        if (y + 4 < brd.height())
        {
            if (brd.isPlayed(HexPointUtil::coordsToPoint(x-1, y+3)))
            {
                m_responseMap[HexPointUtil::coordsToPoint(x, y+4)] =
                    HexPointUtil::coordsToPoint(x, y+3);
            }
            else
            {
                m_responseMap[HexPointUtil::coordsToPoint(x, y+4)] =
                    HexPointUtil::coordsToPoint(x-1, y+3);
            }
        }
    }
}

void HandicapPlayer::makeMiai(HexPoint p1, HexPoint p2)
{
    m_responseMap[p1] = p2;
    m_responseMap[p2] = p1;
}

void HandicapPlayer::threeToOne(const StoneBoard& brd, 
                                HexPoint dest, HexPoint p1, HexPoint p2,
                                HexPoint p3)
{
    if (brd.isPlayed(dest) && brd.isBlack(dest))
    {
        m_responseMap[p3] = (p3 > p2) 
            ? static_cast<HexPoint>(p3 + MAX_WIDTH) 
            : static_cast<HexPoint>(p3 - MAX_WIDTH);
    }
    else if (brd.isPlayed(dest))
    {
        if (brd.isPlayed(p2) && brd.isPlayed(p3))
        {
            m_responseMap[p2] = p1;
            m_responseMap[p3] = p1;
            return;
        }
        else if (brd.isPlayed(p1) && brd.isPlayed(p3))
        {
            m_responseMap[p1] = p2;
            m_responseMap[p3] = p2;
            return;
        }
        else if (brd.isPlayed(p1) && brd.isPlayed(p2))
        {
            m_responseMap[p1] = p3;
            m_responseMap[p2] = p3;
            return;
        }
        else
        {
            makeMiai(p1, p2);
            m_responseMap[p3] = (p3 > p2) 
                ? static_cast<HexPoint>(p3 + MAX_WIDTH) 
                : static_cast<HexPoint>(p3 - MAX_WIDTH);
        }
    }
    else if (brd.isWhite(p1) || brd.isWhite(p2) || brd.isWhite(p3))
    {
        m_responseMap[p3] = (p3 > p2) 
            ? static_cast<HexPoint>(p3 + MAX_WIDTH) 
            : static_cast<HexPoint>(p3 - MAX_WIDTH);
    }
    else 
    {
        m_responseMap[p1] = dest;
        m_responseMap[p2] = dest;
        m_responseMap[p3] = dest;
    }
}

//----------------------------------------------------------------------------
