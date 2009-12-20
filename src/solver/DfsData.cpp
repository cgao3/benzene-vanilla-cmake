//----------------------------------------------------------------------------
/** @file DfsData.cpp
 */
//----------------------------------------------------------------------------

#include "Misc.hpp"
#include "DfsData.hpp"
#include "BoardUtils.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

int DfsData::PackedSize() const
{
    return (sizeof(m_win) + 
            sizeof(m_flags) +
            sizeof(m_numstates) +
            sizeof(m_nummoves) +
            sizeof(m_bestmove));
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
    
    MiscUtil::WordToBytes(m_numstates, &data[index]);
    index += 4;

    MiscUtil::WordToBytes(m_nummoves, &data[index]);
    index += 4;

    MiscUtil::WordToBytes(m_bestmove, &data[index]);
    index += 4;

    return data;
}

void DfsData::Unpack(const byte* data)
{
    int index = 0;
    
    m_win = MiscUtil::BytesToWord(&data[index]);
    index += 4;

    m_flags = MiscUtil::BytesToWord(&data[index]);
    index += 4;
    
    m_numstates = MiscUtil::BytesToWord(&data[index]);
    index += 4;

    m_nummoves = MiscUtil::BytesToWord(&data[index]);
    index += 4;

    m_bestmove = static_cast<HexPoint>(MiscUtil::BytesToWord(&data[index]));
}

void DfsData::Rotate(const ConstBoard& brd)
{
    m_bestmove = BoardUtils::Rotate(brd, m_bestmove);
}

//----------------------------------------------------------------------------
