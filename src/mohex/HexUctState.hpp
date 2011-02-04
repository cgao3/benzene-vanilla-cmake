//----------------------------------------------------------------------------
/** @file HexUctState.hpp */
//----------------------------------------------------------------------------

#ifndef HEXUCTSTATE_HPP
#define HEXUCTSTATE_HPP

#include "SgBlackWhite.h"
#include "SgPoint.h"
#include "SgUctSearch.h"

#include "HashMap.hpp"
#include "HexBoard.hpp"
#include "HexState.hpp"
#include "HexUctKnowledge.hpp"
#include "Move.hpp"
#include "VC.hpp"

#include <boost/scoped_ptr.hpp>

_BEGIN_BENZENE_NAMESPACE_

class HexUctSearch;

//----------------------------------------------------------------------------

/** Black and white stones. 
    This appears to be a pointless class since all the functionality
    is also in StoneBoard. However, StoneBoard hash extra information
    (like the hash and some colorset information) that would be a waste
    of space. So we use this for now. 
 */
struct HexUctStoneData
{
    bitset_t black;

    bitset_t white;

    bitset_t played;

    /** Creates empty stone set. */
    HexUctStoneData();

    /** Copies stones from board. */
    HexUctStoneData(const StoneBoard& brd);

    /** Returns true only if all three sets are equal. */
    bool operator==(const HexUctStoneData& other) const;
};

inline HexUctStoneData::HexUctStoneData()
{
}

inline HexUctStoneData::HexUctStoneData(const StoneBoard& brd)
    : black(brd.GetBlack()),
      white(brd.GetWhite()),
      played(brd.GetPlayed())
{
}

inline bool HexUctStoneData::operator==(const HexUctStoneData& other) const
{
    return black == other.black 
        && white == other.white
        && played == other.played;
}

//----------------------------------------------------------------------------

/** Data shared among all threads. */
struct HexUctSharedData
{
    /** Moves from begining of game leading to this position. */
    MoveSequence gameSequence;

    /** State at root of search. */
    HexState rootState;

    /** Set of moves to consider from the root. */
    bitset_t rootConsider;

    /** Stores fillin information for states in the tree. */
    HashMap<HexUctStoneData> stones;

    explicit HexUctSharedData(int fillinMapBits);
};

inline HexUctSharedData::HexUctSharedData(int fillinMapBits)
    : stones(fillinMapBits)
{ 
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
    virtual HexPoint GenerateMove(PatternState& pastate, HexColor color, 
                                  HexPoint lastMove) = 0;

    virtual void InitializeForRollout(const StoneBoard& brd) = 0;

    virtual void InitializeForSearch() = 0;
};

//----------------------------------------------------------------------------

/** Thread state for HexUctSearch. */
class HexUctState : public SgUctThreadState
{
public: 
    /** Constructor.
        @param threadId The number of the thread. Needed for passing to
        constructor of SgUctThreadState.
        @param sch Parent Search object.
        @param treeUpdateRadius Pattern matching radius in tree.
        @param playoutUpdateRadius Pattern matching radius in playouts.
    */
    HexUctState(const unsigned int threadId, HexUctSearch& sch,
                int treeUpdateRadius, int playoutUpdateRadius);

    virtual ~HexUctState();

    //-----------------------------------------------------------------------

    /** @name Pure virtual functions of SgUctThreadState */
    // @{

    SgUctValue Evaluate();
    
    void Execute(SgMove move);

    void ExecutePlayout(SgMove move);
   
    bool GenerateAllMoves(SgUctValue count, std::vector<SgUctMoveInfo>& moves,
                          SgUctProvenType& provenType);

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

    const StoneBoard& Board() const;

    const PatternState& GetPatternState() const;

    HexUctSearchPolicy* Policy();

    bool IsInPlayout() const;

    std::string Dump() const;

    /** Sets policy (takes control of pointer) and deletes the old
        one, if it existed. */
    void SetPolicy(HexUctSearchPolicy* policy);
    
    HexPoint GetLastMovePlayed() const;

    HexColor GetColorToPlay() const;

private:
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

    boost::scoped_ptr<HexState> m_state;

    /** Board used during search. */
    boost::scoped_ptr<PatternState> m_pastate;

    /** Board used to compute knowledge. */
    boost::scoped_ptr<HexBoard> m_vcBrd;

    /** Playout policy. */
    boost::scoped_ptr<HexUctSearchPolicy> m_policy;

    /** Data shared between threads. */
    HexUctSharedData* m_sharedData;

    HexUctKnowledge m_knowledge;

    /** Parent search object. */
    HexUctSearch& m_search;
   
    /** See HexUctSearch::TreeUpdateRadius() */
    int m_treeUpdateRadius;

    /** See HexUctSearch::PlayoutUpdateRadius() */
    int m_playoutUpdateRadius;

    /** True if in playout phase. */
    bool m_isInPlayout;

    /** Moves played the game plus moves in tree. */
    MoveSequence m_gameSequence;

    /** Keeps track of last playout move made.
	Used for pattern-generated rollouts when call
	HexUctSearchPolicy. */
    HexPoint m_lastMovePlayed;

    /** True at the start of a game until the first move is played. */
    bool m_atRoot;

    bitset_t ComputeKnowledge(SgUctProvenType& provenType);

    void ExecuteMove(HexPoint cell, int updateRadius);
};

inline const StoneBoard& HexUctState::Board() const
{
    return m_state->Position();
}

inline const PatternState& HexUctState::GetPatternState() const
{
    return *m_pastate;
}

inline HexUctSearchPolicy* HexUctState::Policy()
{
    return m_policy.get();
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
    return m_state->ToPlay();
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // HEXUCTSTATE_HPP
