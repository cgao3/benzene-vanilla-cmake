//----------------------------------------------------------------------------
/** @file DfpnSolver.cpp
 */
//----------------------------------------------------------------------------

#include "BitsetIterator.hpp"
#include "BoardUtils.hpp"
#include "DfpnSolver.hpp"
#include "PatternState.hpp"
#include "EndgameUtils.hpp"
#include "ProofUtil.hpp"
#include "Resistance.hpp"

#include <boost/filesystem/path.hpp>

using namespace benzene;

using std::ceil;

//----------------------------------------------------------------------------

#ifndef NDEBUG
void DfpnBounds::CheckConsistency() const
{
    // Check range
    HexAssert(phi <= INFTY);
    HexAssert(delta <= INFTY);
    // If 0 then other must be infinity
    HexAssert(0 != phi || INFTY == delta);
    HexAssert(0 != delta || INFTY == phi);
    // Special case for root: if infinity other must be 0 or infinity
    HexAssert(INFTY != phi || 0 == delta || INFTY == delta);
    HexAssert(INFTY != delta || 0 == phi || INFTY == phi);
}
#else
void DfpnBounds::CheckConsistency() const
{
}
#endif

//----------------------------------------------------------------------------

DfpnChildren::DfpnChildren()
{
}

void DfpnChildren::SetChildren(const std::vector<HexPoint>& children)
{
    m_children = children;
}

void DfpnChildren::PlayMove(int index, StoneBoard& brd, HexColor color) const
{
    brd.PlayMove(color, m_children[index]);
}

void DfpnChildren::UndoMove(int index, StoneBoard& brd) const
{
    brd.UndoMove(m_children[index]);
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
    : m_index(-1),
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

void DfpnSolver::GuiFx::PlayMove(HexColor color, int index)
{
    m_color = color;
    m_index = index;
}

void DfpnSolver::GuiFx::UndoMove()
{
    m_index = -1;
}

void DfpnSolver::GuiFx::UpdateCurrentBounds(const DfpnBounds& bounds)
{
    HexAssert(m_index != -1);
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
    if (m_index != -1)
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
        + sizeof(HexPoint) * (m_children.Size() + 1);
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
    const std::vector<HexPoint>& moves = m_children.m_children;
    for (std::size_t i = 0; i < moves.size(); ++i)
    {
        *reinterpret_cast<HexPoint*>(off) = moves[i];
        off += sizeof(HexPoint);
    }
    *reinterpret_cast<HexPoint*>(off) = INVALID_POINT;
    off += sizeof(HexPoint);
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
    std::vector<HexPoint> moves;
    while (true)
    {
        HexPoint p = *reinterpret_cast<const HexPoint*>(data);
        data += sizeof(HexPoint);
        if (p == INVALID_POINT)
            break;
        moves.push_back(p);
    }
    m_children.SetChildren(moves);
}

void DfpnData::Rotate(const ConstBoard& brd)
{
    if (m_bestMove != INVALID_POINT)
        m_bestMove = BoardUtils::Rotate(brd, m_bestMove);
    m_maxProofSet = BoardUtils::Rotate(brd, m_maxProofSet);
    std::vector<HexPoint>& moves = m_children.m_children;
    for (std::size_t i = 0; i < moves.size(); ++i)
        moves[i] = BoardUtils::Rotate(brd, moves[i]);
}

//----------------------------------------------------------------------------

DfpnSolver::DfpnSolver()
    : m_positions(0),
      m_useGuiFx(false),
      m_timelimit(0.0),
      m_wideningBase(1),
      m_wideningFactor(0.25f),
      m_guiFx()
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
       << "MID calls       " << m_numMIDcalls << '\n'
       << "VC builds       " << m_numVCbuilds << '\n'
       << "Terminal nodes  " << m_numTerminal << '\n'
       << "Work            " << m_numMIDcalls + m_numTerminal << '\n'
       << "Wasted Work     " << m_totalWastedWork
       << " (" << (m_totalWastedWork * 100.0 
                   / (m_numMIDcalls + m_numTerminal)) << "%)\n"
       << "Elapsed Time    " << m_timer.GetTime() << '\n'
       << "MIDs/sec        " << m_numMIDcalls / m_timer.GetTime() << '\n'
       << "VCs/sec         " << m_numVCbuilds / m_timer.GetTime() << '\n'
       << "Cnt prune sib   " << m_prunedSiblingStats.Count() << '\n'
       << "Avg prune sib   ";
    m_prunedSiblingStats.Write(os);
    os << '\n'
       << "Consider Size   ";
    m_considerSetSize.Write(os);
    os << '\n'
       << "Move Index      ";
    m_moveOrderingIndex.Write(os);
    os << '\n';
    os << "Move Percent    ";
    m_moveOrderingPercent.Write(os);
    os << '\n'
       << "Winner          " << winner << '\n'
       << "PV              " << HexPointUtil::ToString(pv) << '\n';
    if (m_positions->Database())
        os << '\n' << m_positions->Database()->GetStatistics().Write() << '\n';
    if (m_positions->HashTable())    
        os << '\n' << m_positions->HashTable()->Stats() << '\n';
    LogInfo() << os.str();
}

HexColor DfpnSolver::StartSearch(HexBoard& board, HexColor colorToMove,
                                 DfpnPositions& positions, PointSequence& pv)
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

    m_brd.reset(new StoneBoard(board.GetState()));
    m_workBoard = &board;
    m_checkTimerAbortCalls = 0;

    // Skip search if already solved
    DfpnData data;
    if (TTRead(*m_brd, data) && data.m_bounds.IsSolved())
    {
        LogInfo() << "Already solved!\n";
        HexColor w = data.m_bounds.IsWinning() ? colorToMove : !colorToMove;
        SolverDBUtil::GetVariation(*m_brd, colorToMove, *m_positions, pv);
        LogInfo() << w << " wins!\n";
        LogInfo() << "PV: " << HexPointUtil::ToString(pv) << '\n';
        return w;
    }

    DfpnBounds root(INFTY, INFTY);
    m_timer.Start();
    DfpnHistory history;
    MID(root, history, colorToMove);
    m_timer.Stop();

    SolverDBUtil::GetVariation(*m_brd, colorToMove, *m_positions, pv);
    HexColor winner = EMPTY;
    if (TTRead(*m_brd, data) && data.m_bounds.IsSolved())
        winner = data.m_bounds.IsWinning() ? colorToMove : !colorToMove;
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
                }
                else
                {
                    if (m_numMIDcalls < 100)
                        m_checkTimerAbortCalls = 10;
                    else
                    {
                        size_t midsPerSec = static_cast<size_t>
                            (m_numMIDcalls / elapsed);
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


size_t DfpnSolver::MID(const DfpnBounds& bounds, DfpnHistory& history,
                       HexColor colorToMove)
{
    bounds.CheckConsistency();
    HexAssert(bounds.phi > 1);
    HexAssert(bounds.delta > 1);

    int depth = history.Depth();
    size_t prevWork = 0;
    bitset_t maxProofSet;
    DfpnChildren children;
    {
        DfpnData data;
        if (TTRead(*m_brd, data)) 
        {
            children = data.m_children;
            maxProofSet = data.m_maxProofSet;
            prevWork = data.m_work;
            HexAssert(bounds.phi > data.m_bounds.phi);
            HexAssert(bounds.delta > data.m_bounds.delta);
        }
        else
        {
            m_workBoard->GetState().SetState(*m_brd);
            m_workBoard->ComputeAll(colorToMove);
            ++m_numVCbuilds;

            // Compute the maximum possible proof set if colorToMove wins.
            // This data is used to prune siblings of this state.
            maxProofSet = ProofUtil::MaximumProofSet(*m_workBoard, colorToMove);

            if (EndgameUtils::IsDeterminedState(*m_workBoard, colorToMove))
            {
                ++m_numTerminal;
                DfpnBounds terminal;
                if (EndgameUtils::IsWonGame(*m_workBoard, colorToMove))
                    DfpnBounds::SetToWinning(terminal);
                else 
                    DfpnBounds::SetToLosing(terminal);
                
                if (m_useGuiFx && depth == 1)
                {
                    m_guiFx.UpdateCurrentBounds(terminal);
                    m_guiFx.Write();
                }
                TTWrite(*m_brd, DfpnData(terminal, DfpnChildren(), 
                                         INVALID_POINT, 1, maxProofSet));
                return 1;
            }
            bitset_t childrenBitset 
                = EndgameUtils::MovesToConsider(*m_workBoard, colorToMove);

            m_considerSetSize.Add(childrenBitset.count());
            Resistance resist;
            resist.Evaluate(*m_workBoard);
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
            children.SetChildren(sortedChildren);
        }
    }

    ++m_numMIDcalls;
    size_t localWork = 1;

    // Not thread safe: perhaps move into while loop below later...
    std::vector<DfpnData> childrenData(children.Size());
    for (size_t i = 0; i < children.Size(); ++i)
        LookupData(childrenData[i], children, i, colorToMove);
    // Index used for progressive widening
    size_t maxChildIndex = ComputeMaxChildIndex(childrenData);

    if (m_useGuiFx && depth == 0)
        m_guiFx.SetChildren(children, childrenData);

    hash_t currentHash = m_brd->Hash();   
    HexPoint bestMove = INVALID_POINT;
    DfpnBounds currentBounds;
    do
    {
        UpdateBounds(currentBounds, childrenData, maxChildIndex);

        if (m_useGuiFx && depth == 1)
        {
            m_guiFx.UpdateCurrentBounds(currentBounds);
            m_guiFx.Write();
        }

        if (bounds.phi <= currentBounds.phi 
            || bounds.delta <= currentBounds.delta)
        {
            break;
        }

        // Select most proving child
        int bestIndex = -1;
        std::size_t delta2 = INFTY;
        SelectChild(bestIndex, delta2, childrenData, maxChildIndex);
        DfpnBounds child(childrenData[bestIndex].m_bounds);
        bestMove = children.FirstMove(bestIndex);

        // Update thresholds
        child.phi = bounds.delta - (currentBounds.delta - child.phi);
        child.delta = std::min(bounds.phi, delta2 + 1);
        HexAssert(child.phi > childrenData[bestIndex].m_bounds.phi);
        HexAssert(child.delta > childrenData[bestIndex].m_bounds.delta);

        if (m_useGuiFx && depth == 0)
            m_guiFx.PlayMove(colorToMove, bestIndex);

        // Recurse on best child
        children.PlayMove(bestIndex, *m_brd, colorToMove);
        history.Push(bestMove, currentHash); // FIXME: handle sequences!
        localWork += MID(child, history, !colorToMove);
        history.Pop();
        children.UndoMove(bestIndex, *m_brd);

        if (m_useGuiFx && depth == 0)
            m_guiFx.UndoMove();

        // Update bounds for best child
        LookupData(childrenData[bestIndex], children, bestIndex, colorToMove);

        // Compute some stats when find winning move
        if (childrenData[bestIndex].m_bounds.IsLosing())
        {
            m_moveOrderingIndex.Add(bestIndex);
            m_moveOrderingPercent.Add(bestIndex / (double)childrenData.size());
            m_totalWastedWork += prevWork + localWork
                - childrenData[bestIndex].m_work;
        }
        else if (childrenData[bestIndex].m_bounds.IsWinning())
            maxChildIndex = ComputeMaxChildIndex(childrenData);

        // Shrink children list using knowledge of bestMove child's proof set.
        // That is, if this child is losing, conclude what other children
        // must also be losing (i.e. cannot interfere with the proof set
        // that disproves this child).
        // And of course if this child is winning, no need to explore
        // these other siblings either.
        {
            /* @todo Perhaps track allChildren instead of recomputing? */
            bitset_t allChildren;
            for (std::vector<HexPoint>::iterator it
                     = children.m_children.begin();
                 it != children.m_children.end(); ++it)
            {
                allChildren.set(*it);
            }
            bitset_t canPrune = allChildren
                - childrenData[bestIndex].m_maxProofSet;
            canPrune.reset(bestMove);
            int pruneCount = canPrune.count();

            if (pruneCount)
            {
                m_prunedSiblingStats.Add(pruneCount);
                /*
                LogInfo() << "Pruning " << pruneCount
                          << " moves via " << bestMove
                          << ".\nChildren:\n" << m_brd->Write(allChildren)
                          << "\nRemoving...\n" << m_brd->Write(canPrune)
                          << "\n";
                */
                DeleteChildren(children, childrenData, canPrune);
                maxChildIndex = ComputeMaxChildIndex(childrenData);
                if (m_useGuiFx && depth == 0)
                    m_guiFx.SetChildren(children, childrenData);
            }
        }
    } while (!CheckAbort());

    if (m_useGuiFx && depth == 0)
        m_guiFx.WriteForced();

    // Find the most delaying move for losing states, and the smallest
    // winning move for winning states.
    if (currentBounds.IsSolved())
    {
        if (currentBounds.IsLosing())
        {
            std::size_t maxWork = 0;
            for (std::size_t i = 0; i < children.Size(); ++i)
            {
                if (childrenData[i].m_work > maxWork)
                {
                    maxWork = childrenData[i].m_work;
                    bestMove = children.FirstMove(i);
                }
            }
        }
        else
        {
            std::size_t minWork = INFTY;
            for (std::size_t i = 0; i < children.Size(); ++i)
            {
                if (childrenData[i].m_bounds.IsLosing() 
                    && childrenData[i].m_work < minWork)
                {
                    minWork = childrenData[i].m_work;
                    bestMove = children.FirstMove(i);
                }
            }
        }
    }
    
    // Store search results and notify listeners
    DfpnData data(currentBounds, children, bestMove, localWork + prevWork, 
                  maxProofSet);
    TTWrite(*m_brd, data);
    if (data.m_bounds.IsSolved())
        NotifyListeners(history, data);
    return localWork;
}

size_t DfpnSolver::ComputeMaxChildIndex(const std::vector<DfpnData>&
                                        childrenData) const
{
    int numNonLosingChildren = 0;
    for (size_t i = 0; i < childrenData.size(); ++i)
        if (!childrenData[i].m_bounds.IsWinning())
            ++numNonLosingChildren;
    if (numNonLosingChildren < 2)
        return childrenData.size();

    // this needs experimenting!
    int childrenToLookAt = WideningBase() + (int) ceil(numNonLosingChildren
                                                       * WideningFactor());
    // Must examine at least two children when have two or more live,
    // since otherwise delta2 will be set to infinity in SelectChild.
    HexAssert(childrenToLookAt >= 2);

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
    HexAssert(children.Size() == childrenData.size());
    bitset_t deleted;
    std::vector<HexPoint>::iterator it1 = children.m_children.begin();
    std::vector<DfpnData>::iterator it2 = childrenData.begin();
    while (it1 != children.m_children.end())
    {
        HexAssert(it2 != childrenData.end());
        if (deleteChildren.test(*it1))
        {
            HexAssert(!deleted.test(*it1));
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
    HexAssert(children.Size() > 0);
    HexAssert(children.Size() == childrenData.size());
    HexAssert(deleteChildren == deleted);
}

void DfpnSolver::NotifyListeners(const DfpnHistory& history,
                                 const DfpnData& data)
{
    for (std::size_t i = 0; i < m_listener.size(); ++i)
        m_listener[i]->StateSolved(history, data);
}

void DfpnSolver::SelectChild(int& bestIndex, std::size_t& delta2,
                             const std::vector<DfpnData>& childrenData,
                             size_t maxChildIndex) const
{
    std::size_t delta1 = INFTY;

    HexAssert(1 <= maxChildIndex && maxChildIndex <= childrenData.size());
    for (std::size_t i = 0; i < maxChildIndex; ++i)
    {
        const DfpnBounds& child = childrenData[i].m_bounds;

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
    HexAssert(delta1 < INFTY);
}

void DfpnSolver::UpdateBounds(DfpnBounds& bounds, 
                              const std::vector<DfpnData>& childData,
                              size_t maxChildIndex) const
{
    DfpnBounds boundsAll(INFTY, 0);
    HexAssert(1 <= maxChildIndex && maxChildIndex <= childData.size());
    for (std::size_t i = 0; i < maxChildIndex; ++i)
    {
        const DfpnBounds& childBounds = childData[i].m_bounds;
        // Abort on losing child (a winning move)
        if (childBounds.IsLosing())
        {
            HexAssert(INFTY == childBounds.phi);
            DfpnBounds::SetToWinning(bounds);
            return;
        }
        boundsAll.phi = std::min(boundsAll.phi, childBounds.delta);
        HexAssert(childBounds.phi != INFTY);
        boundsAll.delta += childBounds.phi;
    }
    bounds = boundsAll;
}

void DfpnSolver::LookupData(DfpnData& data, const DfpnChildren& children, 
                            int childIndex, HexColor colorToMove)
{
    children.PlayMove(childIndex, *m_brd, colorToMove);
    if (!TTRead(*m_brd, data))
    {
        data.m_bounds.phi = 1;
        data.m_bounds.delta = 1;
        data.m_work = 0;
    }
    children.UndoMove(childIndex, *m_brd);
}

bool DfpnSolver::TTRead(const StoneBoard& brd, DfpnData& data)
{
    return m_positions->Get(brd, data);
}

void DfpnSolver::TTWrite(const StoneBoard& brd, const DfpnData& data)
{
    data.m_bounds.CheckConsistency();
    m_positions->Put(brd, data);
}

//----------------------------------------------------------------------------
