//----------------------------------------------------------------------------
/** @file DfpnSolver.cpp
 */
//----------------------------------------------------------------------------

#include "BitsetIterator.hpp"
#include "BoardUtils.hpp"
#include "DfpnSolver.hpp"
#include "PatternState.hpp"
#include "PlayerUtils.hpp"
#include "Resistance.hpp"

#include <boost/filesystem/path.hpp>

using namespace benzene;

using std::ceil;

//----------------------------------------------------------------------------

/** If true, uses PositionDB instead of TransTable. */
#define USE_DB 0

//----------------------------------------------------------------------------

DfpnChildren::DfpnChildren()
{
}

void DfpnChildren::SetChildren(const std::vector<HexPoint>& children)
{
    m_children = children;
}

void DfpnChildren::PlayMove(int index, StoneBoard& brd) const
{
    brd.PlayMove(brd.WhoseTurn(), m_children[index]);
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
    const std::vector<HexPoint>& moves = m_children.m_children;
    for (std::size_t i = 0; i < moves.size(); ++i)
    {
        *reinterpret_cast<HexPoint*>(off) = moves[i];
        off += sizeof(HexPoint);
    }
    *reinterpret_cast<HexPoint*>(off) = INVALID_POINT;
    off += sizeof(HexPoint);
    if (off - data != PackedSize())
        throw HexException("Bad size!");
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
    m_bestMove = BoardUtils::Rotate(brd, m_bestMove);
    std::vector<HexPoint>& moves = m_children.m_children;
    for (std::size_t i = 0; i < moves.size(); ++i)
        moves[i] = BoardUtils::Rotate(brd, moves[i]);
}

//----------------------------------------------------------------------------

DfpnSolver::DfpnSolver()
    : m_hashTable(0),
      m_useGuiFx(false),
      m_timelimit(0.0),
      m_wideningFactor(0.25f),
      m_guiFx()
{
}

DfpnSolver::~DfpnSolver()
{
}

void DfpnSolver::GetVariation(const StoneBoard& state, 
                              std::vector<HexPoint>& pv)
{
    StoneBoard brd(state);
    while (true) 
    {
        DfpnData data;
        if (!TTRead(brd, data))
            break;
        if (data.m_bestMove == INVALID_POINT)
            break;
        pv.push_back(data.m_bestMove);
        brd.PlayMove(brd.WhoseTurn(), data.m_bestMove);
    }
}

std::string DfpnSolver::PrintVariation(const std::vector<HexPoint>& pv) const
{
    std::ostringstream os;
    for (std::size_t i = 0; i < pv.size(); ++i) 
    {
        if (i) os << ' ';
        os << pv[i];
    }
    return os.str();
}

void DfpnSolver::PrintStatistics()
{
    LogInfo() << "     MID calls: " << m_numMIDcalls << '\n';
    LogInfo() << "     VC builds: " << m_numVCbuilds << '\n';
    LogInfo() << "Terminal nodes: " << m_numTerminal << '\n';
    LogInfo() << " Cnt prune sib: " << m_prunedSiblingStats.Count() << '\n';
    std::ostringstream os;
    os << " Avg prune sib: ";
    m_prunedSiblingStats.Write(os);
    os << '\n';
    LogInfo() << os.str();
    os.str("");
    os << " Consider Size: ";
    m_considerSetSize.Write(os);
    os << '\n';
    LogInfo() << os.str();
    os.str("");
    os << "    Move Index: ";
    m_moveOrderingIndex.Write(os);
    os << '\n';
    LogInfo() << os.str();
    os.str("");
    os << "  Move Percent: ";
    m_moveOrderingPercent.Write(os);
    os << '\n';
    LogInfo() << os.str();
    LogInfo() << "   Wasted Work: " << m_totalWastedWork
              << " (" << (m_totalWastedWork * 100.0 
                          / (m_numMIDcalls + m_numTerminal)) << "%)\n";
    LogInfo() << "  Elapsed Time: " << m_timer.GetTime() << '\n';
    LogInfo() << "      MIDs/sec: " << m_numMIDcalls / m_timer.GetTime()<<'\n';
    LogInfo() << "       VCs/sec: " << m_numVCbuilds / m_timer.GetTime()<<'\n';
#if USE_DB
    LogInfo() << m_db->GetStatistics().Write() << '\n';
#else
    LogInfo() << m_hashTable->Stats() << '\n';
#endif
}

HexColor DfpnSolver::StartSearch(HexBoard& board, DfpnHashTable& hashtable,
                                 PointSequence& pv)
{
    m_aborted = false;
    m_hashTable = &hashtable;
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

#if USE_DB
    boost::scoped_ptr<DfpnDB> db(new DfpnDB("test.db"));
    m_db = db.get();
#else
    m_db = 0;
#endif

    bool performSearch = true;
    {
        // Skip search if already solved
        DfpnData data;
        if (TTRead(*m_brd, data) && data.m_bounds.IsSolved())
            performSearch = false;
    }

    if (performSearch)
    {
        DfpnBounds root(INFTY, INFTY);
        m_timer.Start();
        DfpnHistory history;
        MID(root, history);
        m_timer.Stop();
        PrintStatistics();
    }

    if (!m_aborted)
    {
        DfpnData data;
        TTRead(*m_brd, data);
        CheckBounds(data.m_bounds);

        HexColor colorToMove = m_brd->WhoseTurn();
        HexColor winner = data.m_bounds.IsWinning() 
            ? colorToMove : !colorToMove;
        LogInfo() << winner << " wins!\n";

        pv.clear();
        GetVariation(*m_brd, pv);
        LogInfo() << "PV: " << PrintVariation(pv) << '\n';

        return winner;
    }
    LogInfo() << "Search aborted.\n";
    return EMPTY;
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


size_t DfpnSolver::MID(const DfpnBounds& bounds, DfpnHistory& history)
{
    CheckBounds(bounds);
    HexAssert(bounds.phi > 1);
    HexAssert(bounds.delta > 1);

    if (CheckAbort())
        return 0;

    int depth = history.Depth();
    HexColor colorToMove = m_brd->WhoseTurn();

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
            // Compute the maximum possible proof set if !colorToMove wins.
            // This data is used to prune siblings of this state.
            maxProofSet = m_workBoard->GetState().GetEmpty()
                | m_workBoard->GetState().GetPlayed(colorToMove)
                | m_workBoard->GetInferiorCells().DeductionSet(colorToMove);

            if (PlayerUtils::IsDeterminedState(*m_workBoard, colorToMove))
            {
                ++m_numTerminal;
                DfpnBounds terminal;
                if (PlayerUtils::IsWonGame(*m_workBoard, colorToMove))
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
                = PlayerUtils::MovesToConsider(*m_workBoard, colorToMove);

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
        LookupData(childrenData[i], children, i);
    // Index used for progressive widening
    size_t maxChildIndex = ComputeMaxChildIndex(childrenData);

    if (m_useGuiFx && depth == 0)
        m_guiFx.SetChildren(children, childrenData);

    hash_t currentHash = m_brd->Hash();   
    HexPoint bestMove = INVALID_POINT;
    DfpnBounds currentBounds;
    while (!m_aborted) 
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
        children.PlayMove(bestIndex, *m_brd);
        history.Push(bestMove, currentHash); // FIXME: handle sequences!
        localWork += MID(child, history);
        history.Pop();
        children.UndoMove(bestIndex, *m_brd);

        if (m_useGuiFx && depth == 0)
            m_guiFx.UndoMove();

        // Update bounds for best child
        LookupData(childrenData[bestIndex], children, bestIndex);

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
    }

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
    if (!m_aborted)
    {
        DfpnData data(currentBounds, children, bestMove,
                      localWork + prevWork, maxProofSet);
        TTWrite(*m_brd, data);
        if (data.m_bounds.IsSolved())
            NotifyListeners(history, data);
    }
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
    int childrenToLookAt = 1 + (int) ceil(numNonLosingChildren
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
        CheckBounds(child);

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
                            int childIndex)
{
    children.PlayMove(childIndex, *m_brd);
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
#if USE_DB
    return m_db->Get(brd, data);
#else
    return m_hashTable->Get(brd.Hash(), data);
#endif
}

void DfpnSolver::TTWrite(const StoneBoard& brd, const DfpnData& data)
{
    CheckBounds(data.m_bounds);
#if USE_DB
    m_db->Put(brd, data);
#else
    m_hashTable->Put(brd.Hash(), data);
#endif
}

#ifndef NDEBUG
void DfpnSolver::CheckBounds(const DfpnBounds& bounds) const
{
    HexAssert(bounds.phi <= INFTY);
    HexAssert(bounds.delta <= INFTY);
    HexAssert(0 != bounds.phi || INFTY == bounds.delta);
    HexAssert(0 != bounds.delta || INFTY == bounds.phi);
    HexAssert(INFTY!= bounds.phi || 0 == bounds.delta ||INFTY == bounds.delta);
    HexAssert(INFTY!= bounds.delta || 0 == bounds.phi ||INFTY == bounds.phi);
}
#else
void DfpnSolver::CheckBounds(const DfpnBounds& bounds) const
{
    SG_UNUSED(bounds);
}
#endif

//----------------------------------------------------------------------------
