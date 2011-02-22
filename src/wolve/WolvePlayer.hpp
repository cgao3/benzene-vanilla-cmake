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

    std::size_t MinDepth() const;

    void SetMinDepth(std::size_t min);
   
    std::size_t MaxDepth() const;

    void SetMaxDepth(std::size_t max);

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

    std::size_t m_minDepth;

    std::size_t m_maxDepth;

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

inline std::size_t WolvePlayer::MinDepth() const
{
    return m_minDepth;
}

inline void WolvePlayer::SetMinDepth(std::size_t min)
{
    m_minDepth = min;
}

inline std::size_t WolvePlayer::MaxDepth() const
{
    return m_maxDepth;
}

inline void WolvePlayer::SetMaxDepth(std::size_t max)
{
    m_maxDepth = max;
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
