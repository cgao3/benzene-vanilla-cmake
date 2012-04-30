//----------------------------------------------------------------------------
/** @file MoHexBoard.hpp */
//----------------------------------------------------------------------------

#ifndef MOHEXBOARD_HPP
#define MOHEXBOARD_HPP

#include <stdint.h>
#include <boost/static_assert.hpp>
#include "StoneBoard.hpp"

class SgRandom;

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Track info to speedup SaveBridge().
    Slows down PlayMove() by around 7%, but speeds up SaveBridge() by
    15% in practice, so roughly 7% gain. Turn this off if not using
    SaveBridge() in playouts. */
#define TRACK_LAST_MOVE_FOR_SAVE_BRIDGE 1

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

    HexPoint Parent(HexPoint c) const;

    HexPoint SaveBridge(const HexPoint lastMove, const HexColor toPlay,
                        SgRandom& random) const;


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

#if TRACK_LAST_MOVE_FOR_SAVE_BRIDGE
    int8_t m_lastMove;
    int8_t m_emptyNbs;
    int8_t m_oppNbs;
#endif

    void SetConstBoard(const ConstBoard& brd);
    void SetColor(HexPoint cell, HexColor color);
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

inline HexPoint MoHexBoard::SaveBridge(const HexPoint lastMove, 
                                       const HexColor toPlay,
                                       SgRandom& random) const
{
#if TRACK_LAST_MOVE_FOR_SAVE_BRIDGE
    if (m_oppNbs < 2 || m_emptyNbs == 0 || m_emptyNbs > 4)
        return INVALID_POINT;
#endif
    // State machine: s is number of cells matched.
    // In clockwise order, need to match CEC, where C is the color to
    // play and E is an empty cell. We start at a random direction and
    // stop at first match, which handles the case of multiple bridge
    // patterns occuring at once.
    // TODO: speed this up?
    int s = 0;
    const int start = random.Int(6);
    HexPoint ret = INVALID_POINT;
    for (int j = 0; j < 8; ++j)
    {
        const int i = (j + start) % 6;
        const HexPoint p = Const().PointInDir(lastMove, (HexDirection)i);
        const bool mine = GetColor(p) == toPlay;
        if (s == 0)
        {
            if (mine) s = 1;
        }
        else if (s == 1)
        {
            if (mine) s = 1;
            else if (GetColor(p) == !toPlay) s = 0;
            else
            {
                s = 2;
                ret = p;
            }
        }
        else if (s == 2)
        {
            if (mine) return ret; // matched!!
            else s = 0;
        }
    }
    return INVALID_POINT;
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // MOHEXBOARD_HPP
