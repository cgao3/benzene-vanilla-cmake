//----------------------------------------------------------------------------
/** @file BenzeneTestEngine.hpp
 */
//----------------------------------------------------------------------------

#ifndef WOLVEENGINE_HPP
#define WOLVEENGINE_HPP

#include "BenzeneHtpEngine.hpp"
#include "BookCheck.hpp"
#include "BookCommands.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Htp engine for Wolve. */
class BenzeneTestEngine : public BenzeneHtpEngine
{
public:

    BenzeneTestEngine(GtpInputStream& in, GtpOutputStream& out, 
                      int boardsize);
    
    ~BenzeneTestEngine();

    /** @name Command Callbacks */
    // @{

    // The callback functions are documented in the cpp file
    void CmdSetPlayer(HtpCommand& cmd);

    void CmdParamPlayer(HtpCommand& cmd);

    // @} // @name

#if GTPENGINE_PONDER
    virtual void Ponder();
    virtual void InitPonder();
    virtual void StopPonder();
#endif

private:

    boost::scoped_ptr<HexPlayer> m_player;

    double TimeForMove(HexColor color);

    HexPoint GenMove(HexColor color, bool useGameClock);

    void RegisterCmd(const std::string& name,
                     GtpCallback<BenzeneTestEngine>::Method method);

};

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // WOLVEENGINE_HPP
