//----------------------------------------------------------------------------
/** @file SolvedState.cpp
 */
//----------------------------------------------------------------------------

#include "Misc.hpp"
#include "SolvedState.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

void SolvedState::CheckCollision(const SolvedState& other) const
{
    if (this->black != other.black || this->white != other.white)
    {
        LogSevere() << "HASH COLLISION!" << '\n'
		    << "this:" << '\n'
		    << HexPointUtil::ToString(this->black) << '\n'
		    << HexPointUtil::ToString(this->white) << '\n'
		    << "other:" << '\n'
		    << HexPointUtil::ToString(other.black) << '\n'
		    << HexPointUtil::ToString(other.white) << '\n';
	abort();
    } 
}

//----------------------------------------------------------------------------

int SolvedState::PackedSize() const
{
    return (sizeof(win) + 
            sizeof(flags) +
            sizeof(numstates) +
            sizeof(nummoves) +
            sizeof(bestmove));
}

/** @bug NOT THREADSAFE! */
byte* SolvedState::Pack() const
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

void SolvedState::Unpack(const byte* data)
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

//----------------------------------------------------------------------------
