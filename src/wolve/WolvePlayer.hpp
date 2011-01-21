//----------------------------------------------------------------------------
/** @file WolvePlayer.hpp */
//----------------------------------------------------------------------------

#ifndef WOLVEPLAYER_HPP
#define WOLVEPLAYER_HPP

#include "SgSearch.h"
#include "SgHashTable.h"
#include "BenzenePlayer.hpp"
#include "HexSgUtil.hpp"
#include "Resistance.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Varaition TT entry. */
struct VariationInfo
{
    /** Depth state was searched. */
    int m_depth;

    /** Moves to consider from this variation. */
    bitset_t m_consider;

    VariationInfo();
    
    VariationInfo(int depth, const bitset_t& consider);

    ~VariationInfo();
    
    bool IsValid() const;

    void Invalidate();

    bool IsBetterThan(const VariationInfo& other) const;

private:
    bool m_isValid;    
};

inline VariationInfo::VariationInfo()
    : m_isValid(false)
{
}

inline VariationInfo::VariationInfo(int depth, const bitset_t& consider)
    : m_depth(depth),
      m_consider(consider),
      m_isValid(false)
{
}

inline VariationInfo::~VariationInfo()
{
}

inline bool VariationInfo::IsValid() const
{
    return m_isValid;
}

inline void VariationInfo::Invalidate()
{
    m_isValid = false;
}

inline bool VariationInfo::IsBetterThan(const VariationInfo& other) const
{
    return m_depth > other.m_depth;
}

//-------------------------------------------------------------------------- 

/** Performs an AlphaBeta search using Resistance for evaluations. */
class WolveSearch : public SgSearch
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

    /** Number of moves to consider at each depth after move
        ordering. */
    const std::vector<std::size_t>& PlyWidth() const;

    /** See PlyWidth() */
    void SetPlyWidth(const std::vector<std::size_t>& width);

    void SetWorkBoard(HexBoard* brd);

    //-----------------------------------------------------------------------

    /** @name Virtual methods from HexAbSearch. */
    // @{

    bool CheckDepthLimitReached() const;

    std::string MoveString(SgMove move) const;

    void SetToPlay(SgBlackWhite toPlay);

    SgBlackWhite GetToPlay() const;

    void StartOfDepth(int depthLimit);

    void OnStartSearch();

    void Generate(SgVector<SgMove>* moves, int depth);

    int Evaluate(bool* isExact, int depth);

    bool Execute(SgMove move, int* delta, int depth);

    void TakeBack();

    SgHashCode GetHashCode() const;

    bool EndOfGame() const;

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
    HexBoard* m_brd;

    /** Consider sets for each depth. */
    std::vector<bitset_t> m_consider;

    /** Sequence of moves from the root. */
    MoveSequence m_sequence;

    /** See PlyWidths() */
    std::vector<std::size_t> m_plyWidth;

    /** See RootMovesToConsider() */
    bitset_t m_rootMTC;

    /** Variation TT. */
    //TransTable<VariationInfo> m_varTT;

    HexColor m_toPlay;

    /** See BackupIceInfo() */
    bool m_backup_ice_info;

    void ComputeResistance(Resistance& resist);

    void OrderMoves(const bitset_t& consider, const Resistance& resist,
                    SgVector<SgMove>& outMoves);
};

inline bitset_t WolveSearch::RootMovesToConsider() const
{
    return m_rootMTC;
}

inline void WolveSearch::SetRootMovesToConsider(const bitset_t& consider)
{
    m_rootMTC = consider;
}

inline const std::vector<std::size_t>& WolveSearch::PlyWidth() const
{
    return m_plyWidth;
}

inline void WolveSearch::SetPlyWidth(const std::vector<std::size_t>& width)
{
    m_plyWidth = width;
}

inline bool WolveSearch::BackupIceInfo() const
{
    return m_backup_ice_info;
}

inline void WolveSearch::SetBackupIceInfo(bool enable)
{
    m_backup_ice_info = enable;
}

inline void WolveSearch::SetWorkBoard(HexBoard* brd)
{
    m_brd = brd;
}

inline void WolveSearch::SetToPlay(SgBlackWhite toPlay)
{
    m_toPlay = HexSgUtil::SgColorToHexColor(toPlay);
}

inline SgBlackWhite WolveSearch::GetToPlay() const
{
    return HexSgUtil::HexColorToSgColor(m_toPlay);
}

inline SgHashCode WolveSearch::GetHashCode() const
{
    return m_brd->GetPosition().Hash();
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

    /** Depths to search if using iterative deepening.  
        See UseIterativeDeepening(). */
    const std::vector<std::size_t>& SearchDepths() const;

    /** See UseIterativeDeepening() */
    void SetSearchDepths(const std::vector<std::size_t>& depths);

    /** When time remaining is less than this, max search depth is set
	to 4. A value of zero turns this option off. */
    double PanicTime() const;
    
    /** See PanicTime() */
    void SetPanicTime(double time);

    // @}

private: 
    WolveSearch m_search;

    /** TT for search. */
    SgSearchHashTable m_hashTable;

    /** See SearchDepths() */
    std::vector<std::size_t> m_search_depths;

    /** See PanicTime() */
    double m_panic_time;

    void CheckPanicMode(double timeRemaining, 
                        std::vector<std::size_t>& search_depths, 
                        std::vector<std::size_t>& plywidth);

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

inline const std::vector<std::size_t>& WolvePlayer::SearchDepths() const
{
    return m_search_depths;
}

inline void WolvePlayer::SetSearchDepths
(const std::vector<std::size_t>& depths)
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
