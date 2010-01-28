//----------------------------------------------------------------------------
/** @file DfsData.hpp
 */
//----------------------------------------------------------------------------

#ifndef DFSDATA_H
#define DFSDATA_H

#include "Hex.hpp"
#include "ConstBoard.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** A solved state. Stored in a TT or DB. Matches
    TransTableStateConcept.
    Do not forget to update DFS_DB_VERSION if this class changes in a
    way that invalidiates old databases.
*/
struct DfsData
{
    /** True if player to move wins. */
    bool m_win;

    /** Flags. */
    int m_flags;
    
    /** Number of states in proof-tree of this result. */
    int m_numStates;
    
    /** Number of moves losing player can delay until winning
        player has a winning virtual connection. */
    int m_numMoves;

    /** Best move in this state. 
        Very important in winning states, not so important in losing
        states. That is, in winning states this move *must* be a
        winning move, in losing states this move is "most blocking",
        but the definition is fuzzy. */
    HexPoint m_bestMove;

    //--------------------------------------------------------------------

    /** Contructs state with default values. */
    DfsData();
    
    /** Initializes state to given values. */
    DfsData(bool w, int nstates, int nmoves, HexPoint bmove);

    /** @name TransTableStateConcept */
    // @{
    
    /** Returns true if this state is not the same as that built by
        the default constructor. */
    bool Initialized() const;

    /** If true, then this will give up its TT slot to other.
        @note ALWAYS RETURNS TRUE FOR NOW!  */
    bool ReplaceWith(const DfsData& other) const;
    
    // @} 

    /** @name PositionDBStateConcept. */
    // @{

    int PackedSize() const;

    byte* Pack() const;

    void Unpack(const byte* t);

    void Rotate(const ConstBoard& brd);

    void Mirror(const ConstBoard& brd);

    // @}
};

inline DfsData::DfsData()
    : m_win(false), 
      m_flags(0), 
      m_numStates(0), 
      m_numMoves(0), 
      m_bestMove(INVALID_POINT)
{ 
}
    
inline DfsData::DfsData(bool w, int nstates, int nmoves, HexPoint bmove)
    : m_win(w),
      m_flags(0),
      m_numStates(nstates),
      m_numMoves(nmoves),
      m_bestMove(bmove)
{ 
}

inline bool DfsData::Initialized() const
{
    return m_bestMove != INVALID_POINT;
}

inline bool DfsData::ReplaceWith(const DfsData& other) const
{
    UNUSED(other);
    return true;
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // DFSDATA_H
