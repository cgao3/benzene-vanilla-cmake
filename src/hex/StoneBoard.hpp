//----------------------------------------------------------------------------
/** @file StoneBoard.hpp */
//----------------------------------------------------------------------------

#ifndef STONEBOARD_H
#define STONEBOARD_H

#include "Hex.hpp"
#include "ConstBoard.hpp"
#include "ZobristHash.hpp"

_BEGIN_BENZENE_NAMESPACE_

//---------------------------------------------------------------------------

/** Packed representation of a board-state. Useful for storing
    board positions in databases, opening books, etc. */
typedef std::vector<byte> BoardID;

//---------------------------------------------------------------------------

/** Tracks played stone information.

    Each cell on the board is assigned a HexColor, and so every cell
    is either EMPTY, BLACK, or WHITE.

    Each cell is also marked as 'played' or 'unplayed'. A cell should
    be marked as played when it corresponds to an actual move played
    in a game; that is, not a fill-in move. This means it is possible
    for a cell to be BLACK or WHITE and not played.  Played stones
    contribute to the board hash and id, unplayed stones do not.     
    @see Hash(), GetBoardID(), setPlayed(). 
*/
class StoneBoard
{
public:

    /** Constructs an unusable board. This is to allow StoneBoard to
        appear in a stl container. */
    StoneBoard();

    /** Constructs a square board. */
    explicit StoneBoard(unsigned size);

    /** Constructs a rectangular board. */
    StoneBoard(unsigned width, unsigned height);

    /** Contructs a rectangular board with given state.
        See SetPosition(std::string) */
    StoneBoard(unsigned width, unsigned height, const std::string& str);

    ~StoneBoard();

    //-----------------------------------------------------------------------

    /** Returns reference to ConstBoard. */
    const ConstBoard& Const() const;

    /** Same as Const().width() */
    int Width() const;

    /** Same as Const().height() */
    int Height() const;

    /** Returns zobrist hash for the current position, which depends
        only on the played cells; unplayed cells do not contribute to
        the hash. Changing the color of an unplayed cell does not
        change the hash for the position. Methods that change the
        color of played cells internally will always compute a new
        hash automatically. */
    SgHashCode Hash() const;

    /** Returns the hash of the position taking the color to play into
        account. This will be different than the hash returned by
        Hash(). */
    SgHashCode Hash(HexColor toPlay) const;

    /** Returns BoardID for the current board state, looking only at
        the played cells. */
    BoardID GetBoardID() const;

    /** Returns BoardID as a string. 
        @todo Moves this out of here? */
    std::string GetBoardIDString() const;

    //-----------------------------------------------------------------------

    /** Number of played stones on the interior of the board. 
        Similar to:
        @code
            num bits = (GetOccupied() & GetPlayed() & GetCells()).count();
        @endcode
    */
    std::size_t NumStones() const;

    /** Computes whose turn it is on the given board;
        IsStandardPosition() must be true to use this method. */
    HexColor WhoseTurn() const;

    /** Returns true if position is attainable in a normal game.
        That is, FIRST_TO_PLAY went first, and the colors alternated
        afterwards. */
    bool IsStandardPosition() const;

    //-----------------------------------------------------------------------

    /** Returns a string representation of the board. */
    std::string Write() const;

    /** Returns a string representation of the board with the cells
        marked in the given bitset denoted by a '*'. */
    std::string Write(const bitset_t& b) const;

    //-----------------------------------------------------------------------

    /** Sets the board to the state encoded by id. 
        Note this state will have no unplayed stones, so the code:
        @code
            brd.SetPosition(brd.GetBoardID());
        @endcode
        will remove all unplayed stones.
    */
    void SetPosition(const BoardID& id);

    /** Copies state of brd into this board. */
    void SetPosition(const StoneBoard& brd);

    /** Copies only the played state of brd. */
    void SetPositionOnlyPlayed(const StoneBoard& brd);

    /** Sets state from string.
        String must contain wxh non space characters. Any spacing is
        allowed betwen such characters. A '.' is an empty cell, 'B'
        and 'W' are played black and white stones, 'b' and 'w' are
        filled-in black and white stones. 
    */
    void SetPosition(const std::string& str);

    //-----------------------------------------------------------------------

    /** @name Methods defined on cells and edges. 
    
        All methods accept and return edge and interior cells only.
        Note that PlayMove() (which is not in this section) can play a
        move like (BLACK, RESIGN), but getBlack() (which is in this
        section) will not return a bitset with the RESIGN move set.

        @todo Why did we agree on this? :)
    */
    // @{
   
    /** Returns the set of BLACK stones. */
    bitset_t GetBlack() const;
    
    /** Returns the set of WHITE stones. */
    bitset_t GetWhite() const;

    /** Returns color's stones. */
    bitset_t GetColor(HexColor color) const;

    /** Returns all empty cells. */
    bitset_t GetEmpty() const;

    /** Returns all occupied (not empty) cells. */
    bitset_t GetOccupied() const;

    /** Returns true if cell is of color. */
    bool IsColor(HexPoint cell, HexColor color) const;

    /** Returns true if cell is empty. */
    bool IsEmpty(HexPoint cell) const;

    /** Returns true if cell is occupied (not empty). */
    bool IsOccupied(HexPoint cell) const;

    // @}

    //-----------------------------------------------------------------------

    /** @name Methods defined on all valid moves. */
    // @{ 

    /** Returns true if cell is BLACK. */
    bool IsBlack(HexPoint cell) const;

    /** Returns true if cell is WHITE. */
    bool IsWhite(HexPoint cell) const;

    /** Retruns color of cell. */
    HexColor GetColor(HexPoint cell) const;

    /** Returns the set of played cells. */
    bitset_t GetPlayed() const;
    bitset_t GetPlayed(HexColor color) const;

    /** Returns the set of all legal moves; ie, moves that can be
        played from this state. */
    bitset_t GetLegal() const;

    /** Returns true if cell has been played. */
    bool IsPlayed(HexPoint cell) const;

    /** Returns true if cell is a legal move. */
    bool IsLegal(HexPoint cell) const;

    // @}
    
    //-----------------------------------------------------------------------

    /** @name Methods not modifying Hash() or BoardID() */
    // @{

    /** Adds the cells in b as stones of color. Does not modify
        hash. */
    void AddColor(HexColor color, const bitset_t& b);

    /** Sets cells in b to EMPTY. Does not modify hash.  */
    void RemoveColor(HexColor color, const bitset_t& b);

    /** Sets color of cell. Does not modify hash. */
    void SetColor(HexColor color, HexPoint cell);

    /** Sets color of cells in bitset. Does not modify hash. */
    void SetColor(HexColor color, const bitset_t& bs);

    /** Returns true if rotating the board returns the same board. */
    bool IsSelfRotation() const;
    
    // @}

    //-----------------------------------------------------------------------

    /** @name Methods modifying Hash() and BoardID() */
    // @{

    /** Clears the board and plays the edge stones. */
    void StartNewGame();

    /** Sets the played stones. These stones, and only these stones,
        will contribute to the board hash and board id. Hash is
        recomputed.  @see Hash(). */
    void SetPlayed(const bitset_t& p);

    /** Plays a move of the given color to the board. Adds cell to the
        set of played stones. Updates the board hash. 
        @param color must BLACK or WHITE 
        @param cell Any valid move, include RESIGN and SWAP_PIECES.
    */
    void PlayMove(HexColor color, HexPoint cell);

    /** Removes the move from the board, setting cell to EMPTY. Hash
        is updated. */
    void UndoMove(HexPoint cell);

    /** Rotates the board by 180' about the center. Hash is
        updated. */
    void RotateBoard();

    /** Mirrors the board in the diagonal joining acute corners. Note
	that this method requires the board to be square. Hash is
	updated. */
    void MirrorBoard();

    // @}

    //-----------------------------------------------------------------------

    /** @name Operators */
    // @{

    /** Two boards are equal if their dimensions match and their sets
        of black, white, and played stones are all equal. */
    bool operator==(const StoneBoard& other) const;

    /** Returns true if the boards differ. See operator==().  */
    bool operator!=(const StoneBoard& other) const;

    // @}

    //-----------------------------------------------------------------------

    /** @name Iterators */
    // @{

    /** Returns iterator over all stones in colorset. */
    BoardIterator Stones(HexColorSet colorset) const;

    /** Returns iterator over all stones of color. */
    BoardIterator Stones(HexColor color) const;

    // @}

    //-----------------------------------------------------------------------

private:
    ConstBoard* m_const;

    bitset_t m_played;

    bitset_t m_stones[BLACK_AND_WHITE];

    /** @see Hash() */
    ZobristHash m_hash;

    mutable bool m_stones_calculated;

    mutable std::vector<HexPoint> m_stones_list[NUM_COLOR_SETS];

    void ComputeHash();

    void MarkAsDirty();

    bool IsBlackWhiteDisjoint();
};

inline const ConstBoard& StoneBoard::Const() const
{
    return *m_const;
}

inline int StoneBoard::Width() const
{
    return m_const->Width();
}

inline int StoneBoard::Height() const
{
    return m_const->Height();
}

inline SgHashCode StoneBoard::Hash() const
{
    return m_hash.Hash(EMPTY);
}

inline SgHashCode StoneBoard::Hash(HexColor toPlay) const
{
    return m_hash.Hash(toPlay);
}

inline bitset_t StoneBoard::GetBlack() const
{
    return m_stones[BLACK] & Const().GetLocations();
}

inline bitset_t StoneBoard::GetWhite() const
{
    return m_stones[WHITE] & Const().GetLocations();
}

inline bitset_t StoneBoard::GetColor(HexColor color) const
{
    BenzeneAssert(HexColorUtil::isValidColor(color));
    if (color == EMPTY) return GetEmpty();
    return m_stones[color] & Const().GetLocations();
}

inline bitset_t StoneBoard::GetEmpty() const
{
    return Const().GetLocations() - GetOccupied();
}

inline bitset_t StoneBoard::GetOccupied() const
{
    return (GetBlack() | GetWhite()) & Const().GetLocations();
}

inline bool StoneBoard::IsBlack(HexPoint cell) const    
{
    BenzeneAssert(Const().IsValid(cell));
    return m_stones[BLACK].test(cell);
}

inline bool StoneBoard::IsWhite(HexPoint cell) const    
{
    BenzeneAssert(Const().IsValid(cell));
    return m_stones[WHITE].test(cell);
}

inline bool StoneBoard::IsColor(HexPoint cell, HexColor color) const
{
    BenzeneAssert(HexColorUtil::isBlackWhite(color));
    BenzeneAssert(Const().IsLocation(cell));
    return m_stones[color].test(cell);
}

inline bool StoneBoard::IsEmpty(HexPoint cell) const
{
    BenzeneAssert(Const().IsLocation(cell));
    return !IsOccupied(cell);
}

inline bool StoneBoard::IsOccupied(HexPoint cell) const
{
    BenzeneAssert(Const().IsLocation(cell));
    return (IsBlack(cell) || IsWhite(cell));
}

inline bitset_t StoneBoard::GetPlayed() const 
{
    return m_played;
}

inline bitset_t StoneBoard::GetPlayed(HexColor color) const 
{
    BenzeneAssert(HexColorUtil::isBlackWhite(color));
    return m_played & GetColor(color);
}

inline bool StoneBoard::IsPlayed(HexPoint cell) const
{
    BenzeneAssert(Const().IsValid(cell));
    return m_played.test(cell);
}

inline std::size_t StoneBoard::NumStones() const
{
    return (GetOccupied() & GetPlayed() & Const().GetCells()).count();
}

inline bool StoneBoard::operator==(const StoneBoard& other) const
{
    return (m_const == other.m_const
            && m_stones[BLACK] == other.m_stones[BLACK]
            && m_stones[WHITE] == other.m_stones[WHITE]
            && m_played == other.m_played);
}

inline bool StoneBoard::operator!=(const StoneBoard& other) const
{
    return !(operator==(other));
}

inline HexColor StoneBoard::WhoseTurn() const
{
    BenzeneAssert(IsStandardPosition());
    bitset_t mask = GetPlayed() & Const().GetCells();
    std::size_t first = (GetColor(FIRST_TO_PLAY) & mask).count();
    std::size_t second = (GetColor(!FIRST_TO_PLAY) & mask).count();
    return (first > second) ? !FIRST_TO_PLAY : FIRST_TO_PLAY;
}

inline bool StoneBoard::IsStandardPosition() const
{
    std::size_t diff = GetPlayed(BLACK).count() - GetPlayed(WHITE).count();
    return (diff == 1) || (diff == 0);
}

inline BoardIterator StoneBoard::Stones(HexColor color) const
{
    return Stones(HexColorSetUtil::Only(color));
}

/** Prints board to output stream. */
inline std::ostream& operator<<(std::ostream &os, const StoneBoard& b)
{
    os << b.Write();
    return os;
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // STONEBOARD_HPP
