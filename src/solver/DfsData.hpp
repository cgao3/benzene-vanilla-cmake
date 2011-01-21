//----------------------------------------------------------------------------
/** @file DfsData.hpp */
//----------------------------------------------------------------------------

#ifndef DFSDATA_HPP
#define DFSDATA_HPP

#include "ConstBoard.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** State solved by DfsSolver. 
    Stored in a DfsHashTable or DfsStates database. Do not forget to
    update DFS_DB_VERSION if this class changes in a way that
    invalidiates old databases. */
struct DfsData
{
    bool m_isValid;

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
    DfsData(bool win, int numStates, int numMoves, HexPoint bestMove);

    /** @name SgHashTable methods. */
    // @{
    
    bool IsValid() const;

    void Invalidate();

    /** Always returns true.
        @todo Try other replacement policies?
     */
    bool IsBetterThan(const DfsData& other) const;
    
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
    : m_isValid(false)
{ 
}
    
inline DfsData::DfsData(bool win, int numStates, int numMoves, 
                        HexPoint bestMove)
    : m_isValid(true),
      m_win(win),
      m_flags(0),
      m_numStates(numStates),
      m_numMoves(numMoves),
      m_bestMove(bestMove)
{ 
}

inline bool DfsData::IsValid() const
{
    return m_isValid;
}

inline void DfsData::Invalidate()
{
    m_isValid = false;
}

inline bool DfsData::IsBetterThan(const DfsData& other) const
{
    UNUSED(other);
    return true;
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // DFSDATA_HPP
