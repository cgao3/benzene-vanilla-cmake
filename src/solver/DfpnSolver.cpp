//----------------------------------------------------------------------------
/** @file DfpnSolver.cpp
 */
//----------------------------------------------------------------------------

#include "BitsetIterator.hpp"
#include "DfpnSolver.hpp"
#include "PlayerUtils.hpp"
#include "Resistance.hpp"

using namespace benzene;

#define DEBUG_BOUNDS_CORRECTION 0

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
    : m_move(INVALID_POINT),
      m_timeOfLastWrite(0.0),
      m_delay(1.0)
{
}

void DfpnSolver::GuiFx::SetChildren(const std::vector<HexPoint>& children,
                                    const std::vector<DfpnData>& data)
{
    m_children = children;
    m_data = data;
}

void DfpnSolver::GuiFx::PlayMove(HexColor color, HexPoint move) 
{
    m_color = color;
    m_move = move;
}

void DfpnSolver::GuiFx::UndoMove()
{
    m_move = INVALID_POINT;
}

void DfpnSolver::GuiFx::UpdateCurrentBounds(const DfpnBounds& bounds)
{
    HexAssert(m_move != INVALID_POINT);
    for (std::size_t i = 0; i < m_children.size(); ++i)
        if (m_children[i] == m_move)
            m_data[i].m_bounds = bounds;
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
void DfpnSolver::GuiFx::DoWrite()
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

/** @page boundscorrection Dfpn Bounds Correction
    @ingroup dfpn

    Transpositions in the graph of visited states can cause the bounds
    to be artificially inflated. We try to alleviate this by noting a
    fixed number of transpositions per state and checking wether each
    would actually affect the bound.

    This is not that effective in Hex. Transpositions are rare (less
    than 1% of states have one) and of those, only a small percentage
    result in actual bounds inflation.

    This technique may be useful in other domains where transpositions
    are more common and more likely to inflate the bounds.
*/

DfpnTransposition::DfpnTransposition()
    : m_hash(0)
{
}

DfpnTransposition::DfpnTransposition(hash_t hash)
    : m_hash(hash)
{
}

void DfpnTranspositions::Add(hash_t hash, size_t length, 
                             HexPoint* rightMove, hash_t* rightHashes, 
                             HexPoint* leftMove, hash_t* leftHashes)
{
    if (m_slot.size() >= NUM_SLOTS)
        return;
    for (size_t i = 0; i < m_slot.size(); ++i)
        if (m_slot[i].m_hash == hash)
            return;
    m_slot.push_back(DfpnTransposition(hash));
    while (length--)
    {
        m_slot.back().m_rightMove.push_back(*rightMove++);
        m_slot.back().m_rightHash.push_back(*rightHashes++);
        m_slot.back().m_leftMove.push_back(*leftMove++);
        m_slot.back().m_leftHash.push_back(*leftHashes++);
    }
}

/** Attempts to modify the bounds for the current state by removing
    duplicate estimates created by each transposition. */
size_t DfpnTranspositions::ModifyBounds(hash_t currentHash, 
                                        DfpnBounds& bounds, 
                                        DfpnHashTable& hashTable,
                                        DfpnStatistics& slotStats) const
{
    if (m_slot.empty())
        return 0;
    slotStats.Add(m_slot.size());
    size_t ret = 0;
    for (std::size_t i = 0; i < m_slot.size(); ++i)
        if (m_slot[i].ModifyBounds(currentHash, bounds, hashTable))
            ++ret;
    return ret;
}

/** Checks that the sequence of moves matches those in the hashtable
    on every second level (the level where the bound is propagated via
    a min operation). The 'bestmove' stored in DfpnData corresponds to
    the child with min delta, and so much match the move at this
    location in the transposition. */
bool DfpnTransposition::IsMinPath(DfpnHashTable& hashTable, 
                                  const std::vector<HexPoint>& move,
                                  const std::vector<hash_t>& hash) const
{
    bool minMove = true;
    // Skip the check for position 0:
    //  1) not a min check anyway
    //  2) state may not be in table if first time state was visited
    for (std::size_t i = 1; i < hash.size(); ++i)
    {
#if DEBUG_BOUNDS_CORRECTION
        LogInfo() << i << ": " << HashUtil::toString(hash[i]) 
                  << ' ' << move[i] << '\n';
#endif
        DfpnData data;
        if (!hashTable.Get(hash[i], data))
            return false;
        if (minMove && data.m_bestMove != move[i])
        {
#if DEBUG_BOUNDS_CORRECTION
            LogInfo() << "Not min path!\n";
#endif
            return false;
        }
        minMove = !minMove;
    }
    return true;
}

/** If both paths from ancestor to descendant contribute to the
    ancestor's delta, reduces delta by descendant's phi or delta
    (which to use is dependant on length of path).
    We can only modify delta because phi is computed as the min of
    child deltas, hence the descendant's bound is counted at most
    once. Delta, however, is the sum of child phis and so the
    descendant's bound could be counted twice.
*/
bool DfpnTransposition::ModifyBounds(hash_t currentHash, DfpnBounds& bounds,
                                     DfpnHashTable& hashTable) const
{
#ifdef NDEBUG
    SG_UNUSED(currentHash);
#else
    HexAssert(currentHash == m_rightHash[0]);
    HexAssert(currentHash == m_leftHash[0]);
#endif
#if DEBUG_BOUNDS_CORRECTION
    LogInfo() << "ModifyBounds():\n";
    LogInfo() << "bounds: " << bounds << '\n';
    LogInfo() << "m_hash: " << HashUtil::toString(m_hash) << '\n';
    LogInfo() << "length: " << m_rightHash.size() << '\n';
#endif
    DfpnData descendant;   
    if (!hashTable.Get(m_hash, descendant))
        return false;
#if DEBUG_BOUNDS_CORRECTION
    LogInfo() << "Descendant bounds: " << descendant.m_bounds << '\n';
#endif
    if (descendant.m_bounds.IsSolved())
        return false;
    if (!IsMinPath(hashTable, m_rightMove, m_rightHash))
        return false;
    if (!IsMinPath(hashTable, m_leftMove, m_leftHash))
        return false;
    if (m_rightHash.size() & 1)
        bounds.delta -= descendant.m_bounds.phi;
    else
        bounds.delta -= descendant.m_bounds.delta;
#if DEBUG_BOUNDS_CORRECTION
    LogInfo() << "Corrected bounds: " << bounds << '\n';
#endif
    return true;
}

/** Traverses hashtable to find the common ancestor of the current
    state's two parents. Adds a DfpnTransposition at the level of the
    ancestor. */
void DfpnHistory::NotifyCommonAncestor(DfpnHashTable& hashTable, 
                                       DfpnData data, hash_t hash,
                                       DfpnStatistics& stats)
{
    std::size_t depth = m_hash.size();

#if DEBUG_BOUNDS_CORRECTION
    LogInfo() << "History:\n";
    for (std::size_t i = 0; i < depth; ++i)
        LogInfo() << i << ": " << HashUtil::toString(m_hash[i]) 
                  << ' ' << m_move[i] << '\n';
    LogInfo() << "x: " << HashUtil::toString(hash) << '\n';
#endif

    std::vector<hash_t> leftHash(depth, 0);
    std::vector<HexPoint> leftMove(depth, INVALID_POINT);
    for (std::size_t i = depth - 1; i > 0; --i)
    {
        leftHash[i] = data.m_parentHash;
        leftMove[i] = data.m_moveParentPlayed;
        if (!hashTable.Get(data.m_parentHash, data))
            break;
    }

#if DEBUG_BOUNDS_CORRECTION
    LogInfo() << "Table History:\n";
    for (std::size_t i = 0; i < depth; ++i)
        LogInfo() << i << ": " << HashUtil::toString(leftHash[i]) 
                  << ' ' << leftMove[i] << '\n';
    LogInfo() << "x: " << HashUtil::toString(hash) << '\n';
#endif

    std::size_t length = 1;
    for (std::size_t i = m_hash.size() - 1; i > 0; --i, ++length)
    {
        if (length > DfpnTransposition::MAX_LENGTH)
            break;
        if (std::find(m_hash.begin(), m_hash.end(), leftHash[i])
            != m_hash.end())
        {
#if DEBUG_BOUNDS_CORRECTION
            LogInfo() << "Found command ancestor at i = " << i 
                      << " (length " << length << ")\n";
#endif
            stats.Add(length);
            m_transposition[i].Add(hash, length, &m_move[i], &m_hash[i], 
                                   &leftMove[i], &leftHash[i]);
            break;
        }
    }
}

//----------------------------------------------------------------------------

DfpnSolver::DfpnSolver()
    : m_hashTable(0),
      m_useGuiFx(false),
      m_useBoundsCorrection(true),
      m_timelimit(0.0),
      m_guiFx()
{
}

DfpnSolver::~DfpnSolver()
{
}

void DfpnSolver::GetVariation(const StoneBoard& state, 
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

HexColor DfpnSolver::StartSearch(HexBoard& board, DfpnHashTable& hashtable)
{
    m_aborted = false;
    m_hashTable = &hashtable;
    m_numTerminal = 0;
    m_numTranspositions = 0;
    m_numBoundsCorrections = 0;
    m_transStats.Clear();
    m_slotStats.Clear();
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
    os << "     Length: ";
    m_transStats.Write(os);
    LogInfo() << os.str() << '\n';
    os.str("");
    os << "      Slots: ";
    m_slotStats.Write(os);
    LogInfo() << os.str() << '\n';
    LogInfo() << "Corrections: " << m_numBoundsCorrections << '\n';
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
                    LogInfo() << "DfpnSolver::CheckAbort(): Timelimit!" << '\n';
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
    hash_t parentHash = history.LastHash();
    HexColor colorToMove = m_brd->WhoseTurn();

    std::vector<HexPoint> children;
    {
        DfpnData data;
        if (m_hashTable->Get(m_brd->Hash(), data)) 
        {
            children = data.m_children;
            HexAssert(bounds.phi > data.m_bounds.phi);
            HexAssert(bounds.delta > data.m_bounds.delta);

            // Check if our stored parent differs from our current parent
            if (data.m_parentHash != parentHash)
            {
                ++m_numTranspositions;
                if (m_useBoundsCorrection)
                    history.NotifyCommonAncestor(*m_hashTable, data, 
                                                 m_brd->Hash(), m_transStats);
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
                        DfpnData(terminal, std::vector<HexPoint>(), 
                                 INVALID_POINT, 1, parentHash, 
                                 history.LastMove()));
                return 1;
            }
            bitset_t childrenBitset 
                = PlayerUtils::MovesToConsider(*m_workBoard, colorToMove);
          
            Resistance resist;
            resist.Evaluate(*m_workBoard);
            std::vector<std::pair<HexEval, HexPoint> > mvsc;
            for (BitsetIterator it(childrenBitset); it; ++it) 
            {
                HexEval score = resist.Score(*it);
                mvsc.push_back(std::make_pair(-score, *it));
            }
            stable_sort(mvsc.begin(), mvsc.end());
            for (size_t i = 0; i < mvsc.size(); ++i) 
                children.push_back(mvsc[i].second);
        }
    }

    ++m_numMIDcalls;
    size_t localWork = 1;

    // Not thread safe: perhaps move into while loop below later...
    std::vector<DfpnData> childrenData(children.size());
    for (size_t i = 0; i < children.size(); ++i)
        LookupData(childrenData[i], colorToMove, children[i]);
    if (m_useGuiFx && depth == 0)
        m_guiFx.SetChildren(children, childrenData);

    hash_t currentHash = m_brd->Hash();   
    HexPoint bestMove = INVALID_POINT;
    DfpnBounds currentBounds;
    DfpnTranspositions transpositions;
    while (!m_aborted) 
    {
        UpdateBounds(currentHash, currentBounds, childrenData);
        
        if (m_useBoundsCorrection)
            m_numBoundsCorrections 
                += transpositions.ModifyBounds(currentHash, currentBounds, 
                                               *m_hashTable, m_slotStats);
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
        history.Push(bestMove, currentHash);
        localWork += MID(child, history);
        transpositions = history.Transpositions();
        history.Pop();
        m_brd->undoMove(bestMove);

        // Update bounds for best child
        LookupData(childrenData[bestIndex], colorToMove, bestMove);

        if (m_useGuiFx && depth == 0)
            m_guiFx.UndoMove();
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
                }
            }
            HexAssert(false);
        }
    }
    
    // Store search results
    if (!m_aborted)
        TTStore(m_brd->Hash(), DfpnData(currentBounds, children, 
                                        bestMove, localWork, parentHash,
                                        history.LastMove()));
    return localWork;
}

void DfpnSolver::SelectChild(int& bestIndex, std::size_t& delta2,
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

void DfpnSolver::UpdateBounds(hash_t parentHash, DfpnBounds& bounds, 
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

void DfpnSolver::LookupData(DfpnData& data, HexColor colorToMove, 
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

void DfpnSolver::TTStore(hash_t hash, const DfpnData& data)
{
    CheckBounds(data.m_bounds);
    m_hashTable->Put(hash, data);
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
