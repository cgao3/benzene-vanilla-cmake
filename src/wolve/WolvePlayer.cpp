//----------------------------------------------------------------------------
/** @file WolvePlayer.cpp */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "SgSearchValue.h"

#include "BitsetIterator.hpp"
#include "VCSet.hpp"
#include "HexEval.hpp"
#include "Misc.hpp"
#include "EndgameUtils.hpp"
#include "SequenceHash.hpp"
#include "WolvePlayer.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

namespace {

static const int sgTopScore = SgSearchValue::MIN_PROVEN_VALUE - 1;
static const int sgBottomScore = -sgTopScore;
static const HexEval topScore = +10.0;
static const HexEval bottomScore = -10.0;
static const HexEval factor = (sgTopScore - sgBottomScore)
    / (topScore - bottomScore);

/** Converts a floating point score to a score used in SgSearch. */
int ConvertToSgScore(HexEval score)
{
    score = std::max(score, bottomScore);
    score = std::min(score, topScore);
    return static_cast<int>(score * factor);
}

std::string PrintSgScore(int score)
{
    if (score >= +SgSearchValue::MIN_PROVEN_VALUE)
        return "win";
    if (score <= -SgSearchValue::MIN_PROVEN_VALUE)
        return "loss";
    std::ostringstream os;
    os << score;
    return os.str();
}

std::string PrintVector(const SgVector<SgMove>& vec)
{
    std::ostringstream os;
    for (int i = 0; i < vec.Length(); ++i)
        os << (i > 0 ? " " : "") << static_cast<HexPoint>(vec[i]);
    return os.str();
}

}
//----------------------------------------------------------------------------

WolvePlayer::WolvePlayer()
    : BenzenePlayer(),
      m_hashTable(1 << 16),
      m_search_depths(),
      m_panic_time(240)
{
    m_search_depths.push_back(1);
    m_search_depths.push_back(2);
    m_search_depths.push_back(4);
}

WolvePlayer::~WolvePlayer()
{
}

//----------------------------------------------------------------------------

/** Reduce search strength if running out of time. */
void WolvePlayer::CheckPanicMode(double timeRemaining,
                                 std::vector<std::size_t>& search_depths,
                                 std::vector<std::size_t>& plywidth)
{
    // If low on time, set a maximum search depth of 4.
    if (m_panic_time >= 0.01 && timeRemaining < m_panic_time)
    {
	std::vector<std::size_t> new_search_depths;
	for (std::size_t i = 0; i < search_depths.size(); ++i)
	    if (search_depths[i] <= 4)
		new_search_depths.push_back(search_depths[i]);
	search_depths = new_search_depths;
        // We also ensure plywidth is at least 15
        for (std::size_t i = 0; i < plywidth.size(); ++i)
            plywidth[i] = std::max(plywidth[i], 15lu);
	LogWarning() << "############# PANIC MODE #############\n";
    }
}

/** Generates a move using WolveSearch.    
    @todo Handle timelimit. */
HexPoint WolvePlayer::Search(const HexState& state, const Game& game,
                             HexBoard& brd, const bitset_t& consider,
                             double maxTime, double& outScore)
{
    UNUSED(game);
    UNUSED(maxTime);
    std::vector<std::size_t> search_depths = m_search_depths;

    //CheckPanicMode(maxTime, search_depths, plywidth);
    m_search.SetRootMovesToConsider(consider);
    m_search.SetWorkBoard(&brd);
    m_search.SetHashTable(&m_hashTable);
    m_search.SetToPlay(HexSgUtil::HexColorToSgColor(state.ToPlay()));
    SgVector<SgMove> PV;
    int score = m_search.IteratedSearch(1, 4, 
                                        -SgSearchValue::MIN_PROVEN_VALUE,
                                        +SgSearchValue::MIN_PROVEN_VALUE,
                                        &PV);
    BenzeneAssert(PV.Length() > 0);
    HexPoint bestMove = static_cast<HexPoint>(PV[0]);
    LogInfo() << PrintStatistics(score, PV) << '\n';
    outScore = score;
    return bestMove;
}

std::string WolvePlayer::PrintStatistics(int score, const SgVector<SgMove>& pv)
{
    SgSearchStatistics stats;
    m_search.GetStatistics(&stats);
    std::ostringstream os;
    os << '\n'
       << SgWriteLabel("NumNodes") << stats.NumNodes() << '\n'
       << SgWriteLabel("NumEvals") << stats.NumEvals() << '\n'
       << SgWriteLabel("DepthReached") << stats.DepthReached() << '\n'
       << SgWriteLabel("Elapsed") << stats.TimeUsed() << '\n'
       << SgWriteLabel("Nodes/s") << stats.NumNodesPerSecond() << '\n'
       << SgWriteLabel("Score") << PrintSgScore(score) << '\n'
       << SgWriteLabel("PV") << PrintVector(pv) << '\n'
       << '\n' 
       << m_hashTable << '\n';
    return os.str();
}

//----------------------------------------------------------------------------

WolveSearch::WolveSearch()
    : SgSearch(0),
      //m_varTT(16),           // 16bit variation trans-table
      m_backup_ice_info(true)
{
    m_plyWidth.push_back(20);
    m_plyWidth.push_back(20);
    m_plyWidth.push_back(20);
    m_plyWidth.push_back(20);
}

WolveSearch::~WolveSearch()
{
}

bool WolveSearch::CheckDepthLimitReached() const
{
    return true;
}

std::string WolveSearch::MoveString(SgMove move) const
{
    if (move == SG_NULLMOVE)
        return "null";
    return HexPointUtil::ToString(static_cast<HexPoint>(move));
}

void WolveSearch::OnStartSearch()
{
    //m_varTT.Clear();  // *MUST* clear old variation TT!
    m_sequence.clear();
    m_consider.clear();
}

int WolveSearch::Evaluate(bool* isExact, int depth)
{
    UNUSED(depth);
    //LogInfo() << "Evaluate:" << *m_brd << '\n';
    int sgScore;
    if (EndgameUtils::IsDeterminedState(*m_brd, m_toPlay))
    {
        *isExact = true;
        sgScore = EndgameUtils::IsWonGame(*m_brd, m_toPlay) 
            ? +SgSearchValue::MAX_VALUE
            : -SgSearchValue::MAX_VALUE;
        //LogInfo() << "Determined! " << sgScore << '\n';
    }
    else 
    {
        Resistance resist;
        ComputeResistance(resist);
        HexEval score = (m_toPlay == BLACK) ? resist.Score() : -resist.Score();
        //LogInfo() << "Resistance: " << score << '\n';
        sgScore = ConvertToSgScore(score);
    }
    //LogInfo() << "score = " << PrintScScore(sgScore) << '\n';
    return sgScore;
}

bool WolveSearch::EndOfGame() const
{
    return EndgameUtils::IsDeterminedState(*m_brd, m_toPlay);
}

void WolveSearch::Generate(SgVector<SgMove>* moves, int depthRemaining)
{
    UNUSED(depthRemaining);
    //LogInfo() << "Generate:" << *m_brd << '\n';
    if (!EndgameUtils::IsDeterminedState(*m_brd, m_toPlay))
    {
        bitset_t consider;
        VariationInfo varInfo;
#if 0
        if (m_varTT.Get(SequenceHash::Hash(m_sequence), varInfo))
        {
            LogFine() << "Using consider set from TT.\n"
                      << HexPointUtil::ToString(m_sequence) << '\n'
                      << m_brd << '\n';
            consider = varInfo.consider;
        }
#endif
        consider = EndgameUtils::MovesToConsider(*m_brd, m_toPlay);
        m_consider.push_back(consider);
        Resistance resist;
        ComputeResistance(resist);
        OrderMoves(consider, resist, *moves);
    }
}

bool WolveSearch::Execute(SgMove move, int* delta, int depth)
{
    UNUSED(delta);
    UNUSED(depth);
    m_brd->PlayMove(m_toPlay, static_cast<HexPoint>(move));
    m_toPlay = !m_toPlay;
    return true;
}

void WolveSearch::StartOfDepth(int depth)
{
    LogInfo() << "===== Depth " << depth << " =====\n";
}

void WolveSearch::TakeBack()
{
    // TODO: Add AfterStateSearched() functionality here
    m_brd->UndoMove();
    m_toPlay = !m_toPlay;
}

#if 0
void WolveSearch::AfterStateSearched()
{
    if (m_backup_ice_info) 
    {
        // Store new consider set in varTT
        bitset_t old_consider = m_consider[m_current_depth];
        bitset_t new_consider 
            = EndgameUtils::MovesToConsider(*m_brd, m_toplay) & old_consider;
        SgHashCode hash = SequenceHash::Hash(m_sequence);
        m_varTT.Put(hash, VariationInfo(m_current_depth, new_consider));
    }
    m_consider.pop_back();
}
#endif

/** Computes resistance on game board using vcs from filled-in
    board. */
void WolveSearch::ComputeResistance(Resistance& resist)
{
    StoneBoard plain(m_brd->Width(), m_brd->Height());
    plain.SetPosition(m_brd->GetPosition());
    Groups groups;
    GroupBuilder::Build(plain, groups);
    AdjacencyGraph graphs[BLACK_AND_WHITE];
    ResistanceUtil::AddAdjacencies(*m_brd, graphs);
    resist.Evaluate(groups, graphs);
}

/** Sorts moves in consider set and stores the best in outMoves. The
    number of moves copied is equal to plyWidth[CurrentDepth()]. */
void WolveSearch::OrderMoves(const bitset_t& consider,
                             const Resistance& resist,
                             SgVector<SgMove>& outMoves) const
{
    std::vector<std::pair<HexEval, HexPoint> > mvsc;
    for (BitsetIterator it(consider); it; ++it) 
    {
        HexEval score = resist.Score(*it);
        mvsc.push_back(std::make_pair(-score, *it));
    }
    // NOTE: To ensure we are deterministic, we must use stable_sort.
    stable_sort(mvsc.begin(), mvsc.end());
    const std::size_t movesToCopy 
        = std::min(mvsc.size(), m_plyWidth[CurrentDepth()]);
    for (std::size_t i = 0; i < movesToCopy;  ++i)
        outMoves.PushBack(mvsc[i].second);
}

//----------------------------------------------------------------------------
