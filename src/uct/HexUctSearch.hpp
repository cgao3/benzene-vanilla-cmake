//----------------------------------------------------------------------------
// $Id: HexUctSearch.hpp 1807 2008-12-19 22:19:36Z broderic $ 
//----------------------------------------------------------------------------

#ifndef HEXUCTSEARCH_H
#define HEXUCTSEARCH_H

#include "SgBlackWhite.h"
#include "SgPoint.h"
#include "SgUctSearch.h"

#include "HexUctState.hpp"
#include "PatternBoard.hpp"

class SgNode;

//----------------------------------------------------------------------------

class HexUctSharedPolicy;

/** Creates threads. */
class HexThreadStateFactory : public SgUctThreadStateFactory
{
public:
    HexThreadStateFactory(HexUctSharedPolicy* shared_policy);

    ~HexThreadStateFactory();

    SgUctThreadState* Create(std::size_t threadId,
                             const SgUctSearch& search);
private:

    HexUctSharedPolicy* m_shared_policy;
};

//----------------------------------------------------------------------------

/** Monte-Carlo search using UCT for Hex. */
class HexUctSearch : public SgUctSearch
{
public:

    /** Constructor.
        @param factory Creates HexUctState instances for each thread.
        @param maxMoves Maximum move number.
    */
    HexUctSearch(SgUctThreadStateFactory* factory,
                 int maxMoves = 0);
    
    /** Destructor. */
    ~HexUctSearch();    

    //-----------------------------------------------------------------------
    
    /** @name Pure virtual functions of SgUctSearch */
    // @{

    std::string MoveString(SgMove move) const;

    float UnknownEval() const;

    float InverseEval(float eval) const;

    // @}

    //-----------------------------------------------------------------------

    /** @name Virtual functions of SgUctSearch */
    // @{

    void OnSearchIteration(std::size_t gameNumber, int threadId,
                           const SgUctGameInfo& info);

    void OnStartSearch();

    // @}

    //-----------------------------------------------------------------------

    /** @name Hex-specific functions */
    // @{

    void SetBoard(PatternBoard& board);

    PatternBoard& Board();

    const PatternBoard& Board() const;

    void SetInitialData(const HexUctInitialData* data);

    const HexUctInitialData* InitialData() const;

    /** @see SetKeepGames()
        @throws SgException if KeepGames() was false at last invocation of
        StartSearch()
    */
    void SaveGames(const std::string& fileName) const;

    /** @see HexUctUtil::SaveTree() */
    void SaveTree(std::ostream& out) const;

    //FIXME: Phil commented these out just because, but don't seem to exist?!?
    //void SetToPlay(HexColor toPlay);
    //HexColor ToPlay() const;

    // @}

    //-----------------------------------------------------------------------

    /** @name Hex-specific parameters */
    // @{

    /** Keep a SGF tree of all games.
        This is reset in OnStartSearch() and can be saved with SaveGames().
    */
    void SetKeepGames(bool enable);

    /** @see SetKeepGames() */
    bool KeepGames() const;

    /** Enable outputting of live graphics commands for GoGui.
        Outputs LiveGfx commands for GoGui to the debug stream every
        n games.
        @see GoGuiGfx(), SetLiveGfxInterval()
    */
    void SetLiveGfx(bool enable);

    /** @see SetLiveGfx() */
    bool LiveGfx() const;

    /** Set interval for outputtingof live graphics commands for GoGui.
        Default is every 5000 games.
        @see SetLiveGfx()
    */
    void SetLiveGfxInterval(int interval);

    /** @see SetLiveGfxInterval() */
    int LiveGfxInterval() const;

    /** Pattern-check radius to use during in-tree phase. */
    int TreeUpdateRadius() const;

    /** See TreeUpdateRadius() */
    void SetTreeUpdateRadius(int radius);
    
    /** Pattern-check radius to use during playout phase. */
    int PlayoutUpdateRadius() const;
    
    void SetPlayoutUpdateRadius(int radius);

    // @} 

protected:

    /** @see SetKeepGames() */
    bool m_keepGames;

    /** @see SetLiveGfx() */
    bool m_liveGfx;

    /** @see SetLiveGfxInterval() */
    int m_liveGfxInterval;

    /** @see TreeUpdateRadius() */
    int m_treeUpdateRadius;

    /** @see PlayoutUpdateRadius() */
    int m_playoutUpdateRadius;

    //----------------------------------------------------------------------

    /** Nothing is done to this board. We do not own this pointer.
        Threads will synchronise with this board at the start of the
        search. */
    PatternBoard* m_brd;
   
    /** Data for first few ply of the game. Shared amoung threads. */
    const HexUctInitialData* m_initial_data;

    //----------------------------------------------------------------------

    /** @see SetKeepGames() */
    SgNode* m_root;
    
    /** Not implemented */
    HexUctSearch(const HexUctSearch& search);

    /** Not implemented */
    HexUctSearch& operator=(const HexUctSearch& search);

    //void AppendGame(const std::vector<SgMove>& sequence);
};

inline void HexUctSearch::SetBoard(PatternBoard& board)
{
    m_brd = &board;
}

inline PatternBoard& HexUctSearch::Board()
{
    return *m_brd;
}

inline const PatternBoard& HexUctSearch::Board() const
{
    return *m_brd;
}

inline bool HexUctSearch::KeepGames() const
{
    return m_keepGames;
}

inline bool HexUctSearch::LiveGfx() const
{
    return m_liveGfx;
}

inline int HexUctSearch::LiveGfxInterval() const
{
    return m_liveGfxInterval;
}

inline void HexUctSearch::SetKeepGames(bool enable)
{
    m_keepGames = enable;
}

inline void HexUctSearch::SetLiveGfx(bool enable)
{
    m_liveGfx = enable;
}

inline void HexUctSearch::SetLiveGfxInterval(int interval)
{
    SG_ASSERT(interval > 0);
    m_liveGfxInterval = interval;
}

inline int HexUctSearch::TreeUpdateRadius() const
{
    return m_treeUpdateRadius;
}

inline void HexUctSearch::SetTreeUpdateRadius(int radius)
{
    m_treeUpdateRadius = radius;
}
    
inline int HexUctSearch::PlayoutUpdateRadius() const
{
    return m_playoutUpdateRadius;
}
    
inline void HexUctSearch::SetPlayoutUpdateRadius(int radius)
{
    m_playoutUpdateRadius = radius;
}

inline void HexUctSearch::SetInitialData(const HexUctInitialData* data)
{
    m_initial_data = data;
}

inline const HexUctInitialData* HexUctSearch::InitialData() const
{
    return m_initial_data;
}

//----------------------------------------------------------------------------

#endif // HEXUCTSEARCH_H
