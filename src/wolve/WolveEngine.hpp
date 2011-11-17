//----------------------------------------------------------------------------
/** @file WolveEngine.hpp */
//----------------------------------------------------------------------------

#ifndef WOLVEENGINE_HPP
#define WOLVEENGINE_HPP

#include "CommonHtpEngine.hpp"
#include "CacheBook.hpp"
#include "WolvePlayer.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Htp engine for Wolve. */
class WolveEngine : public CommonHtpEngine
{
public:
    WolveEngine(int boardsize, WolvePlayer& player);
    
    ~WolveEngine();

    /** @page wolvehtpenginecommands WolveEngine Commands
        - @link CmdParam() @c param_wolve @endlink
        - @link CmdScores() @c wolve-scores @endlink
        - @link CmdData() @c wolve-data @endlink
    */

    /** @name Command Callbacks */
    // @{

    // The callback functions are documented in the cpp file
    void CmdAnalyzeCommands(HtpCommand& cmd);
    void CmdParam(HtpCommand& cmd);
    void CmdGetPV(HtpCommand& cmd);
    void CmdScores(HtpCommand& cmd);
    void CmdData(HtpCommand& cmd);
    void CmdClearHash(HtpCommand& cmd);

    // @} // @name

#if GTPENGINE_PONDER
    virtual void Ponder();
    virtual void InitPonder();
    virtual void StopPonder();
#endif

private:
    WolvePlayer& m_player;

    CacheBook m_cacheBook;

    bool m_useCacheBook;

    double TimeForMove(HexColor color);

    HexPoint GenMove(HexColor color, bool useGameClock);

    HexPoint DoSearch(HexColor color, double maxTime);

    void RegisterCmd(const std::string& name,
                     GtpCallback<WolveEngine>::Method method);
};

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // WOLVEENGINE_HPP
