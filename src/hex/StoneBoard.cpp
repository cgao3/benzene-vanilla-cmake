//----------------------------------------------------------------------------
/** @file StoneBoard.cpp
 */
//----------------------------------------------------------------------------

#include "BoardUtils.hpp"
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
    HexAssert(Const().IsValid(cell));
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
	HexAssert(!IsPlayed(SWAP_PIECES));
	HexAssert(GetColor(FIRST_TO_PLAY).count() >= 3);
	HexAssert((GetPlayed(FIRST_TO_PLAY)).count() == 3);
	HexAssert(GetColor(!FIRST_TO_PLAY).count() == 2);
	legal.set(SWAP_PIECES);
    }
    HexAssert(Const().IsValid(legal));
    return legal;
}

bool StoneBoard::IsLegal(HexPoint cell) const
{
    HexAssert(Const().IsValid(cell));
    return GetLegal().test(cell);
}

BoardIterator StoneBoard::Stones(HexColorSet colorset) const
{
    if (!m_stones_calculated) 
    {
        for (int i = 0; i < NUM_COLOR_SETS; ++i)
        {
            m_stones_list[i].clear();
            for (BoardIterator p(Const().EdgesAndInterior()); p; ++p) 
                if (HexColorSetUtil::InSet(GetColor(*p), (HexColorSet)i))
                    m_stones_list[i].push_back(*p);
            m_stones_list[i].push_back(INVALID_POINT);
        }
        m_stones_calculated = true;
    }
    return BoardIterator(m_stones_list[colorset]);
}

//----------------------------------------------------------------------------

void StoneBoard::MarkAsDirty()
{
    m_stones_calculated = false;
}

void StoneBoard::AddColor(HexColor color, const bitset_t& b)
{
    HexAssert(HexColorUtil::isBlackWhite(color));
    m_stones[color] |= b;
    HexAssert(IsBlackWhiteDisjoint());
    if (b.any()) MarkAsDirty();
}

void StoneBoard::RemoveColor(HexColor color, const bitset_t& b)
{
    HexAssert(HexColorUtil::isBlackWhite(color));
    m_stones[color] = m_stones[color] - b;
    HexAssert(IsBlackWhiteDisjoint());
    if (b.any()) MarkAsDirty();
}

void StoneBoard::SetColor(HexColor color, HexPoint cell)
{
    HexAssert(HexColorUtil::isValidColor(color));
    HexAssert(Const().IsValid(cell));

    if (color == EMPTY) {
	for (BWIterator it; it; ++it)
	    m_stones[*it].reset(cell);
    } else {
        m_stones[color].set(cell);
        HexAssert(IsBlackWhiteDisjoint());
    }

    MarkAsDirty();
}

void StoneBoard::SetColor(HexColor color, const bitset_t& bs)
{
    /** @todo Should we make this support EMPTY color too? */
    HexAssert(HexColorUtil::isBlackWhite(color));
    HexAssert(Const().IsValid(bs));

    m_stones[color] = bs;
    HexAssert(IsBlackWhiteDisjoint());

    MarkAsDirty();
}

void StoneBoard::SetPlayed(const bitset_t& played)
{
    m_played = played;
    ComputeHash();
    MarkAsDirty();
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
    MarkAsDirty();
}

void StoneBoard::PlayMove(HexColor color, HexPoint cell)
{
    HexAssert(HexColorUtil::isBlackWhite(color));
    HexAssert(Const().IsValid(cell));

    m_played.set(cell);
    if (Const().IsLocation(cell))
        m_hash.Update(color, cell);
    SetColor(color, cell);

    MarkAsDirty();
}

void StoneBoard::UndoMove(HexPoint cell)
{
    HexAssert(Const().IsValid(cell));
    HexColor color = GetColor(cell);
    HexAssert(color != EMPTY);

    m_played.reset(cell);
    if (Const().IsLocation(cell))
        m_hash.Update(color, cell);
    SetColor(EMPTY, cell);

    MarkAsDirty();
}

//----------------------------------------------------------------------------

void StoneBoard::RotateBoard()
{
    m_played = BoardUtils::Rotate(Const(), m_played);
    for (BWIterator it; it; ++it)
        m_stones[*it] = BoardUtils::Rotate(Const(), m_stones[*it]);
    ComputeHash();
    MarkAsDirty();
}

bool StoneBoard::IsSelfRotation() const
{
    if (m_stones[BLACK] != BoardUtils::Rotate(Const(), m_stones[BLACK]))
        return false;
    if (m_stones[WHITE] != BoardUtils::Rotate(Const(), m_stones[WHITE]))
        return false;
    return true;
}

void StoneBoard::MirrorBoard()
{
    m_played = BoardUtils::Mirror(Const(), m_played);
    for (BWIterator it; it; ++it)
        m_stones[*it] = BoardUtils::Mirror(Const(), m_stones[*it]);
    ComputeHash();
    MarkAsDirty();
}

//----------------------------------------------------------------------------

BoardID StoneBoard::GetBoardID() const
{
    /** Packs each interior cell into 2 bits.
        @note Assumes all valid HexColors lie between [0,2]. */
    std::size_t n = (Width() * Height() + 3) / 4 * 4;

    /** @note When this code was written, the cells were iterated over
        in the order (a1, b1, c1, ..., a2, b2, c2, ... , etc). Any
        changes to the order in Interior() will break all existing
        databases that use BoardID as a lookup, unless this method is
        updated to always compute in the above order. */
    std::size_t i = 0;
    std::vector<byte> val(n, 0);
    bitset_t played = GetPlayed();
    for (BoardIterator p(Const().Interior()); p; ++p, ++i) 
        val[i] = (played.test(*p)) 
            ? static_cast<byte>(GetColor(*p))
            : static_cast<byte>(EMPTY);

    BoardID id;
    for (i = 0; i < n; i += 4)
        id.push_back(val[i] 
                     + (val[i+1] << 2)
                     + (val[i+2] << 4)
                     + (val[i+3] << 6));
    return id;
}

std::string StoneBoard::GetBoardIDString() const
{
    std::string hexval[16] = {"0", "1", "2", "3", "4", "5", "6", "7",
			      "8", "9", "a", "b", "c", "d", "e", "f"};
    BoardID brdID = GetBoardID();
    std::string idString;
    for (std::size_t i = 0; i < brdID.size(); ++i) {
	byte b = brdID[i];
	idString += hexval[((b >> 4) & 0xF)];
	idString += hexval[b & 0x0F];
    }
    return idString;
}

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

void StoneBoard::SetPosition(const BoardID& id)
{
    std::size_t n = (Width() * Height() + 3) / 4 * 4;
    HexAssert(id.size() == n / 4);

    std::vector<byte> val(n, 0);
    for (std::size_t i = 0; i < n; i += 4) 
    {
        byte packed = id[i/4];
        val[i] = packed & 0x3;
        val[i+1] = (packed >> 2) & 0x3;
        val[i+2] = (packed >> 4) & 0x3;
        val[i+3] = (packed >> 6) & 0x3;
    }

    /** @note This depends on the order defined by Interior().
        See note in implementation of GetBoardID(). */
    StartNewGame();
    std::size_t i = 0;
    for (BoardIterator p(Const().Interior()); p; ++p, ++i) 
    {
        HexColor color = static_cast<HexColor>(val[i]);
        if (color == BLACK || color == WHITE)
            PlayMove(color, *p);
    }

    ComputeHash();
    MarkAsDirty();
}

void StoneBoard::SetPosition(const std::string& str)
{
    /** @note This depends on the order defined by Interior(). */
    StartNewGame();
    for (std::size_t i = 0, cell = 0; 
         i < str.size() && (int)cell < Width()*Height();
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
    MarkAsDirty();
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
    out << "  " << HashUtil::toString(Hash()) << '\n';
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
