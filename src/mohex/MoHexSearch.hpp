//----------------------------------------------------------------------------
/** @file MoHexSearch.hpp */
//----------------------------------------------------------------------------

#ifndef MOHEXSEARCH_H
#define MOHEXSEARCH_H

#include "SgBlackWhite.h"
#include "SgPoint.h"
#include "SgNode.h"
#include "SgUctSearch.h"

#include "MoHexThreadState.hpp"
#include "PatternState.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

class MoHexSharedPolicy;

/** Creates threads. */
class HexThreadStateFactory : public SgUctThreadStateFactory
{
public:
    HexThreadStateFactory(MoHexSharedPolicy* shared_policy);

    ~HexThreadStateFactory();

    SgUctThreadState* Create(unsigned int threadId, const SgUctSearch& search);
private:

    MoHexSharedPolicy* m_shared_policy;
};

//----------------------------------------------------------------------------

/** Monte-Carlo search using UCT for Hex. */
class MoHexSearch : public SgUctSearch
{
public:
    /** Constructor.
        @param factory Creates MoHexState instances for each thread.
        @param maxMoves Maximum move number.
    */
    MoHexSearch(SgUctThreadStateFactory* factory,
                int maxMoves = 0);
    
    ~MoHexSearch();    

    //-----------------------------------------------------------------------
    
    /** @name Pure virtual functions of SgUctSearch */
    // @{

    std::string MoveString(SgMove move) const;

    SgUctValue UnknownEval() const;

    SgUctValue InverseEval(SgUctValue eval) const;

    // @}

    //-----------------------------------------------------------------------

    /** @name Virtual functions of SgUctSearch */
    // @{

    void OnSearchIteration(SgUctValue gameNumber, const unsigned int threadId,
                           const SgUctGameInfo& info);

    void OnStartSearch();

    // @}

    //-----------------------------------------------------------------------

    /** @name Hex-specific functions */
    // @{

    void SetBoard(HexBoard& board);

    HexBoard& Board();

    const HexBoard& Board() const;

    void SetSharedData(MoHexSharedData& data);

    MoHexSharedData& SharedData();

    const MoHexSharedData& SharedData() const;

    /** @see SetKeepGames()
        @throws SgException if KeepGames() was false at last invocation of
        StartSearch() */
    void SaveGames(const std::string& fileName) const;

    void AppendGame(const std::vector<SgMove>& sequence);

    /** @see MoHexUtil::SaveTree() */
    void SaveTree(std::ostream& out, int maxDepth) const;

    // @}

    //-----------------------------------------------------------------------

    /** @name Hex-specific parameters */
    // @{

    /** Keeps a SGF tree of all games.
        Games are cleared in each OnStartSearch(). Games can be saved
        with SaveGames(). */
    void SetKeepGames(bool enable);

    /** See SetKeepGames(). */
    bool KeepGames() const;

    /** Enables output of live graphics commands for HexGui.
        See GoGuiGfx() */
    void SetLiveGfx(bool enable);

    /** See SetLiveGfx(). */
    bool LiveGfx() const;

    /** Pattern-check radius to use during in-tree phase. */
    int TreeUpdateRadius() const;

    /** See TreeUpdateRadius(). */
    void SetTreeUpdateRadius(int radius);
    
    /** Pattern-check radius to use during playout phase. */
    int PlayoutUpdateRadius() const;
    
    /** See PlayoutUpdateRadius(). */
    void SetPlayoutUpdateRadius(int radius);

    /** Size of the map of fillin states. */
    int FillinMapBits() const;

    /** See FillinMapBits(). */
    void SetFillinMapBits(int bits);

    // @} 

private:
    /** See SetKeepGames() */
    bool m_keepGames;

    /** See SetLiveGfx() */
    bool m_liveGfx;

    /** See TreeUpdateRadius() */
    int m_treeUpdateRadius;

    /** See PlayoutUpdateRadius() */
    int m_playoutUpdateRadius;

    /** Nothing is done to this board. 
        We do not own this pointer. Threads will create their own
        HexBoards, but the settings (ICE and VCs) will be copied from
        this board. */
    HexBoard* m_brd;

    int m_fillinMapBits;
   
    /** Data among threads. */
    boost::scoped_ptr<MoHexSharedData> m_sharedData;

    StoneBoard m_lastPositionSearched;

    /** See SetKeepGames().
        Should be non-null only if KeepGames() is true. */
    SgNode* m_root;

    SgUctValue m_nextLiveGfx;

    /** Not implemented */
    MoHexSearch(const MoHexSearch& search);

    /** Not implemented */
    MoHexSearch& operator=(const MoHexSearch& search);
};

inline void MoHexSearch::SetBoard(HexBoard& board)
{
    m_brd = &board;
}

inline HexBoard& MoHexSearch::Board()
{
    return *m_brd;
}

inline const HexBoard& MoHexSearch::Board() const
{
    return *m_brd;
}

inline bool MoHexSearch::KeepGames() const
{
    return m_keepGames;
}

inline bool MoHexSearch::LiveGfx() const
{
    return m_liveGfx;
}

inline void MoHexSearch::SetKeepGames(bool enable)
{
    m_keepGames = enable;
}

inline void MoHexSearch::SetLiveGfx(bool enable)
{
    m_liveGfx = enable;
}

inline int MoHexSearch::TreeUpdateRadius() const
{
    return m_treeUpdateRadius;
}

inline void MoHexSearch::SetTreeUpdateRadius(int radius)
{
    m_treeUpdateRadius = radius;
}
    
inline int MoHexSearch::PlayoutUpdateRadius() const
{
    return m_playoutUpdateRadius;
}
    
inline void MoHexSearch::SetPlayoutUpdateRadius(int radius)
{
    m_playoutUpdateRadius = radius;
}

inline void MoHexSearch::SetSharedData(MoHexSharedData& data)
{
    m_sharedData.reset(new MoHexSharedData(data));
}

inline MoHexSharedData& MoHexSearch::SharedData()
{
    return *m_sharedData;
}

inline const MoHexSharedData& MoHexSearch::SharedData() const
{
    return *m_sharedData;
}

inline int MoHexSearch::FillinMapBits() const
{
    return m_fillinMapBits;
}

inline void MoHexSearch::SetFillinMapBits(int bits)
{
    m_fillinMapBits = bits;
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // MOHEXSEARCH_H
