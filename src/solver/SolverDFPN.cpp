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

SolverDFPN::SolverDFPN()
    : m_hashTable(0),
      m_showProgress(false),
      m_progressDepth(1),
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
    m_hashTable.reset(new DfpnHashTable(m_ttsize));
    m_children.clear();
    m_terminal.clear();
    m_seen.clear();

    m_numTerminal = 0;
    m_numMIDcalls = 0;
    m_brd.reset(new StoneBoard(board));
    m_workBoard = &board;

    DfpnBounds root(INFTY, INFTY);
    SgTimer timer;
    MID(root, 0);
    timer.Stop();

    DfpnData data;
    m_hashTable->get(m_brd->Hash(), data);
    LogInfo() << "Root proof number is " << data.m_bounds.phi << "\n";
    LogInfo() << "Root disproof number is " << data.m_bounds.delta << "\n\n";

    LogInfo() << "     MID calls: " << m_numMIDcalls << "\n";
    LogInfo() << "  Unique nodes: " << m_seen.size() << "\n";
    LogInfo() << "Terminal nodes: " << m_numTerminal << "\n";
    LogInfo() << "  Elapsed Time: " << timer.GetTime() << '\n';
    LogInfo() << "      MIDs/sec: " << m_numMIDcalls/timer.GetTime() << '\n';
    LogInfo() << m_hashTable->stats() << '\n';

    std::vector<HexPoint> pv;
    GetVariation(*m_brd, pv);
    LogInfo() << "PV: " << PrintVariation(pv) << '\n';

    CheckBounds(data.m_bounds);
    if (0 == data.m_bounds.phi)
        return colorToMove;
    else
        return !colorToMove;
}

void SolverDFPN::MID(const DfpnBounds& bounds, int depth)
{
    CheckBounds(bounds);
    HexAssert(bounds.phi > 1);
    HexAssert(bounds.delta > 1);
    {
        // Check thresholds
        // FIXME: remove this!
        DfpnData data;
        if (m_hashTable->get(m_brd->Hash(), data)) 
        {
            HexAssert(bounds.phi > data.m_bounds.phi);
            HexAssert(bounds.delta > data.m_bounds.delta);
        }
    }

    // If we've never been here before, check if it's terminal.
    // Compute children and store them if not terminal.
    HexColor colorToMove = m_brd->WhoseTurn();
    if (!m_seen.count(m_brd->Hash()))
    {
        m_seen.insert(m_brd->Hash());

        m_workBoard->SetState(*m_brd);
        m_workBoard->ComputeAll(colorToMove);
        if (PlayerUtils::IsDeterminedState(*m_workBoard, colorToMove))
        {
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
            m_terminal[m_brd->Hash()] = terminal;
            TTStore(m_brd->Hash(), terminal, INVALID_POINT);
            ++m_numTerminal;

            if (m_showProgress && depth <= m_progressDepth) 
            {
                std::string spaces(2*depth, ' ');
                LogInfo() << spaces << "Terminal! " << terminal << '\n';
            }
            return;
        }

        // Store children
        m_children[m_brd->Hash()] 
            = PlayerUtils::MovesToConsider(*m_workBoard, colorToMove);
    }
    // If we have been here before and this state is marked as terminal,
    // put it back in the TT.
    else if (m_terminal.count(m_brd->Hash()))
    {
        TTStore(m_brd->Hash(), m_terminal[m_brd->Hash()], INVALID_POINT);
        return;
    }

    // We've been to this state before and it is not a terminal,
    // so look up children.
    HexAssert(m_children.count(m_brd->Hash()));
    bitset_t childrenSet = m_children[m_brd->Hash()];
    std::vector<HexPoint> children;
    BitsetUtil::BitsetToVector(childrenSet, children);

    ++m_numMIDcalls;

    // Not thread safe: perhaps move into while loop below later...
    std::vector<DfpnBounds> childrenBounds(children.size());
    for (size_t i = 0; i < children.size(); ++i)
        LookupBounds(childrenBounds[i], colorToMove, children[i]);
    
    HexPoint bestMove = INVALID_POINT;
    DfpnBounds currentBounds;
    while (true) 
    {
        UpdateBounds(currentBounds, childrenBounds);
        if (bounds.phi <= currentBounds.phi 
            || bounds.delta <= currentBounds.delta)
        {
            if (m_showProgress && depth <= m_progressDepth) 
            {
                std::string spaces(2*depth, ' ');
                LogInfo() << spaces << bounds << " -> " 
                          << currentBounds << " Bounds Exceeded!\n";
            }
            break;
        }

        // Select most proving child
        int bestIndex = -1;
        std::size_t delta2 = INFTY;
        SelectChild(bestIndex, delta2, childrenBounds);
        DfpnBounds child(childrenBounds[bestIndex]);
        bestMove = children[bestIndex];

        // Update thresholds
        child.phi += bounds.delta - currentBounds.delta;
        child.delta = std::min(bounds.phi, delta2 + 1);
        HexAssert(child.phi > childrenBounds[bestIndex].phi);
        HexAssert(child.delta > childrenBounds[bestIndex].delta);

        if (m_showProgress && depth <= m_progressDepth) 
        {
            std::string spaces(2*depth, ' ');
            LogInfo() << spaces << bounds << " -> " 
                      << currentBounds << ": " << bestMove << '\n';
        }

        // Recurse on best child
        m_brd->playMove(colorToMove, bestMove);
        MID(child, depth + 1);
        m_brd->undoMove(bestMove);

        // Update bounds for best child
        LookupBounds(childrenBounds[bestIndex], colorToMove, bestMove);
    }

    // Store search results
    TTStore(m_brd->Hash(), currentBounds, bestMove);
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

void SolverDFPN::TTStore(hash_t hash, const DfpnBounds& bounds,
                         HexPoint bestMove) 
{
    CheckBounds(bounds);
    m_hashTable->put(DfpnData(hash, bounds, bestMove));
}

void SolverDFPN::CheckBounds(const DfpnBounds& bounds) const
{
    HexAssert(bounds.phi <= INFTY);
    HexAssert(bounds.delta <= INFTY);
    HexAssert(0 != bounds.phi || INFTY == bounds.delta);
    HexAssert(0 != bounds.delta || INFTY == bounds.phi);
    HexAssert(INFTY != bounds.phi || 0 == bounds.delta ||INFTY == bounds.delta);
    HexAssert(INFTY != bounds.delta || 0 == bounds.phi ||INFTY == bounds.phi);
}

//----------------------------------------------------------------------------
