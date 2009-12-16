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
    return (sizeof(win) + 
            sizeof(flags) +
            sizeof(numstates) +
            sizeof(nummoves) +
            sizeof(bestmove));
}

/** @bug NOT THREADSAFE! */
byte* DfsData::Pack() const
{
    // replace this to make it threadsafe
    static byte data[256];
    
    int index = 0;
    MiscUtil::WordToBytes(win, &data[index]);
    index += 4;

    MiscUtil::WordToBytes(flags, &data[index]);
    index += 4;
    
    MiscUtil::WordToBytes(numstates, &data[index]);
    index += 4;

    MiscUtil::WordToBytes(nummoves, &data[index]);
    index += 4;

    MiscUtil::WordToBytes(bestmove, &data[index]);
    index += 4;

    return data;
}

void DfsData::Unpack(const byte* data)
{
    int index = 0;
    
    win = MiscUtil::BytesToWord(&data[index]);
    index += 4;

    flags = MiscUtil::BytesToWord(&data[index]);
    index += 4;
    
    numstates = MiscUtil::BytesToWord(&data[index]);
    index += 4;

    nummoves = MiscUtil::BytesToWord(&data[index]);
    index += 4;

    bestmove = static_cast<HexPoint>(MiscUtil::BytesToWord(&data[index]));
}

void DfsData::Rotate(const ConstBoard& brd)
{
    bestmove = BoardUtils::Rotate(brd, bestmove);
}

//----------------------------------------------------------------------------
