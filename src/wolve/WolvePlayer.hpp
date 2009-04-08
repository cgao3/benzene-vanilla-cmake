//----------------------------------------------------------------------------
// $Id: WolvePlayer.hpp 1888 2009-02-01 01:04:41Z broderic $
//----------------------------------------------------------------------------

#ifndef WOLVEPLAYER_HPP
#define WOLVEPLAYER_HPP

#include <boost/scoped_ptr.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>

#include "BenzenePlayer.hpp"
#include "TwoDistance.hpp"
#include "HexAbSearch.hpp"
#include "Resistance.hpp"

//----------------------------------------------------------------------------

typedef enum 
{
    /** Tells thread to quit. */
    WOLVE_THREAD_QUIT=0,

    /** Tells thread to play a move. */
    WOLVE_THREAD_PLAY,

    /** Tells thread to undo last move played. */
    WOLVE_THREAD_UNDO

} WolveThreadAction;

/** Plays/Takesback moves. */
struct WolveWorkThread
{
    WolveWorkThread(HexBoard& brd, 
                    boost::barrier& start,
                    boost::barrier& completed,
                    const WolveThreadAction& action,
                    const HexPoint& move,
                    const HexColor& color);

    void operator()();

    HexBoard& m_brd;
    boost::barrier& m_start;
    boost::barrier& m_completed;
    const WolveThreadAction& m_action;
    const HexPoint& m_move;
    const HexColor& m_color;
};

//----------------------------------------------------------------------------

/** Varaition TT entry. */
struct VariationInfo
{
    VariationInfo()
        : hash(0), depth(-1)
    { }
    
    VariationInfo(hash_t h, int d, const bitset_t& con)
        : hash(h), depth(d), consider(con)
    { }

    ~VariationInfo();
    
    bool Initialized() const;

    hash_t Hash() const;

    void CheckCollision(const VariationInfo& other) const;

    bool ReplaceWith(const VariationInfo& other) const;

    //------------------------------------------------------------------------

    /** Hash for this variation. */
    hash_t hash;
   
    /** Depth state was searched. */
    int depth;

    /** Moves to consider from this variation. */
    bitset_t consider;
};

inline VariationInfo::~VariationInfo()
{
}

inline bool VariationInfo::Initialized() const
{
    return (depth != -1);
}

inline hash_t VariationInfo::Hash() const
{
    return hash;
}

inline 
void VariationInfo::CheckCollision(const VariationInfo& UNUSED(other)) const
{
    /** @todo Check for hash variation collisions. */
}

inline bool VariationInfo::ReplaceWith(const VariationInfo& other) const
{
    /** @todo check for better bounds/scores? */

    // replace this state only with a deeper state
    return (other.depth > depth);
}

//-------------------------------------------------------------------------- 

/** Performs an AlphaBeta search with two boards in parallel. 
    
    The first board has all vc/ice options turned on and is used for
    its vc set and strong win/loss detection. The second board does
    not do any ice, and is used solely for the evaluation. The second
    board's vc set is augmented with the vc set of the first board.

    The reason this is necessary is that ice fill-in degrades the
    playing strength of Wolve, probably because the Resistance
    evaluation behaves strangely with fillin.
*/
class WolveSearch : public HexAbSearch
{
public:

    /** Constructor */
    WolveSearch();

    /** Destructor */
    virtual ~WolveSearch();

    //-----------------------------------------------------------------------

    /** Moves to consider in the root state -- this set is used
        instead of the GenerateMoves() since it could have more
        knowledge. */
    bitset_t RootMovesToConsider() const;

    /** See RootMovesToConsider() */
    void SetRootMovesToConsider(const bitset_t& consider);

    //-----------------------------------------------------------------------

    /** @name Virtual methods from HexAbSearch. */
    // @{

    virtual HexEval Evaluate();

    virtual void GenerateMoves(std::vector<HexPoint>& moves);

    virtual void ExecuteMove(HexPoint move);

    virtual void UndoMove(HexPoint move);

    virtual void EnteredNewState();

    virtual void OnStartSearch();

    virtual void OnSearchComplete();

    virtual void AfterStateSearched();

    // @}

    //-----------------------------------------------------------------------

    /** @name Parameters */
    // @{

    /** Whether Wolve should use a thread for each of its boards. */
    bool UseThreads() const;

    /** See UseThreads() */
    void SetUseThreads(bool enable);

    /** Whether the backed-up ice info is used to reduce the moves to
        consider after a state has been searched. This is useful if
        using iterative deepening, since the next time the state is
        visited a smaller number of moves need to be considered. */
    bool BackupIceInfo() const;

    /** BackupIceInfo() */
    void SetBackupIceInfo(bool enable);

    // @}

private:

    /** Copy the board to the fill-in board. */
    void SetupNonFillinBoard();

    /** Computes the evaluation on the no_fillin_board (if it exists)
        using the ConductanceGraph from m_brd. */
    void ComputeResistance(Resistance& resist);

    /** Board with no fill-in. Used for circuit evaluation since 
        fill-in reduces the amount of flow. */
    boost::scoped_ptr<HexBoard> m_no_fillin_board;

    /** Consider sets for each depth. */
    std::vector<bitset_t> m_consider;

    /** See RootMovesToConsider() */
    bitset_t m_rootMTC;

    /** Variation TT. */
    TransTable<VariationInfo> m_varTT;

    /** See UseThreads() */
    bool m_use_threads;

    /** See BackupIceInfo() */
    bool m_backup_ice_info;

    //---------------------------------------------------------------------- 

    /** Allocate and starts threads. */
    void StartThreads();

    /** Stops and free threads. */
    void StopThreads();

    boost::barrier m_start_threads;

    boost::barrier m_threads_completed;

    boost::scoped_ptr<boost::thread> m_thread[2];

    WolveThreadAction m_threadAction;

    HexPoint m_threadMove;

    HexColor m_threadColor;
};

inline bitset_t WolveSearch::RootMovesToConsider() const
{
    return m_rootMTC;
}

inline void WolveSearch::SetRootMovesToConsider(const bitset_t& consider)
{
    m_rootMTC = consider;
}

inline bool WolveSearch::UseThreads() const
{
    return m_use_threads;
}

inline void WolveSearch::SetUseThreads(bool enable)
{
    m_use_threads = enable;
}

inline bool WolveSearch::BackupIceInfo() const
{
    return m_backup_ice_info;
}

inline void WolveSearch::SetBackupIceInfo(bool enable)
{
    m_backup_ice_info = enable;
}

//----------------------------------------------------------------------------

/** Player using HexAbSearch to generate moves. */
class WolvePlayer : public BenzenePlayer
{
public:
    
    /** Creates a player. */
    explicit WolvePlayer();
    
    /** Destructor. */
    virtual ~WolvePlayer();
  
    /** Returns "wolve". */
    std::string name() const;

    /** Returns the search. */
    WolveSearch& Search();

    //-----------------------------------------------------------------------

    /** @name Parameters */
    // @{

    /** Number of moves to consider at each depth after move
        ordering. */
    const std::vector<int>& PlyWidth() const;

    /** See PlyWidth() */
    void SetPlyWidth(const std::vector<int>& width);

    /** Depths to search if using iterative deepening.  
        See UseIterativeDeepening(). */
    const std::vector<int>& SearchDepths() const;

    /** See UseIterativeDeepening() */
    void SetSearchDepths(const std::vector<int>& depths);

    // @}

private: 

    /** Generates a move in the given gamestate using alphabeta. */
    virtual HexPoint search(HexBoard& brd, const Game& game_state,
			    HexColor color, const bitset_t& consider, 
                            double time_remaining, double& score);

    
    WolveSearch m_search;

    /** TT for search. */
    SearchTT m_tt;

    /** See PlyWidths() */
    std::vector<int> m_plywidth;

    /** See SearchDepths() */
    std::vector<int> m_search_depths;
};

inline std::string WolvePlayer::name() const
{
    return "wolve";
}

inline WolveSearch& WolvePlayer::Search()
{
    return m_search;
}

inline const std::vector<int>& WolvePlayer::PlyWidth() const
{
    return m_plywidth;
}

inline void WolvePlayer::SetPlyWidth(const std::vector<int>& width)
{
    m_plywidth = width;
}

inline const std::vector<int>& WolvePlayer::SearchDepths() const
{
    return m_search_depths;
}

inline void WolvePlayer::SetSearchDepths(const std::vector<int>& depths)
{
    m_search_depths = depths;
}

//----------------------------------------------------------------------------

#endif // WOLVEPLAYER_HPP
