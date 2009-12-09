//----------------------------------------------------------------------------
/** @file WolvePlayer.cpp
 */
//----------------------------------------------------------------------------

#include "BitsetIterator.hpp"
#include "VCSet.hpp"
#include "HexEval.hpp"
#include "Misc.hpp"
#include "PlayerUtils.hpp"
#include "SequenceHash.hpp"
#include "WolvePlayer.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

WolvePlayer::WolvePlayer()
    : BenzenePlayer(),
      m_tt(16), 
      m_plywidth(),
      m_search_depths(),
      m_panic_time(240)
{
    LogFine() << "--- WolvePlayer" << '\n';

    m_plywidth.push_back(20);
    m_plywidth.push_back(20);
    m_plywidth.push_back(20);
    m_plywidth.push_back(20);

    m_search_depths.push_back(1);
    m_search_depths.push_back(2);
    m_search_depths.push_back(4);

    m_search.SetTT(&m_tt);
}

WolvePlayer::~WolvePlayer()
{
}

//----------------------------------------------------------------------------

HexPoint WolvePlayer::Search(HexBoard& brd, 
                             const Game& game_state,
			     HexColor color, 
                             const bitset_t& consider,
                             double max_time,
                             double& score)
{
    UNUSED(game_state);
    std::vector<int> search_depths = m_search_depths;

    // If low on time, set a maximum search depth of 2.
    if (max_time < m_panic_time)
    {
	std::vector<int> new_search_depths;
	for (std::size_t i = 0; i < search_depths.size(); ++i)
	    if (search_depths[i] <= 2)
		new_search_depths.push_back(search_depths[i]);
	search_depths = new_search_depths;
	LogWarning() << "############# PANIC MODE #############" << '\n';
    }

    m_search.SetRootMovesToConsider(consider);
    LogInfo() << "Using consider set:" << brd.Write(consider) << '\n'
	      << "Plywidths: " << MiscUtil::PrintVector(m_plywidth) << '\n'
	      << "Depths: " << MiscUtil::PrintVector(m_search_depths) << '\n';

    std::vector<HexPoint> PV;
    score = m_search.Search(brd, color, m_plywidth, search_depths, -1, PV);

    LogInfo() << m_search.DumpStats() << '\n';

    HexAssert(PV.size() > 0);
    return PV[0];
}

//----------------------------------------------------------------------------

WolveSearch::WolveSearch()
    : m_varTT(16),           // 16bit variation trans-table
      m_backup_ice_info(true)
{
}

WolveSearch::~WolveSearch()
{
}

void WolveSearch::OnStartSearch()
{
    m_varTT.Clear();  // *MUST* clear old variation TT!
    m_consider.clear();
}

void WolveSearch::EnteredNewState()
{
}

HexEval WolveSearch::Evaluate()
{
    Resistance resist;
    ComputeResistance(resist);
    HexEval score = (m_toplay == BLACK) ? resist.Score() : -resist.Score();
    LogFine() << "Score for " << m_toplay << ": " << score << '\n';
    return score;
}

void WolveSearch::GenerateMoves(std::vector<HexPoint>& moves)
{
    Resistance resist;
    ComputeResistance(resist);

    // Get moves to consider:
    //   1) from the variation tt, if this variation has been visited before.
    //   2) from the passed in consider set, if at the root.
    //   3) from computing it ourselves.
    bitset_t consider;

    VariationInfo varInfo;
    if (m_varTT.Get(SequenceHash::Hash(m_sequence), varInfo))
    {
        LogFine() << "Using consider set from TT." << '\n'
		  << HexPointUtil::ToString(m_sequence) << '\n'
		  << m_brd << '\n';
        consider = varInfo.consider;
    } 
    else if (m_current_depth == 0)
    {
        LogFine() << "Using root consider set." << '\n';
        consider = m_rootMTC;
    }
    else 
    {
        LogFine() << "Computing our own consider set." << '\n';
        consider = PlayerUtils::MovesToConsider(*m_brd, m_toplay);
    }

    m_consider.push_back(consider);
    HexAssert((int)m_consider.size() == m_current_depth+1);

    // order the moves
    moves.clear();
    std::vector<std::pair<HexEval, HexPoint> > mvsc;
    for (BitsetIterator it(consider); it; ++it) 
    {
        // Prefer the best move from the TT if possible
        HexEval score = (*it == m_tt_bestmove) 
            ? 10000
            : resist.Score(*it);
        mvsc.push_back(std::make_pair(-score, *it));
    }
    /** @note to ensure we are deterministic, we must use stable_sort. */
    stable_sort(mvsc.begin(), mvsc.end());
    for (std::size_t i = 0; i < mvsc.size(); ++i)
        moves.push_back(mvsc[i].second);
}

void WolveSearch::ExecuteMove(HexPoint move)
{
    m_brd->PlayMove(m_toplay, move);
}

void WolveSearch::UndoMove(HexPoint move)
{
    UNUSED(move);
    m_brd->UndoMove();
}

void WolveSearch::AfterStateSearched()
{
    if (m_backup_ice_info) 
    {
        // store new consider set in varTT
        bitset_t old_consider = m_consider[m_current_depth];
        bitset_t new_consider 
            = PlayerUtils::MovesToConsider(*m_brd, m_toplay) & old_consider;
        hash_t hash = SequenceHash::Hash(m_sequence);
        m_varTT.Put(hash, VariationInfo(m_current_depth, new_consider));
    }
    m_consider.pop_back();
}

void WolveSearch::OnSearchComplete()
{
}

//----------------------------------------------------------------------------

void WolveSearch::ComputeResistance(Resistance& resist)
{
    StoneBoard plain(m_brd->Width(), m_brd->Height());
    StoneBoard& state = m_brd->GetState();
    plain.AddColor(BLACK, state.GetBlack() & state.GetPlayed());
    plain.AddColor(WHITE, state.GetWhite() & state.GetPlayed());    
    plain.SetPlayed(state.GetPlayed());
    Groups groups;
    GroupBuilder::Build(plain, groups);
    AdjacencyGraph graphs[BLACK_AND_WHITE];
    ResistanceUtil::AddAdjacencies(*m_brd, graphs);
    resist.Evaluate(groups, graphs);
}

//----------------------------------------------------------------------------
