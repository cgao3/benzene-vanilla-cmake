//----------------------------------------------------------------------------
/** @file
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

    HexHtpEngine(std::istream& in, std::ostream& out, 
                 int boardsize);
    
    ~HexHtpEngine();

    /** @name Command Callbacks */
    // @{
    // The callback functions are documented in the cpp file

    virtual void CmdName(HtpCommand&);
    virtual void CmdVersion(HtpCommand&);
    virtual void CmdPlay(HtpCommand&);
    virtual void CmdGenMove(HtpCommand& cmd);
    virtual void CmdUndo(HtpCommand& cmd);
    virtual void CmdNewGame(HtpCommand& cmd);
    virtual void CmdClearBoard(HtpCommand& cmd);
    virtual void CmdShowboard(HtpCommand&);
    virtual void CmdBoardID(HtpCommand&);
    virtual void CmdTimeLeft(HtpCommand&);
    virtual void CmdFinalScore(HtpCommand& cmd);
    virtual void CmdAllLegalMoves(HtpCommand& cmd);
    virtual void CmdLoadSgf(HtpCommand& cmd);
    virtual void CmdParamGame(HtpCommand& cmd);

    void CmdExec(HtpCommand& cmd);

#if GTPENGINE_INTERRUPT
    virtual void CmdInterrupt(HtpCommand& cmd);
#endif

    // @} // @name

#if GTPENGINE_PONDER
    virtual void Ponder();

    virtual void InitPonder();

    virtual void StopPonder();
#endif

#if GTPENGINE_INTERRUPT
    /** Calls SgSetUserAbort(). */
    void Interrupt();
#endif

protected:

    StoneBoard m_board;

    boost::scoped_ptr<Game> m_game;

    /** Clears SgAbortFlag() */
    void BeforeHandleCommand();

    /** Does nothing. */
    void BeforeWritingResponse();

    HexColor ColorArg(const HtpCommand& cmd, std::size_t number) const;

    HexPoint MoveArg(const HtpCommand& cmd, std::size_t number) const;

    void PrintBitsetToHTP(HtpCommand& cmd, const bitset_t& bs) const;

    /** Plays a move. */
    virtual void Play(HexColor color, HexPoint move);

    /** Creates a new game on a board with given dimensions. */
    virtual void NewGame(int width, int height);

    /** Generates a move. */
    virtual HexPoint GenMove(HexColor color, double max_time);

    /** Returns time available for the player to make their move. 
        Default implementation returns time remaining in the game.
     */
    virtual double TimeForMove(HexColor color);

    void SetPosition(const SgNode* node);

private:
    void RegisterCmd(const std::string& name,
                     GtpCallback<HexHtpEngine>::Method method);
};

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // HEXHTPENGINE_H
