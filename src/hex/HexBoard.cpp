//----------------------------------------------------------------------------
/** @file HexBoard.cpp
 */
//----------------------------------------------------------------------------

#include "Time.hpp"
#include "BoardUtils.hpp"
#include "BitsetIterator.hpp"
#include "VCSet.hpp"
#include "HexBoard.hpp"
#include "VCUtils.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

HexBoard::HexBoard(int width, int height, const ICEngine& ice,
                   VCBuilderParam& param)
    : PatternBoard(width, height), 
      m_ice(&ice),
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
    : PatternBoard(other),
      m_ice(other.m_ice),
      m_builder(other.m_builder),
      m_history(other.m_history),
      m_inf(other.m_inf),
      m_use_vcs(other.m_use_vcs),
      m_use_ice(other.m_use_ice),
      m_use_decompositions(other.m_use_decompositions),
      m_backup_ice_info(other.m_backup_ice_info)
{
    for (BWIterator color; color; ++color)
    {
        m_cons[*color].reset(new VCSet(*other.m_cons[*color]));
        m_log[*color] = m_log[*color];
    }
}

void HexBoard::Initialize()
{
    for (BWIterator c; c; ++c) 
        m_cons[*c].reset(new VCSet(Const(), *c));
    ClearHistory();
}

HexBoard::~HexBoard()
{
}

//----------------------------------------------------------------------------

void HexBoard::SetState(const StoneBoard& brd)
{
    startNewGame();
    setColor(BLACK, brd.getBlack());
    setColor(WHITE, brd.getWhite());
    setPlayed(brd.getPlayed());
}

//----------------------------------------------------------------------------

void HexBoard::ComputeInferiorCells(HexColor color_to_move)
{
    if (m_use_ice) 
    {
        InferiorCells inf;
        m_ice->ComputeInferiorCells(color_to_move, *this, inf);
        IceUtil::Update(m_inf, inf, *this);
    }
}

void HexBoard::BuildVCs()
{
    for (BWIterator c; c; ++c)
        m_builder.Build(*m_cons[*c], *this);
}

void HexBoard::BuildVCs(bitset_t added[BLACK_AND_WHITE], bool markLog)
{
    HexAssert((added[BLACK] & added[WHITE]).none());
    for (BWIterator c; c; ++c) 
    {
        if (markLog)
            m_log[*c].push(ChangeLog<VC>::MARKER, VC());
        m_builder.Build(*m_cons[*c], *this, added, &m_log[*c]);
    }
}

void HexBoard::RevertVCs()
{
    for (BWIterator c; c; ++c)
        m_cons[*c]->Revert(m_log[*c]);
}

/** In non-terminal states, checks for combinatorial decomposition
    with a vc using BoardUtils::FindCombinatorialDecomposition(). 
    Plays the carrier using AddStones(). Loops until no more
    decompositions are found. */
void HexBoard::HandleVCDecomposition(HexColor color_to_move)
{
    if (!m_use_decompositions) 
        return;

    /** @todo Check for a vc win/loss here instead of just solid
	chains. */
    if (isGameOver()) 
        return;

    int decompositions = 0;
    for (;;) 
    {
        bool found = false;
        for (BWIterator c; c; ++c) 
        {
            bitset_t captured;
            if (BoardUtils::FindCombinatorialDecomposition(*this, *c,
							   captured))
            {
                LogFine() << "Decomposition " << decompositions << ": for " 
			  << *c << "." << '\n' 
                          << printBitset(captured) << '\n';
            
                AddStones(*c, captured, color_to_move);
                m_inf.AddCaptured(*c, captured);
            
                LogFine() << "After decomposition " << decompositions 
			  << ": " << *this << '\n';
                
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
    double s = Time::Get();
    
    update();
    absorb();
    m_inf.Clear();

    bitset_t old_black = getColor(BLACK);
    bitset_t old_white = getColor(WHITE);
    ComputeInferiorCells(color_to_move);

    bitset_t added[BLACK_AND_WHITE];
    added[BLACK] = getColor(BLACK) - old_black;
    added[WHITE] = getColor(WHITE) - old_white;
    
    if (m_use_vcs)
    {
        BuildVCs();
        HandleVCDecomposition(color_to_move);
    }

    double e = Time::Get();
    LogFine() << (e-s) << "s to compute all.\n";
}

void HexBoard::PlayMove(HexColor color, HexPoint cell)
{
    LogFine() << "Playing (" << color << ", " << cell << ")\n";

    double s = Time::Get();
    PushHistory(color, cell);
    bitset_t old_black = getColor(BLACK);
    bitset_t old_white = getColor(WHITE);

    playMove(color, cell);
    update(cell);
    absorb(cell);

    ComputeInferiorCells(!color);

    bitset_t added[BLACK_AND_WHITE];
    added[BLACK] = getColor(BLACK) - old_black;
    added[WHITE] = getColor(WHITE) - old_white;

    if (m_use_vcs)
    {
        BuildVCs(added);
        HandleVCDecomposition(!color);
    }
    double e = Time::Get();
    LogFine() << (e-s) << "s to play stones.\n";
}

void HexBoard::PlayStones(HexColor color, const bitset_t& played,
                          HexColor color_to_move)
{
    LogFine() << "Playing (" << color << ","
              << HexPointUtil::ToPointListString(played) << ")\n";
    HexAssert(BitsetUtil::IsSubsetOf(played, getEmpty()));

    double s = Time::Get();
    PushHistory(color, INVALID_POINT);
    bitset_t old_black = getColor(BLACK);
    bitset_t old_white = getColor(WHITE);

    addColor(color, played);
    update(played);
    absorb(played);

    ComputeInferiorCells(color_to_move);

    bitset_t added[BLACK_AND_WHITE];
    added[BLACK] = getColor(BLACK) - old_black;
    added[WHITE] = getColor(WHITE) - old_white;

    if (m_use_vcs)
    {
        BuildVCs(added);
        HandleVCDecomposition(color_to_move);
    }

    double e = Time::Get();
    LogFine() << (e-s) << "s to play stones.\n";
}

void HexBoard::AddStones(HexColor color, const bitset_t& played,
                         HexColor color_to_move)
{
    HexAssert(BitsetUtil::IsSubsetOf(played, getEmpty()));
    LogFine() << "Adding (" << color << ", "
              << HexPointUtil::ToPointListString(played) << ")\n";

    double s = Time::Get();
    bitset_t old_black = getColor(BLACK);
    bitset_t old_white = getColor(WHITE);

    addColor(color, played);
    update(played);
    absorb(played);

    ComputeInferiorCells(color_to_move);

    bitset_t added[BLACK_AND_WHITE];
    added[BLACK] = getColor(BLACK) - old_black;
    added[WHITE] = getColor(WHITE) - old_white;

    if (m_use_vcs)
        BuildVCs(added, false); 

    double e = Time::Get();
    LogFine() << (e-s) << "s to add stones.\n";
}

void HexBoard::UndoMove()
{
    double s = Time::Get();

    PopHistory();
    update();
    absorb();

    double e = Time::Get();
    LogFine() << (e-s) << "s to undo move.\n";
}

//----------------------------------------------------------------------------

void HexBoard::ClearHistory()
{
    m_history.clear();
}

void HexBoard::PushHistory(HexColor color, HexPoint cell)
{
    m_history.push_back(History(*this, m_inf, color, cell));
}

/** Restores the old board position, backs up ice info, and reverts
    virtual connections. */
void HexBoard::PopHistory()
{
    HexAssert(!m_history.empty());

    History hist = m_history.back();
    m_history.pop_back();

    SetState(hist.board);
    if (m_backup_ice_info && hist.last_played != INVALID_POINT)
    {
        // Cells that were not marked as inferior in parent state
        // and are either dead or captured (for the color to play in the
        // parent state) are marked as dominated. 
        bitset_t a = getEmpty() - hist.inf.All();
        a &= m_inf.Dead() | m_inf.Captured(hist.to_play);

        for (BitsetIterator p(a); p; ++p) 
            hist.inf.AddDominated(*p, hist.last_played);
    }
    m_inf = hist.inf;
    RevertVCs();
}

//----------------------------------------------------------------------------
