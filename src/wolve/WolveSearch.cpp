//----------------------------------------------------------------------------
/** @file WolvePlayer.cpp */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "SgSearchValue.h"

#include "BitsetIterator.hpp"
#include "VCSet.hpp"
#include "HexEval.hpp"
#include "HexState.hpp"
#include "EndgameUtil.hpp"
#include "SequenceHash.hpp"
#include "WolveSearch.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

namespace {

//----------------------------------------------------------------------------

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

//----------------------------------------------------------------------------

} // namespace

//----------------------------------------------------------------------------

WolveSearchControl::WolveSearchControl(double maxTime, bool useEarlyAbort,
                                       const SgVector<SgMove>& PV)
    : m_maxTime(maxTime),
      m_useEarlyAbort(useEarlyAbort),
      m_pv(PV),
      m_lastDepthFinishedAt(0.)
{
}

WolveSearchControl::~WolveSearchControl()
{
}

bool WolveSearchControl::Abort(double elapsedTime, int ignoreNumNodes)
{
    UNUSED(ignoreNumNodes);
    if (elapsedTime >= m_maxTime)
    {
        LogInfo() << "WolveSearchControl: time elapsed. Aborting...\n";
        return true;
    }
    return false;
}

bool WolveSearchControl::StartNextIteration(int depth, double elapsedTime, 
                                            int ignoreNumNodes)
{
    UNUSED(ignoreNumNodes);
    if (depth > 1)
    {
        const double timeLeft = m_maxTime - elapsedTime;
        const double timeSinceLast = elapsedTime - m_lastDepthFinishedAt;
        LogInfo() << WolveSearch::PrintPV(m_pv) << '\n'
                  << std::fixed << std::setprecision(1)
                  << "elapsed=" << elapsedTime << "s, "
                  << "last=" << timeSinceLast << "s, "
                  << "left=" << timeLeft << "s\n";
        // Assume the next depth will take at least as long as the
        // last depth
        if (m_useEarlyAbort && timeSinceLast > timeLeft)
        {
            LogInfo() << "Insufficient time for next depth. Aborting...\n";
            return false;
        }
        m_lastDepthFinishedAt = elapsedTime;
    }
    return true;
}

//----------------------------------------------------------------------------

void WolveSearchUtil::ExtractPVFromHashTable(const HexState& state, 
                                       const SgSearchHashTable& hashTable, 
                                             std::vector<HexPoint>& pv)
{
    HexState myState(state);
    SgSearchHashData data;
    while (hashTable.Lookup(myState.Hash(), &data))
    {
        if (data.BestMove() == SG_NULLMOVE)
            break;
        HexPoint bestMove = static_cast<HexPoint>(data.BestMove());
        pv.push_back(bestMove);
        myState.PlayMove(bestMove);
    }
}

std::string WolveSearchUtil::PrintScores(const HexState& state,
                                         const SgSearchHashTable& hashTable)
{
    std::ostringstream os;
    HexState myState(state);
    for (BitsetIterator p(state.Position().GetEmpty()); p; ++p) 
    {
        myState.PlayMove(*p);
        SgSearchHashData data;
        if (hashTable.Lookup(myState.Hash(), &data))
        {
            os << ' ' << *p;
            int value = -data.Value();
            if (value <= -SgSearchValue::MIN_PROVEN_VALUE)
                os << " L";
            else if (value >= SgSearchValue::MIN_PROVEN_VALUE)
                os << " W";
            else 
                os << ' ' << value;
        }
        myState.UndoMove(*p);
    }
    return os.str();
}

void WolveSearchUtil::DumpGuiFx(const HexState& state, 
                                const SgSearchHashTable& hashTable)
{
    std::ostringstream os;
    os << "gogui-gfx:\n"
       << "ab\n"
       << "VAR";
    std::vector<HexPoint> pv;
    WolveSearchUtil::ExtractPVFromHashTable(state, hashTable, pv);
    HexColor color = state.ToPlay();
    for (std::size_t i = 0; i < pv.size(); ++i) 
    {
        os << ' ' << ((color == BLACK) ? "B" : "W")
           << ' ' << pv[i];
        color = !color;
    }
    os << '\n'
       << "LABEL " <<  WolveSearchUtil::PrintScores(state, hashTable)
       << '\n'
       << "TEXT WolveSearch\n"
       << '\n';
    std::cout << os.str();
    std::cout.flush();
}

//----------------------------------------------------------------------------

WolveSearch::WolveSearch()
    : SgSearch(0),
      m_plyWidth(15),
      m_specificPlyWidths(),
      //m_varTT(16),           // 16bit variation trans-table
      m_backup_ice_info(true),
      m_useGuiFx(false)
{
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
    if (EndgameUtil::IsDeterminedState(*m_brd, m_toPlay))
    {
        *isExact = true;
        sgScore = EndgameUtil::IsWonGame(*m_brd, m_toPlay) 
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
    return EndgameUtil::IsDeterminedState(*m_brd, m_toPlay);
}

void WolveSearch::Generate(SgVector<SgMove>* moves, int depthRemaining)
{
    UNUSED(depthRemaining);
    if (!EndgameUtil::IsDeterminedState(*m_brd, m_toPlay))
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
        consider = EndgameUtil::MovesToConsider(*m_brd, m_toPlay);
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
    if (m_useGuiFx && depth > 1)
    {
        const StoneBoard& position = m_brd->GetPosition();
        WolveSearchUtil::DumpGuiFx(HexState(position, position.WhoseTurn()),
                                   *HashTable());
    }
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
            = EndgameUtil::MovesToConsider(*m_brd, m_toplay) & old_consider;
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

/** Sorts moves in consider set and stores the best in outMoves.
    The number of moves depends on PlyWidth() and SpecificPlyWidths().
    See PlyWidth(), SpecificPlyWidth(). */
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
    std::size_t width = m_plyWidth;
    if (0 < m_specificPlyWidths.size())
    {
        BenzeneAssert(std::size_t(CurrentDepth()) < m_plyWidth.size());
        width = m_specificPlyWidths[CurrentDepth()];
    }
    const std::size_t movesToCopy = std::min(mvsc.size(), width);
    for (std::size_t i = 0; i < movesToCopy; ++i)
        outMoves.PushBack(mvsc[i].second);
}

std::string WolveSearch::PrintPV(const SgVector<SgMove>& vec)
{
    std::ostringstream os;
    for (int i = 0; i < vec.Length(); ++i)
        os << (i > 0 ? " " : "") << static_cast<HexPoint>(vec[i]);
    return os.str();
}


//----------------------------------------------------------------------------
