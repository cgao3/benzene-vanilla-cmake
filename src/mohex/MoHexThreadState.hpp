//----------------------------------------------------------------------------
/** @file MoHexThreadState.hpp */
//----------------------------------------------------------------------------

#ifndef MOHEXTHREADSTATE_HPP
#define MOHEXTHREADSTATE_HPP

#include "SgBlackWhite.h"
#include "SgPoint.h"
#include "SgUctSearch.h"

#include "HashMap.hpp"
#include "HexBoard.hpp"
#include "HexState.hpp"
#include "MoHexPriorKnowledge.hpp"
#include "Move.hpp"
#include "VC.hpp"

#include <boost/scoped_ptr.hpp>

_BEGIN_BENZENE_NAMESPACE_

class MoHexSearch;

//----------------------------------------------------------------------------

/** Data shared among all threads. */
struct MoHexSharedData
{
    /** Moves from begining of game leading to this position. */
    MoveSequence gameSequence;

    /** State at root of search. */
    HexState rootState;

    /** Set of moves to consider from the root. */
    bitset_t rootConsider;

    /** Stores fillin information for states in the tree. */
    HashMap<StoneBoard> stones;

    explicit MoHexSharedData(int fillinMapBits);
};

inline MoHexSharedData::MoHexSharedData(int fillinMapBits)
    : stones(fillinMapBits)
{ 
}

//----------------------------------------------------------------------------

/** Interface for policies controlling move generation in the random
    play-out phase of MoHexSearch. */
class MoHexSearchPolicy
{
public:
    virtual ~MoHexSearchPolicy() { };

    /** Generates a move in the random playout phase of
        MoHexSearch. */
    virtual HexPoint GenerateMove(PatternState& pastate, HexColor color, 
                                  HexPoint lastMove) = 0;

    virtual void InitializeForPlayout(const StoneBoard& brd) = 0;

    virtual void InitializeForSearch() = 0;
};

//----------------------------------------------------------------------------

/** Thread state for MoHexSearch. */
class MoHexThreadState : public SgUctThreadState
{
public: 
    /** Constructor.
        @param threadId The number of the thread. Needed for passing to
        constructor of SgUctThreadState.
        @param sch Parent Search object.
        @param treeUpdateRadius Pattern matching radius in tree.
        @param playoutUpdateRadius Pattern matching radius in playouts.
    */
    MoHexThreadState(const unsigned int threadId, MoHexSearch& sch,
                int treeUpdateRadius, int playoutUpdateRadius);

    virtual ~MoHexThreadState();

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

    MoHexSearchPolicy* Policy();

    bool IsInPlayout() const;

    std::string Dump() const;

    /** Sets policy (takes control of pointer) and deletes the old
        one, if it existed. */
    void SetPolicy(MoHexSearchPolicy* policy);
    
    HexPoint GetLastMovePlayed() const;

    HexColor GetColorToPlay() const;

private:
    /** Assertion handler to dump the state of a MoHexThreadState. */
    class AssertionHandler
        : public SgAssertionHandler
    {
    public:
        AssertionHandler(const MoHexThreadState& state);

        void Run();

    private:
        const MoHexThreadState& m_state;
    };

    AssertionHandler m_assertionHandler;

    boost::scoped_ptr<HexState> m_state;

    boost::scoped_ptr<HexState> m_playoutStartState;

    /** Board used during search. */
    boost::scoped_ptr<PatternState> m_pastate;

    boost::scoped_ptr<PatternState> m_playoutStartPatterns;

    /** Board used to compute knowledge. */
    boost::scoped_ptr<HexBoard> m_vcBrd;

    /** Playout policy. */
    boost::scoped_ptr<MoHexSearchPolicy> m_policy;

    /** Data shared between threads. */
    MoHexSharedData* m_sharedData;

    MoHexPriorKnowledge m_priorKnowledge;

    /** Parent search object. */
    MoHexSearch& m_search;
   
    /** See MoHexSearch::TreeUpdateRadius() */
    int m_treeUpdateRadius;

    /** See MoHexSearch::PlayoutUpdateRadius() */
    int m_playoutUpdateRadius;

    /** True if in playout phase. */
    bool m_isInPlayout;

    /** Moves played the game plus moves in tree. */
    MoveSequence m_gameSequence;

    /** Keeps track of last playout move made.
	Used for pattern-generated rollouts when call
	MoHexSearchPolicy. */
    HexPoint m_lastMovePlayed;

    HexPoint m_playoutStartLastMove;

    /** True at the start of a game until the first move is played. */
    bool m_atRoot;

    bool m_usingKnowledge;

    bitset_t ComputeKnowledge(SgUctProvenType& provenType);

    void ExecuteMove(HexPoint cell, int updateRadius);
};

inline const StoneBoard& MoHexThreadState::Board() const
{
    return m_state->Position();
}

inline const PatternState& MoHexThreadState::GetPatternState() const
{
    return *m_pastate;
}

inline MoHexSearchPolicy* MoHexThreadState::Policy()
{
    return m_policy.get();
}

inline bool MoHexThreadState::IsInPlayout() const
{
    return m_isInPlayout;
}

inline HexPoint MoHexThreadState::GetLastMovePlayed() const
{
    return m_lastMovePlayed;
}

inline HexColor MoHexThreadState::GetColorToPlay() const
{
    return m_state->ToPlay();
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // MOHEXTHREADSTATE_HPP
