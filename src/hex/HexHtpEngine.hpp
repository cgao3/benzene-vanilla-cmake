//----------------------------------------------------------------------------
/** @file HexHtpEngine.hpp
 */
//----------------------------------------------------------------------------

#ifndef HEXHTPENGINE_H
#define HEXHTPENGINE_H

#include "GtpEngine.h"
#include "SgNode.h"

#include "Game.hpp"

#include <boost/scoped_ptr.hpp>

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

typedef GtpCommand HtpCommand;
typedef GtpFailure HtpFailure;

//----------------------------------------------------------------------------

/** Basic htp commands any Hex engine is required to support.  */
class HexHtpEngine: public GtpEngine
{
public:

    HexHtpEngine(GtpInputStream& in, GtpOutputStream& out, int boardsize);
    
    ~HexHtpEngine();

    /** @page hexhtpenginecommands HexHtpEngine Commands
        - @link CmdAllLegalMoves() @c all_legal_moves @endlink
        - @link CmdBoardID() @c board_id @endlink
        - @link CmdClearBoard() @c clear_board @endlink
        - @link CmdExec() @c exec @endlink
        - @link CmdFinalScore() @c final_score @endlink
        - @link CmdGenMove() @c genmove @endlink
        - @link CmdRegGenMove() @c reg_genmove @endlink
        - @link CmdInterrupt() @c gogui-interrupt @endlink
        - @link CmdLoadSgf() @c loadsgf @endlink
        - @link CmdName() @c name @endlink
        - @link CmdNewGame() @c boardsize @endlink
        - @link CmdParamGame() @c param_game @endlink
        - @link CmdPlay() @c play @endlink
        - @link CmdShowboard() @c showboard @endlink
        - @link CmdTimeLeft() @c time_left @endlink
        - @link CmdUndo() @c undo @endlink
        - @link CmdVersion() @c version @endlink
    */

    /** @name Command Callbacks */
    // @{
    // The callback functions are documented in the cpp file

    virtual void CmdAllLegalMoves(HtpCommand& cmd);
    virtual void CmdBoardID(HtpCommand&);
    virtual void CmdClearBoard(HtpCommand& cmd);
    virtual void CmdExec(HtpCommand& cmd);
    virtual void CmdFinalScore(HtpCommand& cmd);
    virtual void CmdGenMove(HtpCommand& cmd);
    virtual void CmdRegGenMove(HtpCommand& cmd);
#if GTPENGINE_INTERRUPT
    virtual void CmdInterrupt(HtpCommand& cmd);
#endif
    virtual void CmdLoadSgf(HtpCommand& cmd);
    virtual void CmdName(HtpCommand&);
    virtual void CmdNewGame(HtpCommand& cmd);
    virtual void CmdParamGame(HtpCommand& cmd);
    virtual void CmdPlay(HtpCommand&);
    virtual void CmdShowboard(HtpCommand&);
    virtual void CmdTimeLeft(HtpCommand&);
    virtual void CmdUndo(HtpCommand& cmd);
    virtual void CmdVersion(HtpCommand&);

    // @} // @name

#if GTPENGINE_INTERRUPT
    /** Calls SgSetUserAbort(). */
    void Interrupt();
#endif

protected:

    StoneBoard m_board;

    Game m_game;

    /** Clears SgAbortFlag() */
    void BeforeHandleCommand();

    /** Does nothing. */
    void BeforeWritingResponse();

    /** Plays a move. */
    virtual void Play(HexColor color, HexPoint move);

    /** Creates a new game on a board with given dimensions. */
    virtual void NewGame(int width, int height);

    /** Generates a move for color. */
    virtual HexPoint GenMove(HexColor color, bool useGameClock) = 0;

    void SetPosition(const SgNode* node);

private:
    void RegisterCmd(const std::string& name,
                     GtpCallback<HexHtpEngine>::Method method);
};

//----------------------------------------------------------------------------

/** Misc HTP utilities. */
namespace HtpUtil
{
    HexColor ColorArg(const HtpCommand& cmd, std::size_t number);

    HexPoint MoveArg(const HtpCommand& cmd, std::size_t number);
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // HEXHTPENGINE_H
