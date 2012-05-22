//----------------------------------------------------------------------------
/** @file MoHexBoard.cpp */
//----------------------------------------------------------------------------

#include "BoardUtil.hpp"
#include "MoHexBoard.hpp"
#include "MoHexPatterns.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

MoHexBoard::MoHexBoard()
    : m_const(0)
{
}

MoHexBoard::~MoHexBoard()
{
}

//----------------------------------------------------------------------------

void MoHexBoard::Clear()
{
    m_numMoves = 0;

    m_cell[NORTH].color = BLACK;
    m_cell[NORTH].parent = NORTH;

    m_cell[SOUTH].color = BLACK;
    m_cell[SOUTH].parent = SOUTH;

    m_cell[WEST].color = WHITE;
    m_cell[WEST].parent = WEST;

    m_cell[EAST].color = WHITE;
    m_cell[EAST].parent = EAST;

    for (BoardIterator it(Const().Interior()); it; ++it)
    {
        m_cell[*it].color = EMPTY;
        m_cell[*it].parent = *it;
    }
}

void MoHexBoard::SetPosition(const StoneBoard& pos)
{
    SetConstBoard(pos.Const());
    Clear();
    ComputeKeysOnEmptyBoard();
    for (BoardIterator it(Const().Interior()); it; ++it)
        if (pos.IsOccupied(*it))
            PlayMove(*it, pos.GetColor(*it));

#if 0
    for (BoardIterator it(Const().Interior()); it; ++it)
    {
        if (GetColor(*it) != EMPTY)
            continue;
        uint64_t keys[3];
        MoHexPatterns::GetKeyFromBoard(keys, 12, *this, *it, BLACK);
        if ((keys[0] != Keys(*it)[0]) || (keys[1] != Keys(*it)[1]))
        {
            LogInfo() << "move=" << *it << '\n'
                      << " key[0]=" << keys[0] << '\n'
                      << "bkey[0]=" << Keys(*it)[0] << '\n'
                      << " key[1]=" << keys[1] << '\n'
                      << "bkey[1]=" << Keys(*it)[1] << '\n';
            throw BenzeneException("blah");
        }
    }
#endif
}

HexPoint MoHexBoard::Parent(HexPoint c) const
{
    uint8_t p = m_cell[c].parent;
    if (p != c) 
    {
        do {
            p = m_cell[p].parent;
        } while (m_cell[p].parent != p);
        m_cell[c].parent = p;
    }
    return static_cast<HexPoint>(p);
}

void MoHexBoard::Merge(HexPoint c1, HexPoint c2)
{
    HexPoint p1 = Parent(c1);
    HexPoint p2 = Parent(c2);
    if (p1 == p2)
        return;
    // Smaller parent id becomes new parent:
    // Edges will always be parents of their groups.
    if (p1 < p2)
        m_cell[p2].parent = p1;
    else 
        m_cell[p1].parent = p2;
}

void MoHexBoard::PlayMove(HexPoint cell, HexColor toPlay)
{
    m_numMoves++;
    SetColor(cell, toPlay);

#if TRACK_LAST_MOVE_FOR_SAVE_BRIDGE
    m_lastMove = cell;
    m_emptyNbs = 0;
    m_oppNbs = 0;
#endif

    for (BoardIterator n(m_const->Nbs(cell)); n; ++n)
    {
        if (GetColor(*n) == toPlay)
            Merge(cell, *n);
#if TRACK_LAST_MOVE_FOR_SAVE_BRIDGE
        else if (GetColor(*n) == !toPlay)
            m_oppNbs++;
        else
            m_emptyNbs++;
#endif
    }
    
    UpdateKeys(cell, toPlay);
}

void MoHexBoard::ComputeKeysOnEmptyBoard()
{
    for (BoardIterator it(Const().Interior()); it; ++it)
    {
        MoHexPatterns::GetKeyFromBoard(&m_keys[*it][0], 12, *this,
                                       *it, BLACK);
    }
}

void MoHexBoard::UpdateKeys(const HexPoint cell, const HexColor color)
{
    static const int inverse[] = 
        { 0, 6, 5, 4, 3, 2, 1, 12, 11, 10, 9, 8, 7 };

    const int c = (color == BLACK) ? 1 : 2;
    for (int i = 1; i <= 6; ++i)
    {
        const HexPoint n = Const().PatternPoint(cell, i, BLACK);
        if (GetColor(n) == EMPTY)
        {
            m_keys[n][0] ^= MoHexPatterns::m_zobrist[ inverse[i] ][ 0 ];
            m_keys[n][0] ^= MoHexPatterns::m_zobrist[ inverse[i] ][ c ];
            
            m_keys[n][1] ^= MoHexPatterns::m_zobrist[ inverse[i] ][ 0 ];
            m_keys[n][1] ^= MoHexPatterns::m_zobrist[ inverse[i] ][ c ];
        }
    }
    for (int i = 7; i <= 12; ++i)
    {
        const HexPoint n = Const().PatternPoint(cell, i, BLACK);
        if (GetColor(n) == EMPTY)
        {
            m_keys[n][1] ^= MoHexPatterns::m_zobrist[ inverse[i] ][ 0 ];
            m_keys[n][1] ^= MoHexPatterns::m_zobrist[ inverse[i] ][ c ];
        }
    }
}

//----------------------------------------------------------------------------

std::string MoHexBoard::Write() const
{
    return Write(EMPTY_BITSET);
}

std::string MoHexBoard::Write(const bitset_t& b) const
{
    std::ostringstream out;
    out << '\n';
    out << "  " << "hash" << '\n';
    out << "  ";
    for (int i = 0; i < Const().Width(); i++) 
        out << (char)('a' + i) << "  ";
    out << '\n';
    for (int i = 0; i < Const().Height(); i++) 
    {
        for (int j = 0; j < i; j++) 
            out << " ";
        if (i + 1 < 10) 
            out << " ";
        out << (i + 1) << "\\";
        for (int j = 0; j < Const().Width(); j++) 
        {
            HexPoint p = HexPointUtil::coordsToPoint(j, i);
            if (j) 
                out << "  ";
            if (b.test(p))
                out << "*";
            else if (GetColor(p) == BLACK)
                out << 'B';
            else if (GetColor(p) == WHITE)
                out << 'W';
            else
                out << ".";
        }
        out << "\\";
        out << (i + 1); 
        out << '\n';
    }
    for (int j = 0; j < Const().Height(); j++) 
        out << " ";
    out << "   ";
    for (int i = 0; i < Const().Width(); i++)
        out << (char)('a' + i) << "  ";
    return out.str();
}

//----------------------------------------------------------------------------
