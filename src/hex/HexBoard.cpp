//----------------------------------------------------------------------------
/** @file
 */
//----------------------------------------------------------------------------

#include "Time.hpp"
#include "BoardUtils.hpp"
#include "BitsetIterator.hpp"
#include "Connections.hpp"
#include "HexBoard.hpp"
#include "VCUtils.hpp"

//----------------------------------------------------------------------------

HexBoard::HexBoard(int width, int height, const ICEngine& ice,
                   ConnectionBuilderParam& param)
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
      m_backedup(other.m_backedup),
      m_use_vcs(other.m_use_vcs),
      m_use_ice(other.m_use_ice),
      m_use_decompositions(other.m_use_decompositions),
      m_backup_ice_info(other.m_backup_ice_info)
{
    for (BWIterator color; color; ++color)
    {
        m_cons[*color].reset(new Connections(*other.m_cons[*color]));
        m_log[*color] = m_log[*color];
    }
}

void HexBoard::Initialize()
{
    LogFine() << "--- HexBoard" << '\n'
	      << "sizeof(HexBoard) = " << sizeof(HexBoard) << '\n';

    for (BWIterator c; c; ++c) 
        m_cons[*c].reset(new Connections(Const(), *c));

    ClearHistory();
}

HexBoard::~HexBoard()
{
}

//----------------------------------------------------------------------------

bitset_t HexBoard::getMustplay(HexColor color) const
{
    bitset_t empty;
    HexColor other = !color;
    HexPoint edge1 = HexPointUtil::colorEdge1(other);
    HexPoint edge2 = HexPointUtil::colorEdge2(other);

    if (Cons(other).Exists(edge1, edge2, VC::FULL)) {
        return empty;
    }

    const VCList& semi = Cons(other).GetList(VC::SEMI, edge1, edge2);
    bitset_t intersection = semi.hardIntersection();
    intersection &= getEmpty(); // FIXME: need this line?

    return intersection;
}

void HexBoard::SetState(const StoneBoard& brd)
{
    startNewGame();
    setColor(BLACK, brd.getBlack());
    setColor(WHITE, brd.getWhite());
    setPlayed(brd.getPlayed());
}

//----------------------------------------------------------------------------

void HexBoard::ComputeInferiorCells(HexColor color_to_move, 
                                    EndgameFillin endgame_mode)
{
    if (m_use_ice) 
    {
        InferiorCells inf;
        m_ice->ComputeInferiorCells(color_to_move, *this, inf);

        if (endgame_mode == REMOVE_WINNING_FILLIN && isGameOver()) {
            HexColor winner = getWinner();
            HexAssert(winner != EMPTY);
        
            LogFine() 
                     << "Captured cells caused win! Removing..." 
                     << '\n';

            // Need to remove winner's captured, since may cause game
            // to be over, resulting in a finished game with no
            // available moves.  Likewise, after removing the captured
            // the dead may also provide a win, so we need to remove
            // them as well. This is because the dead are a function
            // of the captured.
            removeColor(winner, inf.Captured(winner));
            removeColor(winner, inf.PermInf(winner));
            removeColor(DEAD_COLOR, inf.Dead());

            inf.ClearPermInf(winner);
            inf.ClearCaptured(winner);
            inf.ClearDead();
            
            update();
            absorb();
            
            HexAssert(!isGameOver());
        } 

        // update the set of inferior cells with this new data
        IceUtil::Update(m_inf, inf, *this);

        // clear the set of backed-up domination arcs
        m_backedup.clear();
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
    for (BWIterator c; c; ++c) {
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

void HexBoard::HandleVCDecomposition(HexColor color_to_move, 
                                     EndgameFillin endgame_mode)
{
    if (!m_use_decompositions) {
        LogFine() << "Skipping decomp check." << '\n';
        return;
    }

    /** @todo check for a vc win/loss here instead of 
	just solid chains. */
    if (isGameOver()) {
        LogFine() << "Game is over; skipping decomp check." << '\n';
        return;
    }

    int decompositions = 0;
    for (;;) {
        bool found = false;
        for (BWIterator c; c; ++c) {
            bitset_t captured;
            if (BoardUtils::FindCombinatorialDecomposition(*this, *c,
							   captured))
            {
                LogFine() << "Decomposition " << decompositions << ": for " 
			  << *c << "." << '\n'
			  << printBitset(captured) << '\n';
            
                AddStones(*c, captured, color_to_move, endgame_mode);
                m_inf.AddCaptured(*c, captured);
            
                LogFine() << "After decomposition " << decompositions 
			  << ": " << *this << '\n';
                
                decompositions++;
                found = true;
                break;
            } 
        }
        if (!found) break;
    }

    LogFine() << "Found " << decompositions << " decompositions." << '\n';
}

void HexBoard::ComputeAll(HexColor color_to_move, 
                          EndgameFillin endgame_mode)
{
    double s = Time::Get();
    
    update();
    absorb();
    m_inf.Clear();

    bitset_t old_black = getColor(BLACK);
    bitset_t old_white = getColor(WHITE);
    ComputeInferiorCells(color_to_move, endgame_mode);

    bitset_t added[BLACK_AND_WHITE];
    added[BLACK] = getColor(BLACK) - old_black;
    added[WHITE] = getColor(WHITE) - old_white;
    
    if (m_use_vcs)
    {
        BuildVCs();
        HandleVCDecomposition(color_to_move, endgame_mode);
    }

    double e = Time::Get();
    LogFine() << (e-s) << "s to compute all." << '\n';
}

void HexBoard::PlayMove(HexColor color, HexPoint cell)
{
    double s = Time::Get();
    LogFine() << "Playing (" << color << ", " 
             << cell << ")" << '\n';

    PushHistory(color, cell);

    bitset_t old_black = getColor(BLACK);
    bitset_t old_white = getColor(WHITE);

    playMove(color, cell);
    update(cell);
    absorb(cell);

    ComputeInferiorCells(!color, DO_NOT_REMOVE_WINNING_FILLIN);

    bitset_t added[BLACK_AND_WHITE];
    added[BLACK] = getColor(BLACK) - old_black;
    added[WHITE] = getColor(WHITE) - old_white;

    if (m_use_vcs)
    {
        BuildVCs(added);
        HandleVCDecomposition(!color, DO_NOT_REMOVE_WINNING_FILLIN);
    }

    double e = Time::Get();
    LogFine() << (e-s) << "s to play stones." << '\n';
}

void HexBoard::PlayStones(HexColor color, const bitset_t& played,
                          HexColor color_to_move)
{
    HexAssert(BitsetUtil::IsSubsetOf(played, getEmpty()));

    double s = Time::Get();
    LogFine() << "Playing (" << color << ","
             << HexPointUtil::ToPointListString(played) << ")"
             << '\n';

    PushHistory(color, INVALID_POINT);

    bitset_t old_black = getColor(BLACK);
    bitset_t old_white = getColor(WHITE);

    addColor(color, played);
    update(played);
    absorb(played);

    ComputeInferiorCells(color_to_move, DO_NOT_REMOVE_WINNING_FILLIN);

    bitset_t added[BLACK_AND_WHITE];
    added[BLACK] = getColor(BLACK) - old_black;
    added[WHITE] = getColor(WHITE) - old_white;

    if (m_use_vcs)
    {
        BuildVCs(added);
        HandleVCDecomposition(color_to_move, 
                              DO_NOT_REMOVE_WINNING_FILLIN);
    }

    double e = Time::Get();
    LogFine() << (e-s) << "s to play stones." << '\n';
}

void HexBoard::AddStones(HexColor color, const bitset_t& played,
                         HexColor color_to_move, 
                         EndgameFillin endgame_mode)
{
    HexAssert(BitsetUtil::IsSubsetOf(played, getEmpty()));

    double s = Time::Get();
    LogFine() << "Adding (" << color << ", "
             << HexPointUtil::ToPointListString(played) << ")"
             << '\n';

    bitset_t old_black = getColor(BLACK);
    bitset_t old_white = getColor(WHITE);

    addColor(color, played);
    update(played);
    absorb(played);

    ComputeInferiorCells(color_to_move, endgame_mode);

    bitset_t added[BLACK_AND_WHITE];
    added[BLACK] = getColor(BLACK) - old_black;
    added[WHITE] = getColor(WHITE) - old_white;

    if (m_use_vcs)
        BuildVCs(added, false); 

    double e = Time::Get();
    LogFine() << (e-s) << "s to add stones." << '\n';
}

void HexBoard::UndoMove()
{
    double s = Time::Get();

    PopHistory();
    update();
    absorb();
    
    double e = Time::Get();
    LogFine() << (e-s) << "s to undo move." << '\n';
}

//----------------------------------------------------------------------------

void HexBoard::ClearHistory()
{
    m_history.clear();
}

void HexBoard::PushHistory(HexColor color, HexPoint cell)
{
    History hist(*this, m_inf, m_backedup, color, cell);
    m_history.push_back(hist);
}

void HexBoard::PopHistory()
{
    HexAssert(!m_history.empty());

    History hist = m_history.back();
    m_history.pop_back();

    // restore the old board position
    startNewGame();
    setColor(BLACK, hist.board.getBlack());
    setColor(WHITE, hist.board.getWhite());
    setPlayed(hist.board.getPlayed());

    // backup the ice info
    if (m_backup_ice_info && hist.last_played != INVALID_POINT)
    {
        bitset_t a = getEmpty() - hist.inf.All();
        a &= m_inf.Dead() | m_inf.Captured(hist.to_play);

        for (BitsetIterator p(a); p; ++p) {
            hist.inf.AddDominated(*p, hist.last_played);
            hist.backedup.insert(std::make_pair(*p, hist.last_played));
        }
    }

    m_inf = hist.inf;
    m_backedup = hist.backedup;

    RevertVCs();
}

void HexBoard::AddDominationArcs(const std::set<HexPointPair>& dom)
{
    std::set<HexPointPair>::const_iterator it;
    for (it=dom.begin(); it!=dom.end(); ++it) {
        m_inf.AddDominated(it->first, it->second);
    }
}

//----------------------------------------------------------------------------
