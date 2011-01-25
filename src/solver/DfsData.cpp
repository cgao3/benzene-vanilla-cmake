//----------------------------------------------------------------------------
/** @file DfsData.cpp */
//----------------------------------------------------------------------------

#include "Misc.hpp"
#include "DfsData.hpp"
#include "BoardUtil.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

int DfsData::PackedSize() const
{
    return (sizeof(m_win) + 
            sizeof(m_flags) +
            sizeof(m_numStates) +
            sizeof(m_numMoves) +
            sizeof(m_bestMove));
}

/** @bug NOT THREADSAFE! */
byte* DfsData::Pack() const
{
    // replace this to make it threadsafe
    static byte data[256];
    
    int index = 0;
    MiscUtil::WordToBytes(m_win, &data[index]);
    index += 4;

    MiscUtil::WordToBytes(m_flags, &data[index]);
    index += 4;
    
    MiscUtil::WordToBytes(m_numStates, &data[index]);
    index += 4;

    MiscUtil::WordToBytes(m_numMoves, &data[index]);
    index += 4;

    MiscUtil::WordToBytes(m_bestMove, &data[index]);
    index += 4;

    return data;
}

void DfsData::Unpack(const byte* data)
{
    m_isValid = true;

    int index = 0;
    m_win = MiscUtil::BytesToWord(&data[index]);
    index += 4;

    m_flags = MiscUtil::BytesToWord(&data[index]);
    index += 4;
    
    m_numStates = MiscUtil::BytesToWord(&data[index]);
    index += 4;

    m_numMoves = MiscUtil::BytesToWord(&data[index]);
    index += 4;

    m_bestMove = static_cast<HexPoint>(MiscUtil::BytesToWord(&data[index]));
}

void DfsData::Rotate(const ConstBoard& brd)
{
    BenzeneAssert(m_isValid);
    m_bestMove = BoardUtil::Rotate(brd, m_bestMove);
}

void DfsData::Mirror(const ConstBoard& brd)
{
    BenzeneAssert(m_isValid);
    m_bestMove = BoardUtil::Mirror(brd, m_bestMove);
}
//----------------------------------------------------------------------------
