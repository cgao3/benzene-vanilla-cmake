//----------------------------------------------------------------------------
/** @file DfpnSolver.cpp */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "SgWrite.h"
#include "BitsetIterator.hpp"
#include "BoardUtil.hpp"
#include "DfpnSolver.hpp"
#include "PatternState.hpp"
#include "EndgameUtil.hpp"
#include "ProofUtil.hpp"
#include "Resistance.hpp"

#include <boost/filesystem/path.hpp>

using namespace benzene;

using std::ceil;

//----------------------------------------------------------------------------

/** Current version of the dfpn database.
    Update this if DfpnData changes to prevent old out-of-date
    databases from being loaded. */
const std::string DfpnDB::DFPN_DB_VERSION("BENZENE_DFPN_DB_VER_0002");

//----------------------------------------------------------------------------

#ifndef NDEBUG
void DfpnBounds::CheckConsistency() const
{
    // Check range
    BenzeneAssert(phi <= INFTY);
    BenzeneAssert(delta <= INFTY);
    // one is 0 iff the other is infinity
    BenzeneAssert(0 != phi || INFTY == delta);
    BenzeneAssert(INFTY != phi || 0 == delta);
    BenzeneAssert(0 != delta || INFTY == phi);
    BenzeneAssert(INFTY != delta || 0 == phi);
}
#else
void DfpnBounds::CheckConsistency() const
{
}
#endif

//----------------------------------------------------------------------------

void VirtualBoundsTT::Update(size_t depth, const HexState& state,
                             const DfpnBounds& bounds)
{
    if (depth >= data.size())
        return;
    std::vector<Entry>& level = data[depth];
    for (size_t i = 0; i < level.size(); ++i)
        if (level[i].hash == state.Hash())
        {
            level[i].bounds = bounds;
            return;
        }
}

void VirtualBoundsTT::Store(size_t depth, const HexState& state,
                             const DfpnBounds& bounds)
{
    if (depth >= data.size())
        data.resize(depth + 1);
    else
    {
        std::vector<Entry>& level = data[depth];
        for (size_t i = 0; i < level.size(); ++i)
            if (level[i].hash == state.Hash())
            {
                level[i].count++;
                level[i].bounds = bounds;
                return;
            }
    }
    Entry entry;
    entry.hash = state.Hash();
    entry.count = 1;
    entry.bounds = bounds;
    data[depth].push_back(entry);
}

void VirtualBoundsTT::Lookup(size_t depth, const HexState& state,
                             DfpnBounds& bounds)
{
    if (depth >= data.size())
        return;
    const std::vector<Entry>& level = data[depth];
    for (size_t i = 0; i < level.size(); ++i)
        if (level[i].hash == state.Hash())
        {
            bounds = level[i].bounds;
            return;
        }
}

void VirtualBoundsTT::Remove(size_t depth, const HexState& state)
{
    std::vector<Entry>& level = data[depth];
    for (size_t i = 0; i < level.size(); ++i)
        if (level[i].hash == state.Hash())
        {
            if (--level[i].count == 0)
            {
                level[i] = level.back();
                level.pop_back();
            }
            return;
        }
}

//----------------------------------------------------------------------------

DfpnChildren::DfpnChildren()
{
}

void DfpnChildren::SetChildren(const std::vector<HexPoint>& children)
{
    m_children = children;
}

void DfpnChildren::PlayMove(std::size_t index, HexState& state) const
{
    state.PlayMove(m_children[index]);
}

void DfpnChildren::UndoMove(std::size_t index, HexState& state) const
{
    state.UndoMove(m_children[index]);
}

//----------------------------------------------------------------------------

/** @page dfpnguifx Dfpn Progress Indication
    @ingroup dfpn

    It is difficult to present the user with meaningful progress
    indication in dfpn. The current method simply displays the current
    (phi, delta) bounds of each child of the root. This is output
    whenever a child's bound changes. This is reasonably useful,
    except in the case where only a single child remains and the
    search is stuck several ply deep.
*/

DfpnSolver::GuiFx::GuiFx()
    : m_index(9999),
      m_timeOfLastWrite(0.0),
      m_delay(1.0)
{
}

void DfpnSolver::GuiFx::SetChildren(const DfpnChildren& children,
                                    const std::vector<DfpnData>& data)
{
    m_children = children;
    m_data = data;
}

void DfpnSolver::GuiFx::PlayMove(HexColor color, std::size_t index)
{
    m_color = color;
    m_index = index;
}

void DfpnSolver::GuiFx::UndoMove()
{
    m_index = 9999;
}

void DfpnSolver::GuiFx::UpdateCurrentBounds(const DfpnBounds& bounds)
{
    BenzeneAssert(m_index != 9999);
    m_data[m_index].m_bounds = bounds;
}

/** Always writes output. */
void DfpnSolver::GuiFx::WriteForced()
{
    DoWrite();
}

/** Writes output only if last write was more than m_delay seconds
    ago or if the move is different. */
void DfpnSolver::GuiFx::Write()
{
    double currentTime = SgTime::Get();
    if (m_indexAtLastWrite == m_index)
    {
        if (currentTime < m_timeOfLastWrite + m_delay)
            return;
    }
    m_timeOfLastWrite = currentTime;
    m_indexAtLastWrite = m_index;
    DoWrite();
}

/** Writes progress indication. */
void DfpnSolver::GuiFx::DoWrite()
{
    std::ostringstream os;
    os << "gogui-gfx:\n";
    os << "dfpn\n";
    os << "VAR";
    if (m_index != 9999)
    {
        os << ' ' << (m_color == BLACK ? 'B' : 'W')
           << ' ' << m_children.FirstMove(m_index);
    }
    os << '\n';
    os << "LABEL";
    int numLosses = 0;
    for (std::size_t i = 0; i < m_children.Size(); ++i)
    {
        os << ' ' << m_children.FirstMove(i);
        if (0 == m_data[i].m_bounds.phi)
        {
            numLosses++;
            os << " L";
        }
        else if (0 == m_data[i].m_bounds.delta)
            os << " W";
        else
            os << ' ' << m_data[i].m_bounds.phi 
               << ':' << m_data[i].m_bounds.delta;
    }
    os << '\n';
    os << "TEXT ";
    os << numLosses << '/' << m_children.Size() << " proven losses\n";
    os << '\n';
    std::cout << os.str();
    std::cout.flush();
}

//----------------------------------------------------------------------------

int DfpnData::PackedSize() const
{
    return sizeof(m_bounds)
        + sizeof(m_bestMove)
        + sizeof(m_work)
        + sizeof(m_maxProofSet)
        + sizeof(m_evaluationScore)
        + sizeof(short) * (m_children.Size() + 1);
}

byte* DfpnData::Pack() const
{
    static byte data[4096];
    byte* off = data;
    *reinterpret_cast<DfpnBounds*>(off) = m_bounds;
    off += sizeof(m_bounds);
    *reinterpret_cast<HexPoint*>(off) = m_bestMove;
    off += sizeof(m_bestMove);
    *reinterpret_cast<size_t*>(off) = m_work;
    off += sizeof(m_work);
    *reinterpret_cast<bitset_t*>(off) = m_maxProofSet;
    off += sizeof(m_maxProofSet);
    *reinterpret_cast<float*>(off) = m_evaluationScore;
    off += sizeof(m_evaluationScore);
    const std::vector<HexPoint>& moves = m_children.m_children;
    for (std::size_t i = 0; i < moves.size(); ++i)
    {
        *reinterpret_cast<short*>(off) = static_cast<short>(moves[i]);
        off += sizeof(short);
    }
    *reinterpret_cast<short*>(off) = static_cast<short>(INVALID_POINT);
    off += sizeof(short);
    if (off - data != PackedSize())
        throw BenzeneException("Bad size!");
    return data;
}

void DfpnData::Unpack(const byte* data)
{
    m_bounds = *reinterpret_cast<const DfpnBounds*>(data);
    data += sizeof(m_bounds);
    m_bestMove = *reinterpret_cast<const HexPoint*>(data);
    data += sizeof(m_bestMove);
    m_work = *reinterpret_cast<const size_t*>(data);
    data += sizeof(m_work);
    m_maxProofSet = *reinterpret_cast<const bitset_t*>(data);
    data += sizeof(m_maxProofSet);
    m_evaluationScore = *reinterpret_cast<const float*>(data);
    data += sizeof(m_evaluationScore);
    std::vector<HexPoint> moves;
    while (true)
    {
        short s = *reinterpret_cast<const short*>(data);
        data += sizeof(short);
        HexPoint p = static_cast<HexPoint>(s);
        if (p == INVALID_POINT)
            break;
        moves.push_back(p);
    }
    m_children.SetChildren(moves);
}

void DfpnData::Rotate(const ConstBoard& brd)
{
    if (m_bestMove != INVALID_POINT)
        m_bestMove = BoardUtil::Rotate(brd, m_bestMove);
    m_maxProofSet = BoardUtil::Rotate(brd, m_maxProofSet);
    std::vector<HexPoint>& moves = m_children.m_children;
    for (std::size_t i = 0; i < moves.size(); ++i)
        moves[i] = BoardUtil::Rotate(brd, moves[i]);
}

//----------------------------------------------------------------------------

DfpnSolver::DfpnSolver()
    : m_positions(0),
      m_useGuiFx(false),
      m_timelimit(0.0),
      m_wideningBase(1),
      m_wideningFactor(0.25f),
      m_epsilon(0.0f),
      m_threads(1),
      m_threadWork(1000),
      m_guiFx(),
      m_allEvaluation(-2.5, 2.0, 45),
      m_allSolvedEvaluation(-2.5, 2.0, 45),
      m_winningEvaluation(-2.5, 2.0, 45),
      m_losingEvaluation(-2.5, 2.0, 45)
{
}

DfpnSolver::~DfpnSolver()
{
}

void DfpnSolver::PrintStatistics(HexColor winner,
                                 const PointSequence& pv) const
{
    std::ostringstream os;
    os << '\n'
       << SgWriteLabel("MID calls") << m_numMIDcalls << '\n'
       << SgWriteLabel("VC builds") << m_numVCbuilds << '\n'
       << SgWriteLabel("Terminal") << m_numTerminal << '\n'
       << SgWriteLabel("Work") << m_numMIDcalls + m_numTerminal << '\n'
       << SgWriteLabel("Wasted Work") << m_totalWastedWork
       << " (" << (double(m_totalWastedWork) * 100.0 
                   / double(m_numMIDcalls + m_numTerminal)) << "%)\n"
       << SgWriteLabel("Elapsed Time") << m_timer.GetTime() << '\n'
       << SgWriteLabel("MIDs/sec") 
       << double(m_numMIDcalls) / m_timer.GetTime() << '\n'
       << SgWriteLabel("VCs/sec")
       << double(m_numVCbuilds) / m_timer.GetTime() << '\n'
       << SgWriteLabel("Cnt prune sib") << m_prunedSiblingStats.Count() << '\n'
       << SgWriteLabel("Avg prune sib");
    m_prunedSiblingStats.Write(os);
    os << '\n' << SgWriteLabel("Consider Size");
    m_considerSetSize.Write(os);
    os << '\n' << SgWriteLabel("Move Index");
    m_moveOrderingIndex.Write(os);
    os << '\n' << SgWriteLabel("Move Percent");
    m_moveOrderingPercent.Write(os);
    os << '\n' << SgWriteLabel("Delta Increase");
    m_deltaIncrease.Write(os);
    os << '\n'
       << SgWriteLabel("Winner") << winner << '\n'
       << SgWriteLabel("PV") << HexPointUtil::ToString(pv) << '\n';
    if (m_positions->Database())
        os << '\n' << m_positions->Database()->GetStatistics().Write() << '\n';
    if (m_positions->HashTable())
        os << '\n' << *m_positions->HashTable() << '\n';
    LogInfo() << os.str();
}

std::string DfpnSolver::EvaluationInfo() const
{
    std::ostringstream os;
    os << "All:\n";
    m_allEvaluation.Write(os);
    os << "All Solved:\n";
    m_allSolvedEvaluation.Write(os);
    os << "Winning:\n";
    m_winningEvaluation.Write(os);
    os << "Losing:\n";
    m_losingEvaluation.Write(os);
    return os.str();
}

HexColor DfpnSolver::StartSearch(const HexState& state, HexBoard& board,
                                 DfpnStates& positions, PointSequence& pv)
{
    return StartSearch(state, board, positions, pv, 
                       DfpnBounds(DfpnBounds::MAX_WORK, DfpnBounds::MAX_WORK));
}

class RunDfpnThread
{
    int index;
    DfpnSolver& solver;
    const DfpnBounds& maxBounds;
    const HexState& state;
    HexBoard& board;
public:
    RunDfpnThread(int index,
                  DfpnSolver& solver, const DfpnBounds& maxBounds,
                  const HexState& state, HexBoard& board)
        : index(index),
          solver(solver), maxBounds(maxBounds), state(state), board(board)
    { }

    void operator()()
    {
        solver.RunThread(index, maxBounds, state, board);
    }
};

HexColor DfpnSolver::StartSearch(const HexState& state, HexBoard& board,
                                 DfpnStates& positions, PointSequence& pv,
                                 const DfpnBounds& maxBounds)
{
    m_aborted = false;
    m_positions = &positions;
    m_numTerminal = 0;
    m_numMIDcalls = 0;
    m_numVCbuilds = 0;
    m_totalWastedWork = 0;
    m_prunedSiblingStats.Clear();
    m_moveOrderingPercent.Clear();
    m_moveOrderingIndex.Clear();
    m_considerSetSize.Clear();
    m_deltaIncrease.Clear();
    m_losingEvaluation.Clear();
    m_winningEvaluation.Clear();
    m_allEvaluation.Clear();
    m_allSolvedEvaluation.Clear();
    m_checkTimerAbortCalls = 0;

    // Skip search if already solved
    DfpnData data;
    if (TTRead(state, data) && data.m_bounds.IsSolved())
    {
        LogInfo() << "Already solved!\n";
        HexColor w = data.m_bounds.IsWinning()
            ? m_state->ToPlay() : !m_state->ToPlay();
        SolverDBUtil::GetVariation(state, *m_positions, pv);
        LogInfo() << w << " wins!\n";
        LogInfo() << "PV: " << HexPointUtil::ToString(pv) << '\n';
        return w;
    }

    m_timer.Start();
    boost::thread threads[m_threads];
    for (int i = 0; i < m_threads; i++)
    {
        RunDfpnThread run(i, *this, maxBounds, state, board);
        threads[i] = boost::thread(run);
    }
    for (int i = 0; i < m_threads; i++)
        threads[i].join();
    m_timer.Stop();

    SolverDBUtil::GetVariation(state, *m_positions, pv);
    HexColor winner = EMPTY;
    if (TTRead(state, data) && data.m_bounds.IsSolved())
        winner = data.m_bounds.IsWinning()
            ? state.ToPlay() : !state.ToPlay();
    PrintStatistics(winner, pv);

    if (m_aborted)
        LogInfo() << "Search aborted.\n";
    return winner;
}

bool DfpnSolver::CheckAbort()
{
    if (!m_aborted)
    {
        if (SgUserAbort()) 
        {
            m_aborted = true;
            LogInfo() << "DfpnSolver::CheckAbort(): Abort flag!\n";
            m_nothingToSearch_cond.notify_all();
        }
        else if (m_timelimit > 0)
        {
            if (m_checkTimerAbortCalls == 0)
            {
                double elapsed = m_timer.GetTime();
                if (elapsed > m_timelimit)
                {
                    m_aborted = true;
                    LogInfo() << "DfpnSolver::CheckAbort(): Timelimit!\n";
                    m_nothingToSearch_cond.notify_all();
                }
                else
                {
                    if (m_numMIDcalls < 100)
                        m_checkTimerAbortCalls = 10;
                    else
                    {
                        size_t midsPerSec = static_cast<size_t>
                            (double(m_numMIDcalls) / elapsed);
                        m_checkTimerAbortCalls = midsPerSec / 2;
                    }
                }
            }
            else
                --m_checkTimerAbortCalls;
        }
    }
    return m_aborted;
}

bool DfpnSolver::TryDescent(std::vector<DescentStack>& stack,
                            DfpnData& data, DfpnBounds& vBounds,
                            DfpnBounds maxBounds, DfpnBounds& midBounds)
{
    if (!maxBounds.GreaterThan(vBounds))
        return false;
    size_t depth = stack.size();
    if (data.m_work < m_threadWork)
    {
        midBounds = maxBounds;
        if (vBounds.phi <= vBounds.delta)
            DfpnBounds::SetToWinning(vBounds);
        else
            DfpnBounds::SetToLosing(vBounds);
        m_vtt.Store(depth, *m_state, vBounds);
        return true;
    }
    BenzeneAssert(data.IsValid());
    stack.push_back(DescentStack());
    DescentStack* sd = &stack.back();

    sd->children = data.m_children;

    sd->childrenData.resize(sd->children.Size());
    LookupChildren(sd->childrenData, sd->children);

    sd->virtualBounds.resize(sd->children.Size());
    LookupChildren(depth, sd->virtualBounds, sd->childrenData, sd->children);

    size_t maxChildIndex = ComputeMaxChildIndex(sd->childrenData);

    bool found = false;
    while (!found)
    {
        UpdateBounds(data.m_bounds, sd->childrenData, maxChildIndex);
        UpdateBounds(vBounds, sd->virtualBounds, maxChildIndex);

        if (!maxBounds.GreaterThan(vBounds))
        {
            stack.pop_back();
            m_vtt.Update(depth, *m_state, vBounds);
            return false;
        }

        DfpnBounds childMaxBounds;
        sd->bestIndex = SelectChild(childMaxBounds, vBounds,
                                    sd->virtualBounds,
                                    maxBounds, maxChildIndex);

        sd->children.PlayMove(sd->bestIndex, *m_state);
        DfpnData childData(sd->childrenData[sd->bestIndex]);
        DfpnBounds childVBounds(sd->virtualBounds[sd->bestIndex]);
        found = TryDescent(stack, childData, childVBounds,
                           childMaxBounds, midBounds);
        sd = &stack[depth];
        sd->virtualBounds[sd->bestIndex] = childVBounds;
        sd->childrenData[sd->bestIndex] = childData;
        sd->children.UndoMove(sd->bestIndex, *m_state);
    }

    m_vtt.Store(depth, *m_state, vBounds);
    return true;
}

void DfpnSolver::CreateHistory(std::vector<DescentStack>& stack)
{
    for (std::size_t i = 0; i < stack.size(); ++i)
    {
        DescentStack& sd = stack[i];
        SgHashCode hash = m_state->Hash();
        sd.children.PlayMove(sd.bestIndex, *m_state);
        m_history->Push(sd.children.FirstMove(sd.bestIndex), hash);
    }
}

void DfpnSolver::UpdateStatesBackward(std::vector<DescentStack>& stack,
                                      const DfpnData& data, size_t work)
{
    if (stack.empty())
    {
        m_vtt.Remove(0, *m_state);
        TTWrite(*m_state, data);
        return;
    }

    m_vtt.Remove(stack.size(), *m_state);
    TTWrite(*m_state, data);
    DescentStack& sd = stack.back();
    sd.children.UndoMove(sd.bestIndex, *m_state);
    m_history->Pop();

    DfpnData parent;
    work += CreateData(parent);
    parent.m_bestMove = sd.children.FirstMove(sd.bestIndex);

    PruneChildren(parent.m_children, sd.childrenData,
                  parent.m_bestMove, data.m_maxProofSet);

    sd.childrenData = std::vector<DfpnData>(parent.m_children.Size());
    LookupChildren(sd.childrenData, parent.m_children);

    size_t maxChildIndex = ComputeMaxChildIndex(sd.childrenData);
    UpdateBounds(parent.m_bounds, sd.childrenData, maxChildIndex);

    UpdateStatsOnWin(sd.childrenData, sd.bestIndex, parent.m_work + work);

    UpdateSolvedBestMove(parent, sd.childrenData);
    parent.m_work += work;

    stack.pop_back();
    UpdateStatesBackward(stack, parent, work);
}

void DfpnSolver::RunThread(int index, const DfpnBounds& maxBounds,
                           const HexState& state, HexBoard& board)
{
    m_state.reset(new HexState(state));
    m_workBoard.reset(new HexBoard(board));
    m_history.reset(new DfpnHistory());

    while (true)
    {
        std::vector<DescentStack> stack;
        DfpnBounds midBounds;

        {
            boost::unique_lock<boost::mutex> lock_descent(m_descent_mutex);

            DfpnData root_data;
            TTRead(*m_state, root_data);
            if (!maxBounds.GreaterThan(root_data.m_bounds))
                return;

            while (!m_aborted)
            {
                DfpnBounds root_vBound(root_data.m_bounds);
                m_vtt.Lookup(0, *m_state, root_vBound);

                // descent for a node to explore
                if (TryDescent(stack, root_data, root_vBound,
                               maxBounds, midBounds))
                    break;

                if (!maxBounds.GreaterThan(root_data.m_bounds))
                    return;

                m_nothingToSearch_cond.wait(lock_descent);
            }
        }

        CreateHistory(stack);
        DfpnData data;
        size_t work = MID(midBounds, m_threadWork, data);

        {
            boost::lock_guard<boost::mutex> lock_descent(m_descent_mutex);
            UpdateStatesBackward(stack, data, work);
        }
        m_nothingToSearch_cond.notify_all();
    }
}

size_t DfpnSolver::CreateData(DfpnData& data)
{
    if (TTRead(*m_state, data))
        return 0;

    HexColor colorToMove = m_state->ToPlay();

    m_workBoard->GetPosition().SetPosition(m_state->Position());
    m_workBoard->ComputeAll(colorToMove);
    ++m_numVCbuilds;

    data.Validate();

    // Compute the maximum possible proof set if colorToMove wins.
    // This data is used to prune siblings of this state.
    data.m_maxProofSet =
        ProofUtil::MaximumProofSet(*m_workBoard, colorToMove);

    if (EndgameUtil::IsDeterminedState(*m_workBoard, colorToMove))
    {
        ++m_numTerminal;
        if (EndgameUtil::IsWonGame(*m_workBoard, colorToMove))
            DfpnBounds::SetToWinning(data.m_bounds);
        else
            DfpnBounds::SetToLosing(data.m_bounds);

        if (m_useGuiFx && m_history->Depth() == 1)
        {
            m_guiFx.UpdateCurrentBounds(data.m_bounds);
            m_guiFx.Write();
        }
        data.m_bestMove = INVALID_POINT;
        data.m_work = 1;
        data.m_evaluationScore = 0.0;
        return 1;
    }

    bitset_t childrenBitset =
        EndgameUtil::MovesToConsider(*m_workBoard, colorToMove);

    m_considerSetSize.Add(float(childrenBitset.count()));
    Resistance resist;
    resist.Evaluate(*m_workBoard);
    data.m_evaluationScore = (colorToMove == BLACK)
        ? resist.Score() : -resist.Score();
    m_allEvaluation.Add(data.m_evaluationScore);
    std::vector<std::pair<HexEval, HexPoint> > mvsc;
    for (BitsetIterator it(childrenBitset); it; ++it)
    {
        HexEval score = resist.Score(*it);
        mvsc.push_back(std::make_pair(-score, *it));
    }
    stable_sort(mvsc.begin(), mvsc.end());
    std::vector<HexPoint> sortedChildren;
    for (size_t i = 0; i < mvsc.size(); ++i)
        sortedChildren.push_back(mvsc[i].second);
    data.m_children.SetChildren(sortedChildren);
    return 1;
}

void DfpnSolver::UpdateStatsOnWin(const std::vector<DfpnData>& childrenData,
                                  size_t bestIndex, size_t work)
{
    if (!childrenData[bestIndex].m_bounds.IsLosing())
        return;
    m_moveOrderingIndex.Add(float(bestIndex));
    m_moveOrderingPercent.Add(float(bestIndex) / (float)childrenData.size());
    m_totalWastedWork += work - childrenData[bestIndex].m_work;
}

void DfpnSolver::PruneChildren(DfpnChildren& children,
                               std::vector<DfpnData>& childrenData,
                               HexPoint bestMove, bitset_t maxProofSet)
{
    // Shrink children list using knowledge of bestMove child's proof set.
    // That is, if this child is losing, conclude what other children
    // must also be losing (i.e. cannot interfere with the proof set
    // that disproves this child).
    // And of course if this child is winning, no need to explore
    // these other siblings either.
    /* @todo Perhaps track allChildren instead of recomputing? */
    bitset_t allChildren;
    for (std::vector<HexPoint>::iterator it = children.m_children.begin();
         it != children.m_children.end(); ++it)
        allChildren.set(*it);
    bitset_t canPrune = allChildren - maxProofSet;
    canPrune.reset(bestMove);
    std::size_t pruneCount = canPrune.count();
    if (pruneCount > 0)
    {
        m_prunedSiblingStats.Add(float(pruneCount));
        DeleteChildren(children, childrenData, canPrune);
        if (m_useGuiFx && m_history->Depth() == 0)
            m_guiFx.SetChildren(children, childrenData);
    }
}

void DfpnSolver::UpdateSolvedBestMove(DfpnData& data,
                                      const std::vector<DfpnData>& childrenData)
{
    if (!data.m_bounds.IsSolved())
        return;
    // Find the most delaying move for losing states, and the smallest
    // winning move for winning states.
    m_allSolvedEvaluation.Add(data.m_evaluationScore);
    if (data.m_bounds.IsLosing())
    {
        m_losingEvaluation.Add(data.m_evaluationScore);
        std::size_t maxWork = 0;
        for (std::size_t i = 0; i < data.m_children.Size(); ++i)
        {
            if (childrenData[i].m_work > maxWork)
            {
                maxWork = childrenData[i].m_work;
                data.m_bestMove = data.m_children.FirstMove(i);
            }
        }
    }
    else
    {
        m_winningEvaluation.Add(data.m_evaluationScore);
        std::size_t minWork = DfpnBounds::INFTY;
        for (std::size_t i = 0; i < data.m_children.Size(); ++i)
        {
            if (childrenData[i].m_bounds.IsLosing()
                && childrenData[i].m_work < minWork)
            {
                minWork = childrenData[i].m_work;
                data.m_bestMove = data.m_children.FirstMove(i);
            }
        }
    }
}

size_t DfpnSolver::MID(const DfpnBounds& maxBounds,
                       const size_t workBound, DfpnData& data)
{
    maxBounds.CheckConsistency();
    BenzeneAssert(maxBounds.phi > 1);
    BenzeneAssert(maxBounds.delta > 1);

    int depth = m_history->Depth();
    size_t work = CreateData(data);

    if (!maxBounds.GreaterThan(data.m_bounds))
    {
        if (work)
            TTWrite(*m_state, data);
        return work;
    }

    ++m_numMIDcalls;

    std::vector<DfpnData> childrenData(data.m_children.Size());

    SgHashCode currentHash = m_state->Hash();
    do
    {
        LookupChildren(childrenData, data.m_children);
        // Index used for progressive widening
        size_t maxChildIndex = ComputeMaxChildIndex(childrenData);

        if (m_useGuiFx && depth == 0)
            m_guiFx.SetChildren(data.m_children, childrenData);

        UpdateBounds(data.m_bounds, childrenData, maxChildIndex);

        if (m_useGuiFx && depth == 1)
        {
            m_guiFx.UpdateCurrentBounds(data.m_bounds);
            m_guiFx.Write();
        }

        if (!maxBounds.GreaterThan(data.m_bounds))
            break;

        if (work >= workBound)
            break;

        // Select most proving child
        DfpnBounds childMaxBounds;
        size_t bestIndex = SelectChild(childMaxBounds, data.m_bounds,
                                       childrenData,
                                       maxBounds, maxChildIndex);
        data.m_bestMove = data.m_children.FirstMove(bestIndex);
        // Recurse on best child
        if (m_useGuiFx && depth == 0)
            m_guiFx.PlayMove(m_state->ToPlay(), bestIndex);
        data.m_children.PlayMove(bestIndex, *m_state);
        m_history->Push(data.m_bestMove, currentHash);
        work += MID(childMaxBounds, workBound - work,
                    childrenData[bestIndex]);
        m_history->Pop();
        data.m_children.UndoMove(bestIndex, *m_state);

        if (m_useGuiFx && depth == 0)
            m_guiFx.UndoMove();

        UpdateStatsOnWin(childrenData, bestIndex, data.m_work + work);

        PruneChildren(data.m_children, childrenData, data.m_bestMove,
                      childrenData[bestIndex].m_maxProofSet);
    } while (!CheckAbort());

    if (m_useGuiFx && depth == 0)
        m_guiFx.WriteForced();

    UpdateSolvedBestMove(data, childrenData);

    // Store search results and notify listeners
    data.m_work += work;
    TTWrite(*m_state, data);
    if (data.m_bounds.IsSolved())
        NotifyListeners(*m_history, data);
    return work;
}

size_t DfpnSolver::ComputeMaxChildIndex(const std::vector<DfpnData>&
                                        childrenData) const
{
    BenzeneAssert(!childrenData.empty());

    int numNonLosingChildren = 0;
    for (size_t i = 0; i < childrenData.size(); ++i)
        if (!childrenData[i].m_bounds.IsWinning())
            ++numNonLosingChildren;
    if (numNonLosingChildren < 2)
        return childrenData.size();

    // this needs experimenting!
    int childrenToLookAt = WideningBase() 
        + int(ceil(float(numNonLosingChildren) * WideningFactor()));
    // Must examine at least two children when have two or more live,
    // since otherwise delta2 will be set to infinity in SelectChild.
    BenzeneAssert(childrenToLookAt >= 1);

    int numNonLosingSeen = 0;
    for (size_t i = 0; i < childrenData.size(); ++i)
    {
        if (!childrenData[i].m_bounds.IsWinning())
            if (++numNonLosingSeen == childrenToLookAt)
                return i + 1;
    }
    return childrenData.size();
}

void DfpnSolver::DeleteChildren(DfpnChildren& children,
                                std::vector<DfpnData>& childrenData,
                                bitset_t deleteChildren) const
{
    BenzeneAssert(children.Size() == childrenData.size());
    bitset_t deleted;
    std::vector<HexPoint>::iterator it1 = children.m_children.begin();
    std::vector<DfpnData>::iterator it2 = childrenData.begin();
    while (it1 != children.m_children.end())
    {
        BenzeneAssert(it2 != childrenData.end());
        if (deleteChildren.test(*it1))
        {
            BenzeneAssert(!deleted.test(*it1));
            deleted.set(*it1);
            it1 = children.m_children.erase(it1);
            it2 = childrenData.erase(it2);
        }
        else
        {
            ++it1;
            ++it2;
        }
    }
    BenzeneAssert(children.Size() > 0);
    BenzeneAssert(children.Size() == childrenData.size());
    BenzeneAssert(deleteChildren == deleted);
}

void DfpnSolver::NotifyListeners(const DfpnHistory& history,
                                 const DfpnData& data)
{
    for (std::size_t i = 0; i < m_listener.size(); ++i)
        m_listener[i]->StateSolved(history, data);
}

template <class T>
size_t DfpnSolver::SelectChild(DfpnBounds& childMaxBounds,
                               const DfpnBounds& currentBounds,
                               const std::vector<T>& childrenBounds,
                               const DfpnBounds& maxBounds,
                               size_t maxChildIndex)
{
    size_t bestIndex = 999999;
    DfpnBoundType delta2 = DfpnBounds::INFTY;
    DfpnBoundType delta1 = DfpnBounds::INFTY;

    BenzeneAssert(1 <= maxChildIndex && maxChildIndex <= childrenBounds.size());
    for (size_t i = 0; i < maxChildIndex; ++i)
    {
        const DfpnBounds& child = childrenBounds[i].GetBounds();

        // Store the child with smallest delta and record 2nd smallest delta
        if (child.delta < delta1)
        {
            delta2 = delta1;
            delta1 = child.delta;
            bestIndex = i;
        }
        else if (child.delta < delta2)
        {
            delta2 = child.delta;
        }

        // Winning move found
        if (child.IsLosing())
            break;
    }
    BenzeneAssert(delta1 < DfpnBounds::INFTY);

    // Compute maximum bound for child
    const DfpnBounds childBounds(childrenBounds[bestIndex].GetBounds());
    childMaxBounds.phi = maxBounds.delta - (currentBounds.delta - childBounds.phi);
    childMaxBounds.delta = delta2 ==
        DfpnBounds::INFTY ? maxBounds.phi :
            std::min(maxBounds.phi,
                std::max(delta2 + 1, DfpnBoundType(delta2 * (1.0 + m_epsilon))));
    BenzeneAssert(childMaxBounds.GreaterThan(childBounds));
    if (delta2 != DfpnBounds::INFTY)
        m_deltaIncrease.Add(float(childMaxBounds.delta-childBounds.delta));
    return bestIndex;
}

template <class T>
void DfpnSolver::UpdateBounds(DfpnBounds& bounds,
                              const std::vector<T>& childData,
                              size_t maxChildIndex) const
{
    DfpnBounds boundsAll(DfpnBounds::INFTY, 0);
    BenzeneAssert(1 <= maxChildIndex && maxChildIndex <= childData.size());
    for (std::size_t i = 0; i < childData.size(); ++i)
    {
        const DfpnBounds& childBounds = childData[i].GetBounds();
        // Abort on losing child (a winning move)
        if (childBounds.IsLosing())
        {
            DfpnBounds::SetToWinning(bounds);
            return;
        }
        if (i < maxChildIndex)
            boundsAll.phi = std::min(boundsAll.phi, childBounds.delta);
        BenzeneAssert(childBounds.phi != DfpnBounds::INFTY);
        boundsAll.delta += childBounds.phi;
    }
    if (boundsAll.phi == DfpnBounds::INFTY)
        boundsAll.delta = 0;
    bounds = boundsAll;
}

void DfpnSolver::LookupData(DfpnData& data, const DfpnChildren& children, 
                            std::size_t childIndex, HexState& state)
{
    children.PlayMove(childIndex, state);
    TTRead(state, data);
    children.UndoMove(childIndex, state);
}

void DfpnSolver::LookupChildren(std::vector<DfpnData>& childrenData,
                                const DfpnChildren& children)
{
    for (size_t i = 0; i < children.Size(); ++i)
        LookupData(childrenData[i], children, i, *m_state);
}

void DfpnSolver::LookupChildren(size_t depth,
                                std::vector<DfpnBounds>& virtualBounds,
                                const std::vector<DfpnData>& childrenData,
                                const DfpnChildren& children)
{
    for (size_t i = 0; i < children.Size(); ++i)
    {
        virtualBounds[i] = childrenData[i].GetBounds();
        children.PlayMove(i, *m_state);
        m_vtt.Lookup(depth, *m_state, virtualBounds[i]);
        children.UndoMove(i, *m_state);
    }
}

bool DfpnSolver::TTRead(const HexState& state, DfpnData& data)
{
    return m_positions->Get(state, data);
}

void DfpnSolver::TTWrite(const HexState& state, const DfpnData& data)
{
    data.m_bounds.CheckConsistency();
    m_positions->Put(state, data);
}

//----------------------------------------------------------------------------

void DfpnSolver::PropagateBackwards(const Game& game, DfpnStates& pos)
{
    HexState state(game.Board(), BLACK);
    MoveSequence history(game.History());
    if (history.empty())
        return;
    do
    {
        HexPoint cell = history.back().Point();
        state.UndoMove(cell);
        state.SetToPlay(history.back().Color());
        history.pop_back();
        DfpnData data;
        if (!pos.Get(state, data))
            break;
        if (data.m_bounds.IsSolved())
            break;
        std::vector<DfpnData> childrenData(data.m_children.Size());
        for (size_t i = 0; i < data.m_children.Size(); ++i)
            LookupData(childrenData[i], data.m_children, i, state);
        size_t maxChildIndex = ComputeMaxChildIndex(childrenData);
        UpdateBounds(data.m_bounds, childrenData, maxChildIndex);
        pos.Put(state, data);
    } while(!history.empty());
}

//----------------------------------------------------------------------------

