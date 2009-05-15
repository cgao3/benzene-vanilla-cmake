//----------------------------------------------------------------------------
/** @file LadderCheck.cpp
 */
//----------------------------------------------------------------------------

#include "LadderCheck.hpp"
#include "VCPattern.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

LadderCheck::LadderCheck(BenzenePlayer* player)
    : BenzenePlayerFunctionality(player),
      m_enabled(false)
{
}

LadderCheck::~LadderCheck()
{
}

HexPoint LadderCheck::pre_search(HexBoard& brd, const Game& game_state,
				 HexColor color, bitset_t& consider,
				 double max_time, double& score)
{
    if (m_enabled)
    {
        LogInfo() << "Checking for bad ladders..." << '\n';

        bitset_t badProbes;
        HexColor other = !color;
        const VCPatternSet& pset 
            = VCPattern::GetPatterns(brd.width(), brd.height(), other);

        for (std::size_t i=0; i<pset.size(); ++i) {
            const VCPattern& pat = pset[i];

            // Ladder matches if pattern hits and both endpoints are occupied.
            if (pat.Matches(other, brd) 
                && brd.getColor(pat.Endpoint(0)) == other
                && brd.getColor(pat.Endpoint(1)) == other)
            {
                // consider only the probes that are unoccupied
                bitset_t bp = pat.BadProbes() & brd.getEmpty();

                // take out the bad probes only if there are moves
                // remaining in the consider set afterward.
                if ((consider - (badProbes | bp)).any())
                    badProbes |= bp;
            }
        }
        
        if (badProbes.any()) {
            consider = consider - badProbes;
            LogInfo() << "Removed bad probes:"
                      << brd.printBitset(badProbes) << '\n';
        }
    }

    return m_player->pre_search(brd, game_state, color, consider,
				max_time, score);
}

//----------------------------------------------------------------------------
