//----------------------------------------------------------------------------
/** @file MoHexBoard.cpp */
//----------------------------------------------------------------------------

#include "BoardUtil.hpp"
#include "MoHexBoard.hpp"

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
    for (BoardIterator it(Const().Interior()); it; ++it)
        if (pos.IsOccupied(*it))
            PlayMove(*it, pos.GetColor(*it));
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
    SetColor(cell, toPlay);
    for (BoardIterator n(m_const->Nbs(cell)); n; ++n)
        if (GetColor(*n) == toPlay)
            Merge(cell, *n);
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
