//----------------------------------------------------------------------------
/** @file MoHexBoard.hpp */
//----------------------------------------------------------------------------

#ifndef MOHEXBOARD_HPP
#define MOHEXBOARD_HPP

#include <stdint.h>
#include <boost/static_assert.hpp>
#include "ConstBoard.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

class MoHexBoard
{
public:
    MoHexBoard();

    ~MoHexBoard();

    void SetPosition(const StoneBoard& pos);

    void PlayMove(HexPoint cell, HexColor toPlay);

    void Clear();

    HexColor GetColor(HexPoint cell) const;

    const ConstBoard& Const() const;

    bool GameOver() const;

    HexColor GetWinner() const;

    int NumMoves() const;

    std::string Write() const;

    std::string Write(const bitset_t& b) const;

private:
    struct Cell
    {
        uint8_t color;
        mutable uint8_t parent;

        BOOST_STATIC_ASSERT(NORTH < SOUTH);
        BOOST_STATIC_ASSERT(EAST < WEST);
        BOOST_STATIC_ASSERT(BITSETSIZE <= 256);
    };

    const ConstBoard* m_const;
    Cell m_cell[BITSETSIZE];

    int8_t m_numMoves;

    void SetConstBoard(const ConstBoard& brd);
    void SetColor(HexPoint cell, HexColor color);
    HexPoint Parent(HexPoint c) const;
    void Merge(HexPoint c1, HexPoint c2);
};

inline const ConstBoard& MoHexBoard::Const() const
{
    return *m_const;
}

inline void MoHexBoard::SetConstBoard(const ConstBoard& brd)
{
    m_const = &brd;
}

inline HexColor MoHexBoard::GetColor(HexPoint cell) const
{
    return static_cast<HexColor>(m_cell[cell].color);
}

inline void MoHexBoard::SetColor(HexPoint cell, HexColor color)
{
    m_cell[cell].color = static_cast<uint8_t>(color & 0xf);
}

inline bool MoHexBoard::GameOver() const
{
    return GetWinner() != EMPTY;
}

inline HexColor MoHexBoard::GetWinner() const
{
    if (m_cell[SOUTH].parent == NORTH)
        return BLACK;
    if (m_cell[WEST].parent == EAST)
        return WHITE;
    return EMPTY;
}

inline int MoHexBoard::NumMoves() const
{
    return m_numMoves;
}

//----------------------------------------------------------------------------


_END_BENZENE_NAMESPACE_

#endif // MOHEXBOARD_HPP
