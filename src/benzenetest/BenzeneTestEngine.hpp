//----------------------------------------------------------------------------
/** @file BenzeneTestEngine.hpp */
//----------------------------------------------------------------------------

#ifndef BENZENETESTENGINE_HPP
#define BENZENETESTENGINE_HPP

#include "CommonHtpEngine.hpp"
#include "BookCheck.hpp"
#include "BookCommands.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Htp engine for Wolve. */
class BenzeneTestEngine : public CommonHtpEngine
{
public:
    BenzeneTestEngine(int boardsize);
    
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

#endif // BENZENETESTENGINE_HPP
