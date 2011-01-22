//----------------------------------------------------------------------------
/** @file WolvePlayer.cpp */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "SgSearchValue.h"

#include "BitsetIterator.hpp"
#include "VCSet.hpp"
#include "HexEval.hpp"
#include "EndgameUtils.hpp"
#include "SequenceHash.hpp"
#include "WolveSearch.hpp"

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
    int sgScore;
    if (EndgameUtils::IsDeterminedState(*m_brd, m_toPlay))
    {
        *isExact = true;
        sgScore = EndgameUtils::IsWonGame(*m_brd, m_toPlay) 
            ? +SgSearchValue::MAX_VALUE
            : -SgSearchValue::MAX_VALUE;
    }
    else 
    {
        Resistance resist;
        ComputeResistance(resist);
        HexEval score = (m_toPlay == BLACK) ? resist.Score() : -resist.Score();
        sgScore = ConvertToSgScore(score);
    }
    return sgScore;
}

bool WolveSearch::EndOfGame() const
{
    return EndgameUtils::IsDeterminedState(*m_brd, m_toPlay);
}

void WolveSearch::Generate(SgVector<SgMove>* moves, int depthRemaining)
{
    UNUSED(depthRemaining);
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
