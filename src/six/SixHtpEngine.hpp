//----------------------------------------------------------------------------
// $Id: SixHtpEngine.hpp 1870 2009-01-28 00:45:23Z broderic $
//----------------------------------------------------------------------------

#ifndef SIXHTPENGINE_H
#define SIXHTPENGINE_H

#include "HexHtpEngine.hpp"

#include "sixplayer.h"
#include "hexboard.h"
#include "hexgame.h"
#include "hexmark.h"

#include <boost/scoped_ptr.hpp>

//----------------------------------------------------------------------------

class SixHtpEngine: public HexHtpEngine
{
public:

    SixHtpEngine(std::istream& in, std::ostream& out, int boardsize);
    
    ~SixHtpEngine();

    /** @name Command Callbacks */
    // @{
    // The callback functions are documented in the cpp file

    virtual void CmdUndo(HtpCommand& cmd);
    virtual void CmdScoreForLastMove(HtpCommand& cmd);

    virtual void CmdVCBuild(HtpCommand& cmd);
    virtual void CmdGetCellsConnectedTo(HtpCommand& cmd);
    virtual void CmdGetVCsBetween(HtpCommand& cmd);

    virtual void CmdParamSix(HtpCommand& cmd);

    // @} // @name

protected:

    /** Generates a move. */
    HexPoint GenMove(HexColor color, double time_reminaing);
    
    /** Convert to Six colors and moves. */
    HexMark SixColor(HexColor color);
    HexMove SixMove(HexColor color, HexPoint move);
    HexPoint SixFieldToHexPoint(HexField field);
    HexField HexPointToSixPoint(HexPoint move);
    HexPoint SixMoveToHexPoint(HexMove move);

    bool VcArg(const std::string& arg);

    /** Plays a move.  */
    virtual void Play(HexColor color, HexPoint move);
    virtual void NewGame(int width, int height);

    boost::scoped_ptr<HexBoard> m_sixboard;
    boost::scoped_ptr<HexGame> m_sixgame;
    boost::scoped_ptr<SixPlayer> m_sixplayer;

    Connector* m_con[BLACK_AND_WHITE];

private:
    void RegisterCmd(const std::string& name,
                     GtpCallback<SixHtpEngine>::Method method);
};

//----------------------------------------------------------------------------

#endif // SIXHTPENGINE_H
