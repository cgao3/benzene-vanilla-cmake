//----------------------------------------------------------------------------
// $Id: HexUctState.hpp 1842 2009-01-18 00:05:04Z broderic $
//----------------------------------------------------------------------------

#ifndef HEXUCTSTATE_HPP
#define HEXUCTSTATE_HPP

#include "SgBlackWhite.h"
#include "SgPoint.h"
#include "SgUctSearch.h"

#include "PatternBoard.hpp"
#include "VC.hpp"

#include <boost/scoped_ptr.hpp>

class HexUctSearch;

//----------------------------------------------------------------------------

/** Data for the start of a new game; fill-in, mtcs, etc...  */
struct HexUctInitialData
{
    /** Boardstate at root node. */
    HexColor root_to_play;
    HexPoint root_last_move_played;
    bitset_t root_black_stones;
    bitset_t root_white_stones;

    /** Moves to consider at root node. */
    bitset_t ply1_moves_to_consider;
    
    /** Boardstate after each possible 1st-ply move. */
    PointToBitset ply1_black_stones;
    PointToBitset ply1_white_stones;
    
    /** Moves to consider for player on 2nd-ply. */
    PointToBitset ply2_moves_to_consider;

    /** Combines consider data for 1ply and 2ply moves; ie. the
        ply1_[color]_stones and ply2_moves_to_consider maps. */
    void Union(const HexUctInitialData& other);
};

inline void HexUctInitialData::Union(const HexUctInitialData& other)
{
    ply1_black_stones.insert(other.ply1_black_stones.begin(), 
                             other.ply1_black_stones.end());
    ply1_white_stones.insert(other.ply1_white_stones.begin(),
                             other.ply1_white_stones.end());
    ply2_moves_to_consider.insert(other.ply2_moves_to_consider.begin(),
                                  other.ply2_moves_to_consider.end());
}

//----------------------------------------------------------------------------

/** Interface for policies controlling move generation in the random
    play-out phase of HexUctSearch. */
class HexUctSearchPolicy
{
public:

    virtual ~HexUctSearchPolicy() { };

    /** Generate a move in the random play-out phase of
        HexUctSearch. */
    virtual HexPoint GenerateMove(PatternBoard& brd, HexColor color, 
                                  HexPoint lastMove) = 0;

    virtual void InitializeForRollout(const StoneBoard& brd) = 0;
};

//----------------------------------------------------------------------------

/** Thread state for HexUctSearch. */
class HexUctState : public SgUctThreadState
{
public: 

    /** Constructor.
        @param threadId The number of the thread. Needed for passing to
        constructor of SgUctThreadState.
        @param bd The board with the current position. The state has is own
        board that will be synchronized with the currently searched position
        in StartSearch()
    */
    HexUctState(std::size_t threadId,
		const HexUctSearch& sch,
                int treeUpdateRadius,
                int playoutUpdateRadius);

    virtual ~HexUctState();

    //-----------------------------------------------------------------------

    /** @name Pure virtual functions of SgUctThreadState */
    // @{

    float Evaluate();
    
    void Execute(SgMove move);

    void ExecutePlayout(SgMove move);
   
    void GenerateAllMoves(std::vector<SgMove>& moves);

    SgMove GeneratePlayoutMove(bool& skipRaveUpdate);
    
    void StartSearch();

    void TakeBackInTree(std::size_t nuMoves);

    void TakeBackPlayout(std::size_t nuMoves);

    SgBlackWhite ToPlay() const;

    // @} // @name

    //-----------------------------------------------------------------------

    /** @name Virtual functions of SgUctThreadState */
    // @{

    void GameStart();

    void StartPlayouts();

    virtual void StartPlayout();

    virtual void EndPlayout();

    // @} // @name

    //-----------------------------------------------------------------------

    const PatternBoard& Board() const;

    HexUctSearchPolicy* Policy();

    bool IsInPlayout() const;

    void Dump(std::ostream& out) const;

    /** Sets random policy (takes control of pointer) and deletes the
        old one, if it existed. */
    void SetPolicy(HexUctSearchPolicy* policy);
    
    HexPoint GetLastMovePlayed() const;

    HexColor GetColorToPlay() const;

private:

    /** Frees policy if one is assigned; does nothing otherwise. */
    void FreePolicy();
    
    /** Executes a move. */
    void ExecuteTreeMove(HexPoint move);
    void ExecuteRolloutMove(HexPoint move);
    void ExecutePlainMove(HexPoint cell, int updateRadius);

    /** Assertion handler to dump the state of a HexUctState. */
    class AssertionHandler
        : public SgAssertionHandler
    {
    public:
        AssertionHandler(const HexUctState& state);

        void Run();

    private:
        const HexUctState& m_state;
    };

    AssertionHandler m_assertionHandler;

    /** Board used during search. */
    boost::scoped_ptr<PatternBoard> m_bd;

    const HexUctInitialData* m_initial_data;

    const HexUctSearch& m_search;

    HexUctSearchPolicy* m_policy;
    
    /** Color to play next. */
    HexColor m_toPlay;

    /** @see HexUctSearch::TreeUpdateRadius() */
    int m_treeUpdateRadius;

    /** @see HexUctSearch::PlayoutUpdateRadius() */
    int m_playoutUpdateRadius;

    //----------------------------------------------------------------------

    /** True if in playout phase. */
    bool m_isInPlayout;

    /** Keeps track of last rollout move made
	Used for pattern-generated rollouts when call HexUctSearchPolicy
     */
    HexPoint m_lastMovePlayed;

    /** Number of stones played since initial board position */
    int m_numStonesPlayed;

    bool m_new_game;
};

inline const PatternBoard& HexUctState::Board() const
{
    return *m_bd;
}

inline HexUctSearchPolicy* HexUctState::Policy()
{
    return m_policy;
}

inline bool HexUctState::IsInPlayout() const
{
    return m_isInPlayout;
}

inline HexPoint HexUctState::GetLastMovePlayed() const
{
    return m_lastMovePlayed;
}

inline HexColor HexUctState::GetColorToPlay() const
{
    return m_toPlay;
}

//----------------------------------------------------------------------------

#endif // HEXUCTSTATE_HPP
