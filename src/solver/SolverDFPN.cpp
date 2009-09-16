//----------------------------------------------------------------------------
/** @file SolverDFPN.cpp
 */
//----------------------------------------------------------------------------

#include "SolverDFPN.hpp"
#include "PlayerUtils.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

SolverDFPN::GuiFx::GuiFx()
    : m_move(INVALID_POINT),
      m_timeOfLastWrite(0.0),
      m_delay(1.0)
{
}

void SolverDFPN::GuiFx::SetChildren(const std::vector<HexPoint>& children,
                                    const std::vector<DfpnData>& data)
{
    m_children = children;
    m_data = data;
}

void SolverDFPN::GuiFx::PlayMove(HexColor color, HexPoint move) 
{
    m_color = color;
    m_move = move;
}

void SolverDFPN::GuiFx::UndoMove()
{
    m_move = INVALID_POINT;
}

void SolverDFPN::GuiFx::UpdateCurrentBounds(const DfpnBounds& bounds)
{
    HexAssert(m_move != INVALID_POINT);
    for (std::size_t i = 0; i < m_children.size(); ++i)
        if (m_children[i] == m_move)
            m_data[i].m_bounds = bounds;
}

/** Always writes output. */
void SolverDFPN::GuiFx::WriteForced()
{
    DoWrite();
}

/** Writes output only if last write was more than m_delay seconds
    ago or if the move is different. */
void SolverDFPN::GuiFx::Write()
{
    double currentTime = SgTime::Get();
    if (m_moveAtLastWrite == m_move)
    {
        if (currentTime < m_timeOfLastWrite + m_delay)
            return;
    }
    m_timeOfLastWrite = currentTime;
    m_moveAtLastWrite = m_move;
    DoWrite();
}

/** Writes progress indication. */
void SolverDFPN::GuiFx::DoWrite()
{
    std::ostringstream os;
    os << "gogui-gfx:\n";
    os << "dfpn\n";
    os << "VAR";
    if (m_move != INVALID_POINT)
        os << ' ' << (m_color == BLACK ? 'B' : 'W') << ' ' << m_move;
    os << '\n';
    os << "LABEL";
    int numLosses = 0;
    for (std::size_t i = 0; i < m_children.size(); ++i)
    {
        os << ' ' << m_children[i];
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
    os << numLosses << '/' << m_children.size() << " proven losses\n";
    os << '\n';
    std::cout << os.str();
    std::cout.flush();
}

//----------------------------------------------------------------------------

void DfpnHistory::NotifyCommonAncestor(DfpnHashTable& hashTable, 
                                       DfpnData data, DfpnStatistics& stats)
{
//     for (std::size_t i = 0; i < m_hash.size(); ++i)
//         LogInfo() << i << ": " << HashUtil::toString(m_hash[i]) 
//                   << ' ' << m_move[i] << '\n';
    std::size_t length = 1;
    for (std::size_t i = m_hash.size() - 1; i > 0; --i, ++length)
    {
//         LogInfo() << "cur: " << HashUtil::toString(data.m_parentHash) << ' '
//                   << data.m_moveParentPlayed << '\n';
        if (std::find(m_hash.begin(), m_hash.end(), data.m_parentHash) 
            != m_hash.end())
        {
//             LogInfo() << "Found it at i = " << i 
//                       << " length = " << length << '\n';
            stats.Add(length);
            // fill slot at ith entry
            break;
        }
        if (!hashTable.Get(data.m_parentHash, data))
            break;
    }
}

//----------------------------------------------------------------------------

SolverDFPN::SolverDFPN()
    : m_hashTable(0),
      m_useGuiFx(false),
      m_timelimit(0.0),
      m_guiFx()
{
}

SolverDFPN::~SolverDFPN()
{
}

void SolverDFPN::GetVariation(const StoneBoard& state, 
                              std::vector<HexPoint>& pv) const
{
    StoneBoard brd(state);
    while (true) 
    {
        DfpnData data;
        if (!m_hashTable->Get(brd.Hash(), data))
            break;
        if (data.m_bestMove == INVALID_POINT)
            break;
        pv.push_back(data.m_bestMove);
        brd.playMove(brd.WhoseTurn(), data.m_bestMove);
    }
}

std::string SolverDFPN::PrintVariation(const std::vector<HexPoint>& pv) const
{
    std::ostringstream os;
    for (std::size_t i = 0; i < pv.size(); ++i) 
    {
        if (i) os << ' ';
        os << pv[i];
    }
    return os.str();
}

HexColor SolverDFPN::StartSearch(HexBoard& board, DfpnHashTable& hashtable)
{
    m_aborted = false;
    m_hashTable = &hashtable;
    m_numTerminal = 0;
    m_numTranspositions = 0;
    m_transStats.Clear();
    m_numMIDcalls = 0;
    m_brd.reset(new StoneBoard(board));
    m_workBoard = &board;
    m_checkTimerAbortCalls = 0;

    DfpnBounds root(INFTY, INFTY);
    m_timer.Start();
    DfpnHistory history;
    MID(root, history);
    m_timer.Stop();

    LogInfo() << "     MID calls: " << m_numMIDcalls << "\n";
    LogInfo() << "Terminal nodes: " << m_numTerminal << "\n";
    LogInfo() << "Transpositions: " << m_numTranspositions << '\n';
    std::ostringstream os;
    os << " Trans. Length: ";
    m_transStats.Write(os);
    LogInfo() << os.str() << '\n';
    LogInfo() << "  Elapsed Time: " << m_timer.GetTime() << '\n';
    LogInfo() << "      MIDs/sec: " << m_numMIDcalls / m_timer.GetTime() << '\n';
    LogInfo() << m_hashTable->Stats() << '\n';

    if (!m_aborted)
    {
        DfpnData data;
        m_hashTable->Get(m_brd->Hash(), data);
        CheckBounds(data.m_bounds);

        HexColor colorToMove = m_brd->WhoseTurn();
        HexColor winner = data.m_bounds.IsWinning() 
            ? colorToMove : !colorToMove;
        LogInfo() << winner << " wins!\n";

        std::vector<HexPoint> pv;
        GetVariation(*m_brd, pv);
        LogInfo() << "PV: " << PrintVariation(pv) << '\n';
        LogInfo() << "Work: " << data.m_work << '\n';

        return winner;
    }
    LogInfo() << "Search aborted.\n";
    return EMPTY;
}

bool SolverDFPN::CheckAbort()
{
    if (!m_aborted)
    {
        if (SgUserAbort()) 
        {
            m_aborted = true;
            LogInfo() << "SolverDFPN::CheckAbort(): Abort flag!\n";
        }
        else if (m_timelimit > 0)
        {
            if (m_checkTimerAbortCalls == 0)
            {
                double elapsed = m_timer.GetTime();
                if (elapsed > m_timelimit)
                {
                    m_aborted = true;
                    LogInfo() << "SolverDFPN::CheckAbort(): Timelimit!" << '\n';
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


size_t SolverDFPN::MID(const DfpnBounds& bounds, DfpnHistory& history)
{
    CheckBounds(bounds);
    HexAssert(bounds.phi > 1);
    HexAssert(bounds.delta > 1);

    if (CheckAbort())
        return 0;

    int depth = history.Depth();
    hash_t parentHash = history.LastHash();
    HexColor colorToMove = m_brd->WhoseTurn();

    bitset_t childrenSet;
    {
        DfpnData data;
        if (m_hashTable->Get(m_brd->Hash(), data)) 
        {
            childrenSet = data.m_children;
            HexAssert(bounds.phi > data.m_bounds.phi);
            HexAssert(bounds.delta > data.m_bounds.delta);

            // Check if our stored parent differs from our current parent
            if (data.m_parentHash != parentHash)
            {
                ++m_numTranspositions;
                history.NotifyCommonAncestor(*m_hashTable, data, m_transStats);
            }
        }
        else
        {
            m_workBoard->SetState(*m_brd);
            m_workBoard->ComputeAll(colorToMove);
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
                TTStore(m_brd->Hash(), 
                        DfpnData(terminal, EMPTY_BITSET, INVALID_POINT, 1, 
                                 parentHash, history.LastMove()));
                return 1;
            }
            childrenSet = PlayerUtils::MovesToConsider(*m_workBoard, 
                                                       colorToMove);
        }
    }

    ++m_numMIDcalls;
    size_t localWork = 1;

    HexAssert(childrenSet.any());
    std::vector<HexPoint> children;
    BitsetUtil::BitsetToVector(childrenSet, children);

    // Not thread safe: perhaps move into while loop below later...
    std::vector<DfpnData> childrenData(children.size());
    for (size_t i = 0; i < children.size(); ++i)
        LookupData(childrenData[i], colorToMove, children[i]);
    if (m_useGuiFx && depth == 0)
        m_guiFx.SetChildren(children, childrenData);

    hash_t currentHash = m_brd->Hash();   
    HexPoint bestMove = INVALID_POINT;
    DfpnBounds currentBounds;
    while (!m_aborted) 
    {
        UpdateBounds(currentHash, currentBounds, childrenData);
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
        SelectChild(bestIndex, delta2, childrenData);
        DfpnBounds child(childrenData[bestIndex].m_bounds);
        bestMove = children[bestIndex];

        // Update thresholds
        child.phi = bounds.delta - (currentBounds.delta - child.phi);
        child.delta = std::min(bounds.phi, delta2 + 1);
        HexAssert(child.phi > childrenData[bestIndex].m_bounds.phi);
        HexAssert(child.delta > childrenData[bestIndex].m_bounds.delta);

        if (m_useGuiFx && depth == 0)
            m_guiFx.PlayMove(colorToMove, bestMove);

        // Recurse on best child
        m_brd->playMove(colorToMove, bestMove);
        history.PushBack(bestMove, currentHash);
        localWork += MID(child, history);
        history.Pop();
        m_brd->undoMove(bestMove);

        if (m_useGuiFx && depth == 0)
            m_guiFx.UndoMove();

        // Update bounds for best child
        LookupData(childrenData[bestIndex], colorToMove, bestMove);
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
            for (std::size_t i = 0; i < children.size(); ++i)
            {
                HexAssert(childrenData[i].m_bounds.IsWinning());
                if (childrenData[i].m_work > maxWork)
                {
                    maxWork = childrenData[i].m_work;
                    bestMove = children[i];
                    break;
                }
            }
        }
        else
        {
            std::size_t minWork = INFTY;
            for (std::size_t i = 0; i < children.size(); ++i)
            {
                if (childrenData[i].m_bounds.IsLosing() 
                    && childrenData[i].m_work < minWork)
                {
                    minWork = childrenData[i].m_work;
                    bestMove = children[i];
                    break;
                }
            }
            HexAssert(false);
        }
    }
    
    // Store search results
    if (!m_aborted)
        TTStore(m_brd->Hash(), DfpnData(currentBounds, childrenSet, 
                                        bestMove, localWork, parentHash,
                                        history.LastMove()));
    return localWork;
}

void SolverDFPN::SelectChild(int& bestIndex, std::size_t& delta2,
                      const std::vector<DfpnData>& childrenData) const
{
    std::size_t delta1 = INFTY;

    HexAssert(1 <= childrenData.size());
    for (std::size_t i = 0; i < childrenData.size(); ++i) 
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

void SolverDFPN::UpdateBounds(hash_t parentHash, DfpnBounds& bounds, 
                              const std::vector<DfpnData>& childData) const
{
    // Track two sets of bounds: all of our children and children that
    // have parentHash as their parent. If some children have
    // parentHash as a parent, then we use only the sum of their phis
    // as our estimate for the parent's delta. This is to reduce
    // artificially inflated delta estimates due to transpositions. To
    // turn this off, uncomment the second last line and comment out
    // the last line of this method.
    DfpnBounds boundsAll(INFTY, 0);
    DfpnBounds boundsParent(INFTY, 0);
    bool useBoundsParent = false;
    for (std::size_t i = 0; i < childData.size(); ++i)
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
        boundsParent.phi = std::min(boundsParent.phi, childBounds.delta);
        HexAssert(childBounds.phi != INFTY);
        boundsAll.delta += childBounds.phi;
        if (childData[i].m_parentHash == parentHash)
        {
            useBoundsParent = true;
            boundsParent.delta += childBounds.phi;
        }
    }
    bounds = boundsAll;
    //bounds = useBoundsParent ? boundsParent : boundsAll;
}

void SolverDFPN::LookupData(DfpnData& data, HexColor colorToMove, 
                            HexPoint cell)
{
    m_brd->playMove(colorToMove, cell);
    hash_t hash = m_brd->Hash();
    m_brd->undoMove(cell);

    if (!m_hashTable->Get(hash, data))
    {
        data.m_bounds.phi = 1;
        data.m_bounds.delta = 1;
        data.m_work = 0;
    }
}

void SolverDFPN::TTStore(hash_t hash, const DfpnData& data)
{
    CheckBounds(data.m_bounds);
    m_hashTable->Put(hash, data);
}

#ifndef NDEBUG
void SolverDFPN::CheckBounds(const DfpnBounds& bounds) const
{
    HexAssert(bounds.phi <= INFTY);
    HexAssert(bounds.delta <= INFTY);
    HexAssert(0 != bounds.phi || INFTY == bounds.delta);
    HexAssert(0 != bounds.delta || INFTY == bounds.phi);
    HexAssert(INFTY!= bounds.phi || 0 == bounds.delta ||INFTY == bounds.delta);
    HexAssert(INFTY!= bounds.delta || 0 == bounds.phi ||INFTY == bounds.phi);
}
#else
void SolverDFPN::CheckBounds(const DfpnBounds& bounds) const
{
    SG_UNUSED(bounds);
}
#endif

//----------------------------------------------------------------------------
