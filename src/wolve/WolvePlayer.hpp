//----------------------------------------------------------------------------
/** @file WolvePlayer.hpp */
//----------------------------------------------------------------------------

#ifndef WOLVEPLAYER_HPP
#define WOLVEPLAYER_HPP

#include "BenzenePlayer.hpp"
#include "WolveSearch.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Wolve. 
    Uses WolveSearch to generate moves. */
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

    /** Maximum time to spend on search (in seconds). */
    double MaxTime() const;

    /** See MaxTime() */
    void SetMaxTime(double time);

    /** Depths to search if using iterative deepening. */
    const std::vector<std::size_t>& SearchDepths() const;

    /** See SearchDepths() */
    void SetSearchDepths(const std::vector<std::size_t>& depths);

    const SgSearchHashTable& HashTable() const;

    /** Use time control to determine how much time to use per move. */
    bool UseTimeManagement() const;

    /** See UseTimeManagement() */
    void SetUseTimeManagement(bool flag);

    // @}

private: 
    WolveSearch m_search;

    SgSearchHashTable m_hashTable;

    /** See MaxTime() */
    double m_maxTime;

    /** See SearchDepths() */
    std::vector<std::size_t> m_searchDepths;

    /** See UseTimeManagement() */
    bool m_useTimeManagement;

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

inline double WolvePlayer::MaxTime() const
{
    return m_maxTime;
}

inline void WolvePlayer::SetMaxTime(double time)
{
    m_maxTime = time;
}

inline const std::vector<std::size_t>& WolvePlayer::SearchDepths() const
{
    return m_searchDepths;
}

inline void WolvePlayer::SetSearchDepths
(const std::vector<std::size_t>& depths)
{
    m_searchDepths = depths;
}

inline const SgSearchHashTable& WolvePlayer::HashTable() const
{
    return m_hashTable;
}

inline bool WolvePlayer::UseTimeManagement() const
{
    return m_useTimeManagement;
}

inline void WolvePlayer::SetUseTimeManagement(bool flag)
{
    m_useTimeManagement = flag;
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // WOLVEPLAYER_HPP
