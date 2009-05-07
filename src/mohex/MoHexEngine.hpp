//----------------------------------------------------------------------------
/** @file
 */
//----------------------------------------------------------------------------

#ifndef MOHEXENGINE_HPP
#define MOHEXEENGINE_HPP

#include "Hex.hpp"
#include "BenzeneHtpEngine.hpp"
#include "BookBuilder.hpp"
#include "MoHexPlayer.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Htp engine for MoHex. */
class MoHexEngine : public BenzeneHtpEngine
{
public:

    MoHexEngine(std::istream& in, std::ostream& out, 
                int boardsize, BenzenePlayer& player);
    
    ~MoHexEngine();

    /** @name Command Callbacks */
    // @{

    // The callback functions are documented in the cpp file
    void CmdBookExpandParam(HtpCommand& cmd);
    void CmdBookPriorities(HtpCommand& cmd);
    void CmdBookExpand(HtpCommand& cmd);
    void CmdBookRefresh(HtpCommand& cmd);
    void CmdBookIncreaseWidth(HtpCommand& cmd);
    void CmdParamBook(HtpCommand& cmd);
    void MoHexParam(HtpCommand& cmd);

    // @} // @name

#if GTPENGINE_PONDER

    virtual void Ponder();

    virtual void InitPonder();

    virtual void StopPonder();
#endif

private:

    BookBuilder<MoHexPlayer> m_bookBuilder;

    void RegisterCmd(const std::string& name,
                     GtpCallback<MoHexEngine>::Method method);
};

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // MOHEXENGINE_HPP
