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
#include "MoHexBoard.hpp"
#include "MoHexPriorKnowledge.hpp"
#include "MoHexPlayoutPolicy.hpp"
#include "Move.hpp"
#include "VCS.hpp"

#include <boost/scoped_ptr.hpp>

_BEGIN_BENZENE_NAMESPACE_

class MoHexSearch;

//----------------------------------------------------------------------------

/** Data shared among all threads. */
struct MoHexSharedData
{
    struct StateData
    {
        MoHexBoard board;
        StoneBoard position;
        bitset_t consider;
    };

    struct TreeStatistics
    {
        std::size_t priorMoves;
        std::size_t priorMovesAfter;
        std::size_t priorPositions;
        std::size_t priorProven;
        std::size_t knowPositions;
        std::size_t knowProven;
        std::size_t knowMovesAfter;
        std::size_t vcmProbes;
        std::size_t vcmResponses;
        std::size_t vcmExpanded;
        TreeStatistics() : priorMoves(0), 
                           priorMovesAfter(0),
                           priorPositions(0),
                           priorProven(0),
                           knowPositions(0),
                           knowProven(0),
                           knowMovesAfter(0),
                           vcmProbes(0),
                           vcmResponses(0),
                           vcmExpanded(0)
        { }

        std::string ToString() const;
    };

    /** Moves from begining of game leading to this position. */
    MoveSequence gameSequence;

    /** State at root of search. */
    HexState rootState;

    /** Set of moves to consider from the root. */
    bitset_t rootConsider;

    MoHexBoard rootBoard;

    /** Stores fillin information for states in the tree. */
    HashMap<StateData> stateData;

    TreeStatistics treeStatistics;
    
    explicit MoHexSharedData(int fillinMapBits);
};

inline MoHexSharedData::MoHexSharedData(int fillinMapBits)
    : stateData(fillinMapBits)
{ 
}

//----------------------------------------------------------------------------

/** Thread state for MoHexSearch. */
class MoHexThreadState : public SgUctThreadState
{
public: 
    /** Constructor.
        @param threadId The number of the thread. Needed for passing to
        constructor of SgUctThreadState.
        @param sch Parent Search object. */
    MoHexThreadState(const unsigned int threadId, MoHexSearch& sch,
                     MoHexSharedPolicy* sharedPolicy);

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

    bool IsValidMove(SgMove move);

    void GameStart();

    void StartPlayouts();

    virtual void StartPlayout();

    virtual void EndPlayout();

    // @} // @name

    //-----------------------------------------------------------------------

    /** Used by MoHexEngine to play some playouts directly. */
    void StartPlayout(const HexState& state, HexPoint lastMovePlayed);

    const HexState& State() const;

    const MoHexBoard& GetMoHexBoard() const;

    MoHexSearch& Search();

    const MoHexSearch& Search() const;

    MoHexPlayoutPolicy& Policy();

    bool IsInPlayout() const;

    std::string Dump() const;

    HexPoint GetLastMovePlayed() const;

    HexColor ColorToPlay() const;

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

    /** Board used to compute knowledge. */
    boost::scoped_ptr<HexBoard> m_vcBrd;

    MoHexBoard m_board;

    MoHexBoard m_playoutStartBoard;

    /** Playout policy. */
    MoHexPlayoutPolicy m_policy;

    /** Data shared between threads. */
    MoHexSharedData* m_sharedData;

    MoHexPriorKnowledge m_priorKnowledge;

    /** Parent search object. */
    MoHexSearch& m_search;
   
    /** True if in playout phase. */
    bool m_isInPlayout;

    /** Keeps track of last playout move made.
	Used for pattern-generated rollouts when call
	MoHexPlayoutPolicy. */
    HexPoint m_lastMovePlayed;

    HexPoint m_playoutStartLastMove;

    /** True at the start of a game until the first move is played. */
    bool m_atRoot;

    bool m_usingKnowledge;

    HexColor m_toPlay;

    bitset_t ComputeKnowledge(SgUctProvenType& provenType);

    void AddTriangleFill(const HexPoint cell, const HexColor color);

    void DoVCMaintenanceInTree(const HexBoard& vcbrd, const bitset_t consider,
                               const HexColor toPlay);

};

inline const HexState& MoHexThreadState::State() const
{
    return *m_state;
}

inline MoHexSearch& MoHexThreadState::Search()
{
    return m_search;
}

inline const MoHexSearch& MoHexThreadState::Search() const
{
    return m_search;
}

inline MoHexPlayoutPolicy& MoHexThreadState::Policy()
{
    return m_policy;
}

inline bool MoHexThreadState::IsInPlayout() const
{
    return m_isInPlayout;
}

inline HexPoint MoHexThreadState::GetLastMovePlayed() const
{
    return m_lastMovePlayed;
}

inline HexColor MoHexThreadState::ColorToPlay() const
{
    return m_toPlay;
}

inline const MoHexBoard& MoHexThreadState::GetMoHexBoard() const
{
    return m_board;
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // MOHEXTHREADSTATE_HPP
