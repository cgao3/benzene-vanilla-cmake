//----------------------------------------------------------------------------
/** @file WolveSearch.hpp */
//----------------------------------------------------------------------------

#ifndef WOLVESEARCH_HPP
#define WOLVESEARCH_HPP

#include "SgSearch.h"
#include "SgSearchControl.h"
#include "SgHashTable.h"
#include "HexSgUtil.hpp"
#include "Resistance.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Variation TT entry. */
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

/** Aborts search when time has expired. */
class WolveSearchControl : public SgSearchControl
{
public:
    WolveSearchControl(double maxTime, bool useEarlyAbort,
                       const SgVector<SgMove>& PV);

    virtual ~WolveSearchControl();

    virtual bool Abort(double elapsedTime, int ignoreNumNodes);

    virtual bool StartNextIteration(int depth, double elapsedTime,
                                    int numNodes);

private:
    double m_maxTime;

    bool m_useEarlyAbort;

    const SgVector<SgMove>& m_pv;

    double m_lastDepthFinishedAt;

    /** Not implemented */
    WolveSearchControl(const WolveSearchControl&);

    /** Not implemented */
    WolveSearchControl& operator=(const WolveSearchControl&);
};

//-------------------------------------------------------------------------- 

/** Search used in Wolve.
    Based on SgSearch from the smartgame library. Performs an
    iterative deepening alpha-beta search using Resistance as the
    evaluation function. */
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

    /** Number of moves to consider at all depths. 
        If SpecificPlyWidths() is non-empty, this value is ignored. */
    std::size_t PlyWidth() const;

    /** See PlyWidth() */
    void SetPlyWidth(std::size_t width);

    /** Moves to consider at each depth.
        If non-empty, overrides PlyWidth() and enforces a maximum
        depth to the search (set to the length of this vector). */
    const std::vector<std::size_t>& SpecificPlyWidths() const;

    /** See SpecificPlyWidth() */
    void SetSpecificPlyWidths(const std::vector<std::size_t>& width);

    /** Board search will use.
        Caller retains ownership. */
    void SetWorkBoard(HexBoard* brd);

    /** Displays search progess.
        Use with HexGui to view search as it progresses. */
    bool GuiFx() const;

    /** See GuiFX(). */
    void SetGuiFx(bool flag);

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

    //-----------------------------------------------------------------------

    /** Prints the vector. */
    static std::string PrintPV(const SgVector<SgMove>& pv);

private:
    HexBoard* m_brd;

    /** Consider sets for each depth. */
    std::vector<bitset_t> m_consider;

    /** Sequence of moves from the root. */
    MoveSequence m_sequence;

    /** See PlyWidth() */
    std::size_t m_plyWidth;

    /** See SpecificPlyWidths() */
    std::vector<std::size_t> m_specificPlyWidths;

    /** See RootMovesToConsider() */
    bitset_t m_rootMTC;

    /** Variation TT. */
    //TransTable<VariationInfo> m_varTT;

    HexColor m_toPlay;

    /** See BackupIceInfo() */
    bool m_backup_ice_info;

    /** See GuiFx() */
    bool m_useGuiFx;

    void ComputeResistance(Resistance& resist);

    void OrderMoves(const bitset_t& consider, const Resistance& resist,
                    SgVector<SgMove>& outMoves) const;
};

inline bitset_t WolveSearch::RootMovesToConsider() const
{
    return m_rootMTC;
}

inline void WolveSearch::SetRootMovesToConsider(const bitset_t& consider)
{
    m_rootMTC = consider;
}

inline std::size_t WolveSearch::PlyWidth() const
{
    return m_plyWidth;
}

inline void WolveSearch::SetPlyWidth(std::size_t width)
{
    m_plyWidth = width;
}

inline const std::vector<std::size_t>& WolveSearch::SpecificPlyWidths() const
{
    return m_specificPlyWidths;
}

inline void WolveSearch::SetSpecificPlyWidths
(const std::vector<std::size_t>& width)
{
    m_specificPlyWidths = width;
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
    return m_brd->GetPosition().Hash(m_toPlay);
}

inline void WolveSearch::SetGuiFx(bool flag)
{
    m_useGuiFx = flag;
}

inline bool WolveSearch::GuiFx() const
{
    return m_useGuiFx;
}

//----------------------------------------------------------------------------

namespace WolveSearchUtil 
{
    /** Print move-value pairs for all moves in this state.
        Winning moves are denoted with a 'W' and losing moves a 'L'. */
    std::string PrintScores(const HexState& state,
                            const SgSearchHashTable& hashTable);

    /** Obtain PV by examining the hashtable. */
    void ExtractPVFromHashTable(const HexState& state, 
                                const SgSearchHashTable& hashTable, 
                                std::vector<HexPoint>& pv);

    /** Dump state info so the gui can display progress.
        Currently only does so after each depth is complete. Has to
        extract the PV from the hashtable implicitly.
        @todo Add hook functions in SgSearch so we can get updates
        more often, like we did before. The old way was to print the
        current pv and values of all searched moves each time the
        search returned to the root (see ticket #84). */
    void DumpGuiFx(const HexState& state, const SgSearchHashTable& hashTable);
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // WOLVESEARCH_HPP
