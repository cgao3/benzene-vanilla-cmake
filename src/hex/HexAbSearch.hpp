//----------------------------------------------------------------------------
/** @file
 */
//----------------------------------------------------------------------------

#ifndef HEXABSEARCH_HPP
#define HEXABSEARCH_HPP

#include "Hex.hpp"
#include "HexEval.hpp"
#include "SearchedState.hpp"
#include "TransTable.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

typedef TransTable<SearchedState> SearchTT;

//----------------------------------------------------------------------------

class HexBoard;

/** Base Alpha-Beta search class. */
class HexAbSearch
{
public:
    
    /** Constructor. */
    explicit HexAbSearch();

    /** Destructor. */
    virtual ~HexAbSearch();

    //------------------------------------------------------------------------

    /** Sets the transposition table to be used during search. */
    void SetTT(SearchTT* tt);

    /** Writes progress of search in guifx format after each root move
        completes. Off by default. */
    bool GuiFx() const;

    /** Sets whether guifx output should be dumped. 
        @see GuiFX(). */
    void SetGuiFx(bool flag);

    //------------------------------------------------------------------------

    /** Runs the AlphaBeta search. 

        @param brd       board to play one.
        @param color     color to play.
        @param plywidth  depth of the search set to plywidth.size()
                         and plywidth[j] top moves are explored.
        @param depths_to_search successive depths to search (like in ID).
        @param timelimit amount of time in which to finish search.
        @param PV        the principal variation will be stored here. 
        @returns         the evaluation of the PV.  

        If search is aborted by the user or the timelimit is reached,
        then the last valid result from interative deepening is
        returned.  If the first iteration has not completed, then a
        score of -EVAL_INFINITY and a PV containing only INVALID_POINT
        are returned.
    */
    HexEval Search(HexBoard& brd, HexColor color, 
                   const std::vector<int>& plywidth, 
                   const std::vector<int>& depths_to_search,
                   int timelimit,
                   std::vector<HexPoint>& PV);
   
    //------------------------------------------------------------------------

    /** Evaluates leaf position. */
    virtual HexEval Evaluate()=0;

    /** Generates moves for this position.  Moves will be played
        in the returned order. */
    virtual void GenerateMoves(std::vector<HexPoint>& moves)=0;

    /** Plays the given move. */
    virtual void ExecuteMove(HexPoint move)=0;

    /** Undoes the given move. */
    virtual void UndoMove(HexPoint move)=0;

    /** Hook function called upon entering new position. 
        Default implementation does nothing. */
    virtual void EnteredNewState();

    /** Hook function called at the very start of the search. 
        Default implementation does nothing. */
    virtual void OnStartSearch();
    
    /** Hook function called after the search has completed. 
        Default implementation does nothing. */
    virtual void OnSearchComplete();

    /** Hook function called after a states moves have been searched. 
        Default implementation does nothing. */
    virtual void AfterStateSearched();

    //------------------------------------------------------------------------

    /** Output stats from search. */
    std::string DumpStats();

protected:

    /** The board we are playing on. */
    HexBoard* m_brd;

    /** Color of player to move next. */
    HexColor m_toplay;

    /** Transposition table to use during search, if any. */
    SearchTT* m_tt;

    /** @see GuiFx(). */
    bool m_use_guifx;
        
    /** Number of moves from the root. */
    int m_current_depth;

    /** Sequences of moves to the current state. */
    PointSequence m_sequence;

    /** If current state exists in TT, but TT state was not deep
        enough, this will hold the best move for that state; otherwise
        it will be INVALID_MOVE.  Could be used in GenerateMoves() to
        improve move ordering when using iterative deepening. */
    HexPoint m_tt_bestmove;
    bool m_tt_info_available;

private:

    void ClearStats();

    void UpdatePV(int current_depth, HexEval value, std::vector<HexPoint>& cv);

    HexEval CheckTerminalState();

    /** Checks SgUserAbort() and sets m_aborted if true. */
    bool CheckAbort();

    HexEval SearchState(const std::vector<int>& plywidth,
                        int depth, HexEval alpha, HexEval beta,
                        std::vector<HexPoint>& cv);

    /** Search statistics. */
    struct Statistics
    {
        int numstates;
        int numleafs;
        int numterminal;
        int numinternal;
        int mustplay_branches;
        int total_branches;
        int visited_branches;
        int cuts;
        int tt_hits;
        int tt_cuts;
        double elapsed_time;
        HexEval value;
        std::vector<HexPoint> pv;

        Statistics()
            : numstates(0), numleafs(0), 
              numterminal(0), numinternal(0),
              mustplay_branches(0), total_branches(0),
              visited_branches(0), cuts(0), tt_hits(0),
              tt_cuts(0), elapsed_time(0.0), value(0)
        { };

        /** Prints statistics in human readable format. */
        std::string Dump() const;
    };

    Statistics m_statistics;

    /** Evaluations for each move from the root state. */
    std::vector<HexMoveValue> m_eval;

    /** True if the search was aborted due to timelimit or user
        intervention. */
    bool m_aborted;
};

inline void HexAbSearch::SetTT(SearchTT* tt)
{
    m_tt = tt;
}

inline void HexAbSearch::SetGuiFx(bool flag)
{
    m_use_guifx = flag;
}

inline bool HexAbSearch::GuiFx() const
{
    return m_use_guifx;
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // HEXABSEARCH_HPP
