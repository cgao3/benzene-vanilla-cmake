//----------------------------------------------------------------------------
/** @file StoneBoard.cpp
 */
//----------------------------------------------------------------------------

#include "BoardUtils.hpp"
#include "StoneBoard.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

StoneBoard::StoneBoard()
    : m_const(0)
{
}

StoneBoard::StoneBoard(unsigned size)
    : m_const(&ConstBoard::Get(size))
{
    startNewGame();
}

StoneBoard::StoneBoard(unsigned width, unsigned height)
    : m_const(&ConstBoard::Get(width, height))
{
    startNewGame();
}

StoneBoard::~StoneBoard()
{
}

//----------------------------------------------------------------------------

HexColor StoneBoard::getColor(HexPoint cell) const
{
    HexAssert(isValid(cell));
    if (isBlack(cell)) return BLACK;
    if (isWhite(cell)) return WHITE;
    return EMPTY;
}

bitset_t StoneBoard::getLegal() const
{
    bitset_t legal;
    if (isPlayed(RESIGN)) return legal;
    
    legal = getPlayed();
    legal.flip();
    legal &= Const().getCells();
    legal.set(RESIGN);
    
    // swap is available only when 4 edges and exactly
    // one cell have been played
    if (getPlayed().count() == 5) {
	HexAssert(!isPlayed(SWAP_PIECES));
	HexAssert(getColor(FIRST_TO_PLAY).count() >= 3);
	HexAssert((getPlayed() & getColor(FIRST_TO_PLAY)).count() == 3);
	HexAssert(getColor(!FIRST_TO_PLAY).count() == 2);
	legal.set(SWAP_PIECES);
    }
    
    HexAssert(isValid(legal));
    return legal;
}

bool StoneBoard::isLegal(HexPoint cell) const
{
    HexAssert(isValid(cell));
    return getLegal().test(cell);
}

const BoardIterator& StoneBoard::Stones(HexColorSet colorset) const
{
    if (!m_stones_calculated) {

        for (int i=0; i<NUM_COLOR_SETS; ++i) {
            m_stones_list[i].clear();
        }

        for (BoardIterator p(EdgesAndInterior()); p; ++p) {
            for (int i=0; i<NUM_COLOR_SETS; ++i) {
                if (HexColorSetUtil::InSet(getColor(*p), (HexColorSet)i))
                    m_stones_list[i].push_back(*p);
            }
        }        

        for (int i=0; i<NUM_COLOR_SETS; ++i) {
            m_stones_list[i].push_back(INVALID_POINT);
            m_stones_iter[i] = BoardIterator(m_stones_list[i]);
        }

        m_stones_calculated = true;
    }

    return m_stones_iter[colorset];
}

//----------------------------------------------------------------------------

void StoneBoard::MarkAsDirty()
{
    m_stones_calculated = false;
}

void StoneBoard::addColor(HexColor color, const bitset_t& b)
{
    HexAssert(HexColorUtil::isIsBlackWhite(color));
    m_stones[color] |= b;
    HexAssert(IsBlackWhiteDisjoint());
    if (b.any()) MarkAsDirty();
}

void StoneBoard::removeColor(HexColor color, const bitset_t& b)
{
    HexAssert(HexColorUtil::isBlackWhite(color));
    m_stones[color] = m_stones[color] - b;
    HexAssert(IsBlackWhiteDisjoint());
    if (b.any()) MarkAsDirty();
}

void StoneBoard::setColor(HexColor color, HexPoint cell)
{
    HexAssert(HexColorUtil::isValidColor(color));
    HexAssert(isValid(cell));

    if (color == EMPTY) {
	for (BWIterator it; it; ++it)
	    m_stones[*it].reset(cell);
    } else {
        m_stones[color].set(cell);
        HexAssert(IsBlackWhiteDisjoint());
    }

    MarkAsDirty();
}

void StoneBoard::setColor(HexColor color, const bitset_t& bs)
{
    /** @todo Should we make this support EMPTY color too? */
    HexAssert(HexColorUtil::isBlackWhite(color));
    HexAssert(isValid(bs));

    m_stones[color] = bs;
    HexAssert(IsBlackWhiteDisjoint());

    MarkAsDirty();
}

void StoneBoard::setPlayed(const bitset_t& played)
{
    m_played = played;
    ComputeHash();
    MarkAsDirty();
}

//----------------------------------------------------------------------------

void StoneBoard::ComputeHash()
{
    // do not include swap in hash value
    bitset_t mask = m_played & Const().getLocations();
    m_hash.compute(m_stones[BLACK] & mask, m_stones[WHITE] & mask);
}

void StoneBoard::startNewGame()
{
    m_played.reset();
    for (BWIterator it; it; ++it) 
    {
        m_stones[*it].reset();
        playMove(*it, HexPointUtil::colorEdge1(*it));
        playMove(*it, HexPointUtil::colorEdge2(*it));
    }
    ComputeHash();
    MarkAsDirty();
}

void StoneBoard::playMove(HexColor color, HexPoint cell)
{
    HexAssert(HexColorUtil::isBlackWhite(color));
    HexAssert(isValid(cell));

    m_played.set(cell);
    if (Const().isLocation(cell))
        m_hash.update(color, cell);
    setColor(color, cell);

    MarkAsDirty();
}

void StoneBoard::undoMove(HexPoint cell)
{
    HexAssert(isValid(cell));
    HexColor color = getColor(cell);
    HexAssert(color != EMPTY);

    m_played.reset(cell);
    if (Const().isLocation(cell))
        m_hash.update(color, cell);
    setColor(EMPTY, cell);

    MarkAsDirty();
}

//----------------------------------------------------------------------------

void StoneBoard::rotateBoard()
{
    m_played = BoardUtils::Rotate(Const(), m_played);
    for (BWIterator it; it; ++it)
        m_stones[*it] = BoardUtils::Rotate(Const(), m_stones[*it]);
    ComputeHash();
    MarkAsDirty();
}

void StoneBoard::mirrorBoard()
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
    std::size_t n = (width()*height()+3)/4*4;

    /** @note When this code was written, the cells were iterated over
        in the order (a1, b1, c1, ..., a2, b2, c2, ... , etc). Any
        changes to the order in Interior() will break all existing
        databases that use BoardID as a lookup, unless this method is
        updated to always compute in the above order. */
    std::size_t i = 0;
    std::vector<byte> val(n, 0);
    bitset_t played = getPlayed();
    for (BoardIterator p(Interior()); p; ++p, ++i) {
        val[i] = (played.test(*p)) 
            ? static_cast<byte>(getColor(*p))
            : static_cast<byte>(EMPTY);
    }

    BoardID id;
    for (i=0; i<n; i+=4) {
        id.push_back(val[i] 
                     + (val[i+1] << 2)
                     + (val[i+2] << 4)
                     + (val[i+3] << 6));
    }
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

void StoneBoard::SetState(const BoardID& id)
{
    std::size_t n = (width()*height()+3)/4*4;
    HexAssert(id.size() == n/4);

    std::vector<byte> val(n, 0);
    for (std::size_t i=0; i<n; i+=4) {
        byte packed = id[i/4];
        val[i] = packed & 0x3;
        val[i+1] = (packed >> 2) & 0x3;
        val[i+2] = (packed >> 4) & 0x3;
        val[i+3] = (packed >> 6) & 0x3;
    }

    /** @note This depends on the order defined by Interior().
        See note in implementation of GetBoardID(). */
    startNewGame();
    std::size_t i = 0;
    for (BoardIterator p(Interior()); p; ++p, ++i) {
        HexColor color = static_cast<HexColor>(val[i]);
        if (color == BLACK || color == WHITE)
            playMove(color, *p);
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
    for (int i = 0; i < width(); i++) 
        out << (char)('a' + i) << "  ";
    out << '\n';
    for (int i = 0; i < height(); i++) 
    {
        for (int j = 0; j < i; j++) 
            out << " ";
        if (i + 1 < 10) 
            out << " ";
        out << (i + 1) << "\\";
        for (int j = 0; j < width(); j++) 
        {
            HexPoint p = HexPointUtil::coordsToPoint(j, i);
            if (j) 
                out << "  ";
            if (b.test(p))
                out << "*";
            else if (isBlack(p) && isPlayed(p))
                out << "B";
            else if (isBlack(p))
                out << "b";
            else if (isWhite(p) && isPlayed(p))
                out << "W";
            else if (isWhite(p))
                out << "w";
            else
                out << ".";
        }
        out << "\\";
        out << (i + 1); 
        out << '\n';
    }
    for (int j = 0; j < height(); j++) 
        out << " ";
    out << "   ";
    for (int i = 0; i < width(); i++)
        out << (char)('a' + i) << "  ";
    return out.str();
}

bool StoneBoard::IsBlackWhiteDisjoint()
{
    if ((m_stones[BLACK] & m_stones[WHITE]).any()) {
        for (BWIterator it; it; ++it)
            LogWarning() << HexPointUtil::ToPointListString(m_stones[*it])
			 << '\n';
        return false;
    }
    return true;
}

//----------------------------------------------------------------------------
