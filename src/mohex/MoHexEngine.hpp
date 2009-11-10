//----------------------------------------------------------------------------
/** @file MoHexEngine.hpp
 */
//----------------------------------------------------------------------------

#ifndef MOHEXENGINE_HPP
#define MOHEXEENGINE_HPP

#include "Hex.hpp"
#include "BenzeneHtpEngine.hpp"
#include "BookBuilderCommands.hpp"
#include "MoHexPlayer.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Htp engine for MoHex. */
class MoHexEngine : public BenzeneHtpEngine
{
public:

    MoHexEngine(GtpInputStream& in, GtpOutputStream& out,
                int boardsize, BenzenePlayer& player);
    
    ~MoHexEngine();

    /** @name Command Callbacks */
    // @{

    void MoHexParam(HtpCommand& cmd);
    void MoHexPolicyParam(HtpCommand& cmd);
    void SaveTree(HtpCommand& cmd);

    // @} // @name

#if GTPENGINE_PONDER

    virtual void Ponder();

    virtual void InitPonder();

    virtual void StopPonder();
#endif

    double TimeForMove(HexColor color);

private:

    BookBuilderCommands<MoHexPlayer> m_bookCommands;

    void RegisterCmd(const std::string& name,
                     GtpCallback<MoHexEngine>::Method method);
};

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // MOHEXENGINE_HPP
