//----------------------------------------------------------------------------
/** @file SolverDFPN.cpp
 */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "SgTimer.h"

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
                                    const std::vector<DfpnBounds>& bounds)
{
    m_children = children;
    m_bounds = bounds;
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
            m_bounds[i] = bounds;
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
        if (0 == m_bounds[i].phi)
        {
            numLosses++;
            os << " L";
        }
        else if (0 == m_bounds[i].delta)
            os << " W";
        else
            os << ' ' << m_bounds[i].phi 
               << ':' << m_bounds[i].delta;
    }
    os << '\n';
    os << "TEXT ";
    os << numLosses << '/' << m_children.size() << " proven losses\n";
    os << '\n';
    std::cout << os.str();
    std::cout.flush();
}

//----------------------------------------------------------------------------

SolverDFPN::SolverDFPN()
    : m_hashTable(0),
      m_guiFx(),
      m_useGuiFx(false),
      m_ttsize(20)
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
        if (!m_hashTable->get(brd.Hash(), data))
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

HexColor SolverDFPN::StartSearch(HexColor colorToMove, HexBoard& board)
{
    m_aborted = false;
    m_hashTable.reset(new DfpnHashTable(m_ttsize));
    m_numTerminal = 0;
    m_numMIDcalls = 0;
    m_brd.reset(new StoneBoard(board));
    m_workBoard = &board;

    DfpnBounds root(INFTY, INFTY);
    SgTimer timer;
    MID(root, 0);
    timer.Stop();

    LogInfo() << "     MID calls: " << m_numMIDcalls << "\n";
    LogInfo() << "Terminal nodes: " << m_numTerminal << "\n";
    LogInfo() << "  Elapsed Time: " << timer.GetTime() << '\n';
    LogInfo() << "      MIDs/sec: " << m_numMIDcalls/timer.GetTime() << '\n';
    LogInfo() << m_hashTable->stats() << '\n';

    if (!m_aborted)
    {
        DfpnData data;
        m_hashTable->get(m_brd->Hash(), data);
        CheckBounds(data.m_bounds);

        HexColor winner 
            = (0 == data.m_bounds.phi) ? colorToMove : !colorToMove;
        LogInfo() << winner << " wins!\n";

        std::vector<HexPoint> pv;
        GetVariation(*m_brd, pv);
        LogInfo() << "PV: " << PrintVariation(pv) << '\n';

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
#if 0
        else if ((m_settings.time_limit > 0) && 
                 ((Time::Get() - m_start_time) > m_settings.time_limit))
        {
            m_aborted = true;
            LogInfo() << "SolverDFPN::CheckAbort(): Timelimit!" << '\n';
        }
#endif
    }
    return m_aborted;
}

void SolverDFPN::MID(const DfpnBounds& bounds, int depth)
{
    CheckBounds(bounds);
    HexAssert(bounds.phi > 1);
    HexAssert(bounds.delta > 1);

    if (CheckAbort())
        return;

    bitset_t childrenSet;
    HexColor colorToMove = m_brd->WhoseTurn();

    DfpnData data;
    if (m_hashTable->get(m_brd->Hash(), data)) 
    {
        childrenSet = data.m_children;
        HexAssert(bounds.phi > data.m_bounds.phi);
        HexAssert(bounds.delta > data.m_bounds.delta);
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
            {
                terminal.phi = 0;
                terminal.delta = INFTY;
            } 
            else 
            {
                terminal.phi = INFTY;
                terminal.delta = 0;
            }
            if (m_useGuiFx && depth == 1)
            {
                m_guiFx.UpdateCurrentBounds(terminal);
                m_guiFx.Write();
            }
            TTStore(DfpnData(m_brd->Hash(), terminal, 
                             EMPTY_BITSET, INVALID_POINT));
            return;
        }
        childrenSet = PlayerUtils::MovesToConsider(*m_workBoard, colorToMove);
    }

    ++m_numMIDcalls;

    HexAssert(childrenSet.any());
    std::vector<HexPoint> children;
    BitsetUtil::BitsetToVector(childrenSet, children);

    // Not thread safe: perhaps move into while loop below later...
    std::vector<DfpnBounds> childrenBounds(children.size());
    for (size_t i = 0; i < children.size(); ++i)
        LookupBounds(childrenBounds[i], colorToMove, children[i]);
    if (m_useGuiFx && depth == 0)
        m_guiFx.SetChildren(children, childrenBounds);
   
    HexPoint bestMove = INVALID_POINT;
    DfpnBounds currentBounds;
    while (!m_aborted) 
    {
        UpdateBounds(currentBounds, childrenBounds);
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
        SelectChild(bestIndex, delta2, childrenBounds);
        DfpnBounds child(childrenBounds[bestIndex]);
        bestMove = children[bestIndex];

        // Update thresholds
        child.phi = bounds.delta - (currentBounds.delta - child.phi);
        child.delta = std::min(bounds.phi, delta2 + 1);
        HexAssert(child.phi > childrenBounds[bestIndex].phi);
        HexAssert(child.delta > childrenBounds[bestIndex].delta);

        if (m_useGuiFx && depth == 0)
            m_guiFx.PlayMove(colorToMove, bestMove);

        // Recurse on best child
        m_brd->playMove(colorToMove, bestMove);
        MID(child, depth + 1);
        m_brd->undoMove(bestMove);

        if (m_useGuiFx && depth == 0)
            m_guiFx.UndoMove();

        // Update bounds for best child
        LookupBounds(childrenBounds[bestIndex], colorToMove, bestMove);
    }

    if (m_useGuiFx && depth == 0)
        m_guiFx.WriteForced();
    
    // Store search results
    if (!m_aborted)
        TTStore(DfpnData(m_brd->Hash(), currentBounds, childrenSet, bestMove));
}

void SolverDFPN::SelectChild(int& bestIndex, std::size_t& delta2,
                      const std::vector<DfpnBounds>& childrenBounds) const
{
    std::size_t delta1 = INFTY;

    HexAssert(1 <= childrenBounds.size());
    for (std::size_t i = 0; i < childrenBounds.size(); ++i) 
    {
        const DfpnBounds& child = childrenBounds[i];
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
        if (0 == child.delta)
            break;
    }
    HexAssert(delta1 < INFTY);
}

void SolverDFPN::UpdateBounds(DfpnBounds& bounds, 
                              const std::vector<DfpnBounds>& childBounds) const
{
    bounds.phi = INFTY;
    bounds.delta = 0;
    for (std::size_t i = 0; i < childBounds.size(); ++i)
    {
        // Abort on losing child (a winning move)
        if (0 == childBounds[i].delta)
        {
            HexAssert(INFTY == childBounds[i].phi);
            bounds.phi = 0;
            bounds.delta = INFTY;
            break;
        }
        bounds.phi = std::min(bounds.phi, childBounds[i].delta);
        HexAssert(childBounds[i].phi != INFTY);
        bounds.delta += childBounds[i].phi;
    }
}

void SolverDFPN::LookupBounds(DfpnBounds& bounds, HexColor colorToMove, 
                              HexPoint cell)
{
    m_brd->playMove(colorToMove, cell);
    hash_t hash = m_brd->Hash();
    m_brd->undoMove(cell);

    DfpnData data;
    if (m_hashTable->get(hash, data)) 
        bounds = data.m_bounds;
    else 
    {
        bounds.phi = 1;
        bounds.delta = 1;
    }
}

void SolverDFPN::TTStore(const DfpnData& data)
{
    CheckBounds(data.m_bounds);
    m_hashTable->put(data);
}

void SolverDFPN::CheckBounds(const DfpnBounds& bounds) const
{
    HexAssert(bounds.phi <= INFTY);
    HexAssert(bounds.delta <= INFTY);
    HexAssert(0 != bounds.phi || INFTY == bounds.delta);
    HexAssert(0 != bounds.delta || INFTY == bounds.phi);
    HexAssert(INFTY!= bounds.phi || 0 == bounds.delta ||INFTY == bounds.delta);
    HexAssert(INFTY!= bounds.delta || 0 == bounds.phi ||INFTY == bounds.phi);
}

//----------------------------------------------------------------------------
