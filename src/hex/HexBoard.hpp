//----------------------------------------------------------------------------
// $Id: HexBoard.hpp 1940 2009-03-07 01:52:15Z broderic $
//----------------------------------------------------------------------------

#ifndef HEXBOARD_H
#define HEXBOARD_H

#include <boost/scoped_ptr.hpp>

#include "ChangeLog.hpp"
#include "ConnectionBuilder.hpp"
#include "ICEngine.hpp"
#include "PatternBoard.hpp"
#include "VCPattern.hpp"

//----------------------------------------------------------------------------

/** Combines GroupBoard, PatternBoard, and Connections into a board
    that handles all updates automatically.
  
    @todo Document me!
*/
class HexBoard : public PatternBoard
{
public:
    
    /** Creates a rectangular board. */
    HexBoard(int width, int size, const ICEngine& ice,
             ConnectionBuilderParam& param);

    /** Copy constructor. */
    HexBoard(const HexBoard& other);

    /** Destructor. */
    ~HexBoard();

    //-----------------------------------------------------------------------

    /** @name Parameters */
    // @{

    /** Whether VCs are computed or not. */
    bool UseVCs() const;

    /** See UseVCs() */
    void SetUseVCs(bool enable);

    /** Whether ICE is used. */
    bool UseICE() const;

    /** See UseICE() */
    void SetUseICE(bool enable);

    /** Whether decompositions are found and filled-in. */
    bool UseDecompositions() const;

    /** See UseDecompositions() */
    void SetUseDecompositions(bool enable);

    /** Whether ICE info is backed-up in UndoMove(). */
    bool BackupIceInfo() const;

    /** See BackupIceInfo() */
    void SetBackupIceInfo(bool enable);

    // @}

    //-----------------------------------------------------------------------

    /** Returns the vc mustplay for color (ie, the intersection of all
        of the other color's winning semi-connections). */
    bitset_t getMustplay(HexColor color) const;

    /** Copies state of stoneboard into this board. */
    void SetState(const StoneBoard& brd);

    //-----------------------------------------------------------------------

    /** Controls how to deal with endgames caused by fill-in. */
    typedef enum 
    { 
        /** All of the winning player's fill-in is removed if the fill-in
            creates a solid winning chain. */
        REMOVE_WINNING_FILLIN,

        /** Winning fill-in is not removed. */
        DO_NOT_REMOVE_WINNING_FILLIN 

    } EndgameFillin;

    /** Clears history.  Computes dead/vcs for current state. */
    virtual void ComputeAll(HexColor color, EndgameFillin endgame_mode);

    /** Stores old state on stack, plays move to board, updates
        ics/vcs.  Hash is modified by the move.  Allows ice info to
        be backed-up. */
    virtual void PlayMove(HexColor color, HexPoint cell);
    
    /** Stores old state on stack, plays set of stones, updates
        ics/vcs. HASH IS NOT MODIFIED! No ice info will be backed up,
        but this set of moves can be reverted with a single call to
        UndoMove(). */
    virtual void PlayStones(HexColor color, const bitset_t& played,
                            HexColor color_to_move);
        
    /** Adds stones for color to board with color_to_move about to
        play next; added stones must be a subset of the empty cells.
        Does not affect the hash of this state. State is not pushed
        onto stack, so a call to UndoMove() will undo these changes
        along with the last changes that changed the stack. */
    virtual void AddStones(HexColor color, const bitset_t& played,
                           HexColor color_to_move, 
                           EndgameFillin endgame_mode);

    /** Reverts to last state stored on the stack, restoring all state
        info. If the option is on, also backs up inferior cell
        info. */
    virtual void UndoMove();

    //-----------------------------------------------------------------------

    /** Returns domination arcs added from backing-up. */
    const std::set<HexPointPair>& GetBackedUp() const;

    /** Adds domination arcs in dom to set of inferior cells for this
        state. */
    void AddDominationArcs(const std::set<HexPointPair>& dom);

    //-----------------------------------------------------------------------

    /** Returns the set of dead cells on the board. This is the union
        of all cells found dead previously during the history of moves
        since the last ComputeAll() call.  */
    bitset_t getDead() const;
    
    /** Returns the set of inferior cell. */
    const InferiorCells& getInferiorCells() const;

    /** Returns the Inferior Cell Engine the board is using. */
    const ICEngine& ICE() const;

    /** Returns the connection set for color. */
    const Connections& Cons(HexColor color) const;

    /** Returns the connection set for color. */
    Connections& Cons(HexColor color);

    /** Returns the connection builder for this board. */
    ConnectionBuilder& Builder();

    /** Returns the connection builder for this board. */
    const ConnectionBuilder& Builder() const;

private:

    void Initialize();

    /** No assignments allowed! Use the copy constructor if you must
        make a copy, but you shouldn't be copying boards around very
        often. */
    void operator=(const HexBoard& other);

    void ComputeInferiorCells(HexColor color_to_move, 
                              EndgameFillin endgame_mode);

    void BuildVCs();

    void BuildVCs(bitset_t added[BLACK_AND_WHITE], bool mark_the_log = true);

    void RevertVCs();

    /** In non-terminal states, checks for combinatorial decomposition
        with a vc using FindCombinatorialDecomposition(). Plays the carrier
	using AddStones(). Loops until no more decompositions are found. */
    void HandleVCDecomposition(HexColor color_to_move, 
                               EndgameFillin endgame_mode);

    void ClearHistory();

    void PushHistory(HexColor color, HexPoint cell);

    void PopHistory();

    //-----------------------------------------------------------------------

    struct History
    {
        /** Saved board state. */
        StoneBoard board;

        /** The inferior cell data for this state. */
        InferiorCells inf;

        /** Domination arcs added from backing-up. */
        std::set<HexPointPair> backedup;

        /** Color to play from this state. */
        HexColor to_play;
        
        /** Move last played from this state. */
        HexPoint last_played;

        History(const StoneBoard& b, const InferiorCells& i, 
                const std::set<HexPointPair>& back, HexColor tp, HexPoint lp)
            : board(b), inf(i), backedup(back), to_play(tp), last_played(lp) 
        { }
    };

    //-----------------------------------------------------------------------

    /** @name Member variables. 
        
        @warning If you change anything here, be sure to 
        update the copy constructor!!
    */
    // @{

    /** ICEngine used to compute inferior cells. */
    const ICEngine* m_ice;

    /** Builder used to compute virtual connections. */
    ConnectionBuilder m_builder;

    /** Connection sets for black and white. */
    boost::scoped_ptr<Connections> m_cons[BLACK_AND_WHITE];

    /** The vc changelogs for both black and white. */
    ChangeLog<VC> m_log[BLACK_AND_WHITE];

    /** History stack. */
    std::vector<History> m_history;

    /** The set of inferior cells for the current boardstate. */
    InferiorCells m_inf;

    /** Domination arcs added from backing-up ic info from moves
        played from this state. */
    std::set<HexPointPair> m_backedup;

    /** See UseVCs() */
    bool m_use_vcs;

    /** See UseICE() */
    bool m_use_ice;

    /** See UseDecompositions() */
    bool m_use_decompositions;

    /** See BackupIceInfo() */
    bool m_backup_ice_info;

    // @}
};

inline bitset_t HexBoard::getDead() const
{
    return m_inf.Dead();
}

inline const InferiorCells& HexBoard::getInferiorCells() const
{
    return m_inf;
}

inline const std::set<HexPointPair>& HexBoard::GetBackedUp() const
{
    return m_backedup;
}

inline const ICEngine& HexBoard::ICE() const
{
    return *m_ice;
}

inline const Connections& HexBoard::Cons(HexColor color) const
{
    return *m_cons[color].get();
}

inline Connections& HexBoard::Cons(HexColor color)
{
    return *m_cons[color].get();
}

inline ConnectionBuilder& HexBoard::Builder()
{
    return m_builder;
}

inline const ConnectionBuilder& HexBoard::Builder() const
{
    return m_builder;
}

inline bool HexBoard::UseVCs() const
{
    return m_use_vcs;
}

inline void HexBoard::SetUseVCs(bool enable)
{
    m_use_vcs = enable;
}

inline bool HexBoard::UseICE() const
{
    return m_use_ice;
}

inline void HexBoard::SetUseICE(bool enable)
{
    m_use_ice = enable;
}

inline bool HexBoard::UseDecompositions() const
{
    return m_use_decompositions;
}

inline void HexBoard::SetUseDecompositions(bool enable)
{
    m_use_decompositions = enable;
}

inline bool HexBoard::BackupIceInfo() const
{
    return m_backup_ice_info;
}

inline void HexBoard::SetBackupIceInfo(bool enable)
{
    m_backup_ice_info = enable;
}

//----------------------------------------------------------------------------

#endif // HEXBOARD_H
