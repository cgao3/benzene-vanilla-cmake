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
    CheckCollision(other.hash, other.black, other.white);
}

void SolvedState::CheckCollision(hash_t hash, const bitset_t& black,
                                 const bitset_t& white) const
{
    if (this->hash == hash && (this->black != black || this->white != white))
    {
        LogSevere() << "HASH COLLISION!" << '\n'
		    << "this:" << '\n'
		    << HashUtil::toString(this->hash) << '\n'
		    << HexPointUtil::ToPointListString(this->black) << '\n'
		    << HexPointUtil::ToPointListString(this->white) << '\n'
		    << "other:" << '\n'
		    << HashUtil::toString(hash) << '\n'
		    << HexPointUtil::ToPointListString(black) << '\n'
		    << HexPointUtil::ToPointListString(white) << '\n';
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
