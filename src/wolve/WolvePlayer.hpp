//----------------------------------------------------------------------------
/** @file WolvePlayer.hpp */
//----------------------------------------------------------------------------

#ifndef WOLVEPLAYER_HPP
#define WOLVEPLAYER_HPP

#include "BenzenePlayer.hpp"
#include "WolveSearch.hpp"

_BEGIN_BENZENE_NAMESPACE_

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

    // @}

private: 
    WolveSearch m_search;

    /** TT for search. */
    SgSearchHashTable m_hashTable;

    /** See SearchDepths() */
    std::vector<std::size_t> m_search_depths;

    virtual HexPoint Search(const HexState& state, const Game& game,
                            HexBoard& brd, const bitset_t& consider,
                            double maxTime, double& score);

    std::string PrintStatistics(int score, const SgVector<SgMove>& pv);
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

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // WOLVEPLAYER_HPP
