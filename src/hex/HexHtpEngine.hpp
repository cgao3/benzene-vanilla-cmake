//----------------------------------------------------------------------------
// $Id: HexHtpEngine.hpp 1879 2009-01-29 03:24:49Z broderic $
//----------------------------------------------------------------------------

#ifndef HEXHTPENGINE_H
#define HEXHTPENGINE_H

#include "GtpEngine.h"
#include "Game.hpp"

#include <boost/scoped_ptr.hpp>

class SgNode;

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
    virtual HexPoint GenMove(HexColor color, double time_remaining);

    void SetPosition(const SgNode* node);

    StoneBoard m_board;
    boost::scoped_ptr<Game> m_game;

private:
    void RegisterCmd(const std::string& name,
                     GtpCallback<HexHtpEngine>::Method method);
};

//----------------------------------------------------------------------------

#endif // HEXHTPENGINE_H
