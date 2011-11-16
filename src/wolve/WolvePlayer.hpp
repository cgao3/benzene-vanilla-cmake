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

    SgSearchHashTable* HashTable();

    void SetHashTable(SgSearchHashTable* hash);

    /** Use time control to determine how much time to use per move. */
    bool UseTimeManagement() const;

    /** See UseTimeManagement() */
    void SetUseTimeManagement(bool flag);

    /** Estimates the time to search to the next ply based on the time
        to search the previous ply, aborts the search if this estimate
        exceeds the time remaining for the search. */
    bool UseEarlyAbort() const;
    
    /** See UseEarlyAbort() */
    void SetUseEarlyAbort(bool flag);

    /** Searches while waiting for a command. */
    bool Ponder() const;

    /** See Ponder() */
    void SetPonder(bool flag);

    // @}

private: 
    WolveSearch m_search;

    boost::scoped_ptr<SgSearchHashTable> m_hashTable;

    /** See MaxTime() */
    double m_maxTime;

    std::size_t m_minDepth;

    std::size_t m_maxDepth;

    /** See UseTimeManagement() */
    bool m_useTimeManagement;

    /** See UseEarlyAbort() */
    bool m_useEarlyAbort;

    /** See Ponder() */
    bool m_ponder;

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

inline SgSearchHashTable* WolvePlayer::HashTable()
{
    return m_hashTable.get();
}

inline void WolvePlayer::SetHashTable(SgSearchHashTable* hash)
{
    m_hashTable.reset(hash);
}

inline bool WolvePlayer::UseTimeManagement() const
{
    return m_useTimeManagement;
}

inline void WolvePlayer::SetUseTimeManagement(bool flag)
{
    m_useTimeManagement = flag;
}


inline bool WolvePlayer::UseEarlyAbort() const
{
    return m_useEarlyAbort;
}
    
inline void WolvePlayer::SetUseEarlyAbort(bool f)
{
    m_useEarlyAbort = f;
}

inline bool WolvePlayer::Ponder() const
{
    return m_ponder;
}

inline void WolvePlayer::SetPonder(bool flag)
{
    m_ponder = flag;
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // WOLVEPLAYER_HPP
