//----------------------------------------------------------------------------
/** @file JYEngine.hpp */
//----------------------------------------------------------------------------

#ifndef JYENGINE_HPP
#define JYENGINE_HPP

#include "CommonHtpEngine.hpp"
#include "JYPlayer.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Htp engine for JY. */
class JYEngine : public CommonHtpEngine
{
public:
    JYEngine(int boardsize, JYPlayer& player);
    
    ~JYEngine();

    /** @page jyhtpenginecommands JYEngine Commands
        - @link CmdParam() @c param_wolve @endlink
        - @link CmdScores() @c wolve-scores @endlink
        - @link CmdData() @c wolve-data @endlink
    */

    /** @name Command Callbacks */
    // @{

    // The callback functions are documented in the cpp file
    void CmdAnalyzeCommands(HtpCommand& cmd);
    void CmdShowJYPatternList(HtpCommand& cmd);
    void CmdLoadPatternFile(HtpCommand& cmd);
    void CmdUndo(HtpCommand& cmd);
    void CmdNewGame(HtpCommand& cmd);
    void CmdClearBoard(HtpCommand& cmd);

    // @} // @name

#if GTPENGINE_PONDER
    virtual void Ponder();
    virtual void InitPonder();
    virtual void StopPonder();
#endif

private:
    JYPlayer& m_player;

    HexPoint GenMove(HexColor color, bool useGameClock);

    HexPoint DoSearch(HexColor color, double maxTime);

    void RegisterCmd(const std::string& name, GtpCallback<JYEngine>::Method method);
};

//----------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // JYENGINE_HPP
