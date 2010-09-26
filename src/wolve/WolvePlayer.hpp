//----------------------------------------------------------------------------
/** @file WolvePlayer.hpp
 */
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

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Varaition TT entry. */
struct VariationInfo
{
    VariationInfo()
        : depth(-1)
    { }
    
    VariationInfo(int d, const bitset_t& con)
        : depth(d), consider(con)
    { }

    ~VariationInfo();
    
    bool Initialized() const;

    void CheckCollision(const VariationInfo& other) const;

    bool ReplaceWith(const VariationInfo& other) const;

    //------------------------------------------------------------------------

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

inline void VariationInfo::CheckCollision(const VariationInfo& other) const
{
    UNUSED(other);
}

inline bool VariationInfo::ReplaceWith(const VariationInfo& other) const
{
    /** @todo check for better bounds/scores? */

    // replace this state only with a deeper state
    return (other.depth > depth);
}

//-------------------------------------------------------------------------- 

/** Performs an AlphaBeta search using Resistance for evaluations. 

    @todo Switch to SgSearch instead of HexAbSearch?
*/
class WolveSearch : public HexAbSearch
{
public:
    WolveSearch();

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

    /** Whether the backed-up ice info is used to reduce the moves to
        consider after a state has been searched. This is useful if
        using iterative deepening, since the next time the state is
        visited a smaller number of moves need to be considered. */
    bool BackupIceInfo() const;

    /** BackupIceInfo() */
    void SetBackupIceInfo(bool enable);

    // @}

private:
    /** Computes the evaluation on the no_fillin_board (if it exists)
        using the ConductanceGraph from m_brd. */
    void ComputeResistance(Resistance& resist);

    /** Consider sets for each depth. */
    std::vector<bitset_t> m_consider;

    /** See RootMovesToConsider() */
    bitset_t m_rootMTC;

    /** Variation TT. */
    TransTable<VariationInfo> m_varTT;

    /** See BackupIceInfo() */
    bool m_backup_ice_info;
};

inline bitset_t WolveSearch::RootMovesToConsider() const
{
    return m_rootMTC;
}

inline void WolveSearch::SetRootMovesToConsider(const bitset_t& consider)
{
    m_rootMTC = consider;
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
    explicit WolvePlayer();

    virtual ~WolvePlayer();
  
    /** Returns "wolve". */
    std::string Name() const;

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

    /** When time remaining is less than this, max search depth is set
	to 4. A value of zero turns this option off. */
    double PanicTime() const;
    
    /** See PanicTime() */
    void SetPanicTime(double time);

    // @}

private: 
    WolveSearch m_search;

    /** TT for search. */
    SearchTT m_tt;

    /** See PlyWidths() */
    std::vector<int> m_plywidth;

    /** See SearchDepths() */
    std::vector<int> m_search_depths;

    /** See PanicTime() */
    double m_panic_time;

    void CheckPanicMode(double timeRemaining, std::vector<int>& search_depths, 
                        std::vector<int>& plywidth);

    virtual HexPoint Search(const HexState& state, const Game& game,
                            HexBoard& brd, const bitset_t& consider,
                            double maxTime, double& score);
};

inline std::string WolvePlayer::Name() const
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

inline double WolvePlayer::PanicTime() const
{
    return m_panic_time;
}

inline void WolvePlayer::SetPanicTime(double time)
{
    m_panic_time = time;
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // WOLVEPLAYER_HPP
