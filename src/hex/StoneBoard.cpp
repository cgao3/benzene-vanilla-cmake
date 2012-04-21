//----------------------------------------------------------------------------
/** @file StoneBoard.cpp */
//----------------------------------------------------------------------------

#include "BoardUtil.hpp"
#include "StoneBoard.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

StoneBoard::StoneBoard()
    : m_const(0),
      m_hash(0, 0)
{
}

StoneBoard::StoneBoard(unsigned size)
    : m_const(&ConstBoard::Get(size)),
      m_hash(size, size)
{
    StartNewGame();
}

StoneBoard::StoneBoard(unsigned width, unsigned height)
    : m_const(&ConstBoard::Get(width, height)),
      m_hash(width, height)
{
    StartNewGame();
}

StoneBoard::StoneBoard(unsigned width, unsigned height, const std::string& str)
    : m_const(&ConstBoard::Get(width, height)),
      m_hash(width, height)
{
    SetPosition(str);
}

StoneBoard::~StoneBoard()
{
}

//----------------------------------------------------------------------------

HexColor StoneBoard::GetColor(HexPoint cell) const
{
    BenzeneAssert(Const().IsValid(cell));
    if (IsBlack(cell)) return BLACK;
    if (IsWhite(cell)) return WHITE;
    return EMPTY;
}

bitset_t StoneBoard::GetLegal() const
{
    bitset_t legal;
    if (IsPlayed(RESIGN)) 
        return legal;
    
    legal = ~GetPlayed() & Const().GetCells();
    legal.set(RESIGN);
    
    // Swap is available only when 4 edges and exactly
    // one cell have been played
    if (GetPlayed().count() == 5) 
    {
	BenzeneAssert(!IsPlayed(SWAP_PIECES));
	BenzeneAssert(GetColor(FIRST_TO_PLAY).count() >= 3);
	BenzeneAssert((GetPlayed(FIRST_TO_PLAY)).count() == 3);
	BenzeneAssert(GetColor(!FIRST_TO_PLAY).count() == 2);
	legal.set(SWAP_PIECES);
    }
    BenzeneAssert(Const().IsValid(legal));
    return legal;
}

bool StoneBoard::IsLegal(HexPoint cell) const
{
    BenzeneAssert(Const().IsValid(cell));
    return GetLegal().test(cell);
}

//----------------------------------------------------------------------------

void StoneBoard::AddColor(HexColor color, const bitset_t& b)
{
    BenzeneAssert(HexColorUtil::isBlackWhite(color));
    m_stones[color] |= b;
    BenzeneAssert(IsBlackWhiteDisjoint());
}

void StoneBoard::RemoveColor(HexColor color, const bitset_t& b)
{
    BenzeneAssert(HexColorUtil::isBlackWhite(color));
    m_stones[color] = m_stones[color] - b;
    BenzeneAssert(IsBlackWhiteDisjoint());
}

void StoneBoard::SetColor(HexColor color, HexPoint cell)
{
    BenzeneAssert(HexColorUtil::isValidColor(color));
    BenzeneAssert(Const().IsValid(cell));
    if (color == EMPTY) {
        m_stones[BLACK].reset(cell);
        m_stones[WHITE].reset(cell);
    } else {
        m_stones[color].set(cell);
        BenzeneAssert(IsBlackWhiteDisjoint());
    }
}

void StoneBoard::SetColor(HexColor color, const bitset_t& bs)
{
    /** @todo Should we make this support EMPTY color too? */
    BenzeneAssert(HexColorUtil::isBlackWhite(color));
    BenzeneAssert(Const().IsValid(bs));
    m_stones[color] = bs;
    BenzeneAssert(IsBlackWhiteDisjoint());
}

void StoneBoard::SetPlayed(const bitset_t& played)
{
    m_played = played;
    ComputeHash();
}

//----------------------------------------------------------------------------

void StoneBoard::ComputeHash()
{
    // do not include swap in hash value
    bitset_t mask = m_played & Const().GetLocations();
    m_hash.Compute(m_stones[BLACK] & mask, m_stones[WHITE] & mask);
}

void StoneBoard::StartNewGame()
{
    m_played.reset();
    for (BWIterator it; it; ++it) 
    {
        m_stones[*it].reset();
        PlayMove(*it, HexPointUtil::colorEdge1(*it));
        PlayMove(*it, HexPointUtil::colorEdge2(*it));
    }
    ComputeHash();
}

void StoneBoard::PlayMove(HexColor color, HexPoint cell)
{
    BenzeneAssert(HexColorUtil::isBlackWhite(color));
    BenzeneAssert(Const().IsValid(cell));

    m_played.set(cell);
    if (Const().IsLocation(cell))
        m_hash.Update(color, cell);
    SetColor(color, cell);
}

void StoneBoard::UndoMove(HexPoint cell)
{
    BenzeneAssert(Const().IsValid(cell));
    HexColor color = GetColor(cell);
    BenzeneAssert(color != EMPTY);

    m_played.reset(cell);
    if (Const().IsLocation(cell))
        m_hash.Update(color, cell);
    SetColor(EMPTY, cell);
}

//----------------------------------------------------------------------------

void StoneBoard::RotateBoard()
{
    m_played = BoardUtil::Rotate(Const(), m_played);
    for (BWIterator it; it; ++it)
        m_stones[*it] = BoardUtil::Rotate(Const(), m_stones[*it]);
    ComputeHash();
}

bool StoneBoard::IsSelfRotation() const
{
    if (m_stones[BLACK] != BoardUtil::Rotate(Const(), m_stones[BLACK]))
        return false;
    if (m_stones[WHITE] != BoardUtil::Rotate(Const(), m_stones[WHITE]))
        return false;
    return true;
}

void StoneBoard::MirrorBoard()
{
    m_played = BoardUtil::Mirror(Const(), m_played);
    for (BWIterator it; it; ++it)
        m_stones[*it] = BoardUtil::Mirror(Const(), m_stones[*it]);
    ComputeHash();
}

//----------------------------------------------------------------------------

void StoneBoard::SetPosition(const StoneBoard& brd)
{
    StartNewGame();
    SetColor(BLACK, brd.GetBlack());
    SetColor(WHITE, brd.GetWhite());
    SetPlayed(brd.GetPlayed());
}

void StoneBoard::SetPositionOnlyPlayed(const StoneBoard& brd)
{
    StartNewGame();
    SetColor(BLACK, brd.GetBlack() & brd.GetPlayed());
    SetColor(WHITE, brd.GetWhite() & brd.GetPlayed());
    SetPlayed(brd.GetPlayed());
}

void StoneBoard::SetPosition(const std::string& str)
{
    /** @note This depends on the order defined by Interior(). */
    StartNewGame();
    int cell = 0;
    for (std::size_t i = 0;
         i < str.size() && cell < Width() * Height();
         ++i)
    {
        int x = cell % Width();
        int y = cell / Width();
        HexPoint p = HexPointUtil::coordsToPoint(x, y);
        if (str[i] == '.')
            cell++;
        else if (str[i] == 'B')
        {
            PlayMove(BLACK, p);
            cell++;
        }
        else if (str[i] == 'W')
        {
            PlayMove(WHITE, p);
            cell++;
        }
        else if (str[i] == 'b')
        {
            SetColor(BLACK, p);
            cell++;
        }
        else if (str[i] == 'w')
        {
            SetColor(WHITE, p);
            cell++;
        }
    }
    ComputeHash();
}

//----------------------------------------------------------------------------

std::string StoneBoard::Write() const
{
    return Write(EMPTY_BITSET);
}

std::string StoneBoard::Write(const bitset_t& b) const
{
    std::ostringstream out;
    out << '\n';
    out << "  " << Hash() << '\n';
    out << "  ";
    for (int i = 0; i < Width(); i++) 
        out << (char)('a' + i) << "  ";
    out << '\n';
    for (int i = 0; i < Height(); i++) 
    {
        for (int j = 0; j < i; j++) 
            out << " ";
        if (i + 1 < 10) 
            out << " ";
        out << (i + 1) << "\\";
        for (int j = 0; j < Width(); j++) 
        {
            HexPoint p = HexPointUtil::coordsToPoint(j, i);
            if (j) 
                out << "  ";
            if (b.test(p))
                out << "*";
            else if (IsBlack(p) && IsPlayed(p))
                out << "B";
            else if (IsBlack(p))
                out << "b";
            else if (IsWhite(p) && IsPlayed(p))
                out << "W";
            else if (IsWhite(p))
                out << "w";
            else
                out << ".";
        }
        out << "\\";
        out << (i + 1); 
        out << '\n';
    }
    for (int j = 0; j < Height(); j++) 
        out << " ";
    out << "   ";
    for (int i = 0; i < Width(); i++)
        out << (char)('a' + i) << "  ";
    return out.str();
}

bool StoneBoard::IsBlackWhiteDisjoint()
{
    if ((m_stones[BLACK] & m_stones[WHITE]).any()) 
    {
        for (BWIterator it; it; ++it)
            LogWarning() << HexPointUtil::ToString(m_stones[*it]) << '\n';
        return false;
    }
    return true;
}

//----------------------------------------------------------------------------
