//----------------------------------------------------------------------------
/** @file EndgameCheck.cpp
 */
//----------------------------------------------------------------------------

#include "EndgameCheck.hpp"
#include "PlayerUtils.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

EndgameCheck::EndgameCheck(BenzenePlayer* player)
    : BenzenePlayerFunctionality(player),
      m_enabled(true),
      m_search_singleton(false)
{
}

EndgameCheck::~EndgameCheck()
{
}

HexPoint EndgameCheck::pre_search(HexBoard& brd, const Game& game_state,
                                  HexColor color, bitset_t& consider,
                                  double max_time, double& score)
{
    if (m_enabled)
    {
        if (PlayerUtils::IsDeterminedState(brd, color, score)) 
        {
            return PlayerUtils::PlayDeterminedState(brd, color);
        } 
        else 
        {
            consider = PlayerUtils::MovesToConsider(brd, color);
            HexAssert(consider.any());
        }

        score = 0;

        if (consider.count() == 1 && !m_search_singleton) 
        {
            HexPoint move = static_cast<HexPoint>
                (BitsetUtil::FindSetBit(consider));
            LogInfo() << "Mustplay is singleton!" << '\n';
            return move;
        }
    }

    return m_player->pre_search(brd, game_state, color, consider,
				max_time, score);
}

//----------------------------------------------------------------------------
