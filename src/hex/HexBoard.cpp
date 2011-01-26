//----------------------------------------------------------------------------
/** @file HexBoard.cpp */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "SgTimer.h"

#include "Decompositions.hpp"
#include "BitsetIterator.hpp"
#include "Groups.hpp"
#include "VCSet.hpp"
#include "HexBoard.hpp"
#include "VCUtil.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

HexBoard::HexBoard(int width, int height, const ICEngine& ice,
                   VCBuilderParam& param)
    : m_brd(width, height), 
      m_ice(&ice),
      m_groups(),
      m_patterns(m_brd),
      m_builder(param),
      m_use_vcs(true),
      m_use_ice(true),
      m_use_decompositions(true),
      m_backup_ice_info(true)
{
    Initialize();
}

/** @warning This is not very maintainable! How to make this
    copy-constructable nicely, even though it has a scoped_ptr? */
HexBoard::HexBoard(const HexBoard& other)
    : m_brd(other.m_brd),
      m_ice(other.m_ice),
      m_groups(other.m_groups),
      m_patterns(m_brd),
      m_builder(other.m_builder),
      m_history(other.m_history),
      m_inf(other.m_inf),
      m_use_vcs(other.m_use_vcs),
      m_use_ice(other.m_use_ice),
      m_use_decompositions(other.m_use_decompositions),
      m_backup_ice_info(other.m_backup_ice_info)
{
    m_patterns.CopyState(other.GetPatternState());
    for (BWIterator color; color; ++color)
    {
        m_cons[*color].reset(new VCSet(*other.m_cons[*color]));
        m_log[*color] = m_log[*color];
    }
}

void HexBoard::Initialize()
{
    GroupBuilder::Build(m_brd, m_groups);
    for (BWIterator c; c; ++c) 
        m_cons[*c].reset(new VCSet(Const(), *c));
    ClearHistory();
}

HexBoard::~HexBoard()
{
}

//----------------------------------------------------------------------------

void HexBoard::ComputeInferiorCells(HexColor color_to_move)
{
    if (m_use_ice) 
    {
        InferiorCells inf;
        m_ice->ComputeInferiorCells(color_to_move, m_groups, m_patterns, inf);
        IceUtil::Update(m_inf, inf);
    }
}

void HexBoard::BuildVCs()
{
    for (BWIterator c; c; ++c)
        m_builder.Build(*m_cons[*c], m_groups, m_patterns);
}

void HexBoard::BuildVCs(const Groups& oldGroups, 
                        bitset_t added[BLACK_AND_WHITE], bool use_changelog)
{
    BenzeneAssert((added[BLACK] & added[WHITE]).none());
    for (BWIterator c; c; ++c)
    {
        ChangeLog<VC>* log = (use_changelog) ? &m_log[*c] : 0;
        m_builder.Build(*m_cons[*c], oldGroups, m_groups, m_patterns, 
                        added, log);
    }
}

void HexBoard::MarkChangeLog()
{
    m_log[BLACK].Push(ChangeLog<VC>::MARKER, VC());
    m_log[WHITE].Push(ChangeLog<VC>::MARKER, VC());
}

void HexBoard::RevertVCs()
{
    for (BWIterator c; c; ++c)
        m_cons[*c]->Revert(m_log[*c]);
}

/** In non-terminal states, checks for combinatorial decomposition
    with a vc using Decompositions::Find(). Plays the carrier using
    AddStones(). Loops until no more decompositions are found. */
void HexBoard::HandleVCDecomposition(HexColor color_to_move, bool use_changelog)
{
    if (!m_use_decompositions) 
        return;

    /** @todo Check for a vc win/loss here instead of just solid
	chains. */
    if (m_groups.IsGameOver()) 
        return;

    int decompositions = 0;
    for (;;) 
    {
        bool found = false;
        for (BWIterator c; c; ++c) 
        {
            bitset_t captured;
            if (Decompositions::Find(*this, *c, captured))
            {
                LogFine() << "Decomposition " << decompositions << ": for " 
			  << *c << ".\n" << m_brd.Write(captured) << '\n';
            
                AddStones(*c, captured, color_to_move, use_changelog);
                m_inf.AddCaptured(*c, captured);
            
                LogFine() << "After decomposition " << decompositions 
			  << ": " << m_brd << '\n';
                
                decompositions++;
                found = true;
                break;
            } 
        }
        if (!found) 
            break;
    }
    LogFine() << "Found " << decompositions << " decompositions.\n";
}

void HexBoard::ComputeAll(HexColor color_to_move)
{
    SgTimer timer;

    m_patterns.Update();
    GroupBuilder::Build(m_brd, m_groups);
    m_inf.Clear();

    ComputeInferiorCells(color_to_move);

    if (m_use_vcs)
    {
        m_builder.ClearStatistics();
        BuildVCs();
        HandleVCDecomposition(color_to_move, false);
    }
    LogFine() << timer.GetTime() << "s to compute all.\n";
}

void HexBoard::PlayMove(HexColor color, HexPoint cell)
{
    LogFine() << "Playing (" << color << ", " << cell << ")\n";

    SgTimer timer;
    PushHistory(color, cell);
    bitset_t old_black = m_brd.GetColor(BLACK);
    bitset_t old_white = m_brd.GetColor(WHITE);

    m_brd.PlayMove(color, cell);
    m_patterns.Update(cell);
    Groups oldGroups(m_groups);
    GroupBuilder::Build(m_brd, m_groups);

    ComputeInferiorCells(!color);

    bitset_t added[BLACK_AND_WHITE];
    added[BLACK] = m_brd.GetColor(BLACK) - old_black;
    added[WHITE] = m_brd.GetColor(WHITE) - old_white;

    if (m_use_vcs)
    {
        m_builder.ClearStatistics();
        MarkChangeLog();
        BuildVCs(oldGroups, added, true);
        HandleVCDecomposition(!color, true);
    }
    LogFine() << timer.GetTime() << "s to play stones.\n";
}

void HexBoard::PlayStones(HexColor color, const bitset_t& played,
                          HexColor color_to_move)
{
    LogFine() << "Playing (" << color << ","
              << HexPointUtil::ToString(played) << ")\n";
    BenzeneAssert(BitsetUtil::IsSubsetOf(played, GetPosition().GetEmpty()));

    SgTimer timer;
    PushHistory(color, INVALID_POINT);
    bitset_t old_black = m_brd.GetColor(BLACK);
    bitset_t old_white = m_brd.GetColor(WHITE);

    m_brd.AddColor(color, played);
    m_patterns.Update(played);
    Groups oldGroups(m_groups);
    GroupBuilder::Build(m_brd, m_groups);

    ComputeInferiorCells(color_to_move);

    bitset_t added[BLACK_AND_WHITE];
    added[BLACK] = m_brd.GetColor(BLACK) - old_black;
    added[WHITE] = m_brd.GetColor(WHITE) - old_white;

    if (m_use_vcs)
    {
        m_builder.ClearStatistics();
        MarkChangeLog();
        BuildVCs(oldGroups, added, true);
        HandleVCDecomposition(color_to_move, true);
    }

    LogFine() << timer.GetTime() << "s to play stones.\n";
}

/** Adds stones for color to board with color_to_move about to
    play next; added stones must be a subset of the empty cells.
    Does not affect the hash of this state. State is not pushed
    onto stack, so a call to UndoMove() will undo these changes
    along with the last changes that changed the stack. */
void HexBoard::AddStones(HexColor color, const bitset_t& played,
                         HexColor color_to_move, bool use_changelog)
{
    BenzeneAssert(BitsetUtil::IsSubsetOf(played, GetPosition().GetEmpty()));
    LogFine() << "Adding (" << color << ", "
              << HexPointUtil::ToString(played) << ")\n";

    SgTimer timer;
    bitset_t old_black = m_brd.GetColor(BLACK);
    bitset_t old_white = m_brd.GetColor(WHITE);

    m_brd.AddColor(color, played);
    m_patterns.Update(played);
    Groups oldGroups(m_groups);
    GroupBuilder::Build(m_brd, m_groups);

    ComputeInferiorCells(color_to_move);

    bitset_t added[BLACK_AND_WHITE];
    added[BLACK] = m_brd.GetColor(BLACK) - old_black;
    added[WHITE] = m_brd.GetColor(WHITE) - old_white;

    if (m_use_vcs)
        BuildVCs(oldGroups, added, use_changelog); 

    LogFine() << timer.GetTime() << "s to add stones.\n";
}

void HexBoard::UndoMove()
{
    SgTimer timer;
    PopHistory();
    m_patterns.Update();
    LogFine() << timer.GetTime() << "s to undo move.\n";
}

//----------------------------------------------------------------------------

void HexBoard::ClearHistory()
{
    m_history.clear();
}

void HexBoard::PushHistory(HexColor color, HexPoint cell)
{
    m_history.push_back(History(m_brd, m_groups, m_inf, color, cell));
}

/** Restores the old board position, backs up ice info, and reverts
    virtual connections. */
void HexBoard::PopHistory()
{
    BenzeneAssert(!m_history.empty());

    History hist = m_history.back();
    m_history.pop_back();

    m_brd.SetPosition(hist.board);
    if (m_backup_ice_info && hist.last_played != INVALID_POINT)
    {
        // Cells that were not marked as inferior in parent state
        // and are either dead or captured (for the color to play in the
        // parent state) are marked as dominated. 
        bitset_t a = m_brd.GetEmpty() - hist.inf.All();
        a &= m_inf.Dead() | m_inf.Captured(hist.to_play);

        for (BitsetIterator p(a); p; ++p) 
            hist.inf.AddDominated(*p, hist.last_played);
    }
    m_inf = hist.inf;
    m_groups = hist.groups;
    RevertVCs();
}

//----------------------------------------------------------------------------
