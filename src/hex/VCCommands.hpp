//----------------------------------------------------------------------------
/** @file VCCommands.hpp
 */
//----------------------------------------------------------------------------

#ifndef VCCOMMANDS_HPP
#define VCCOMMANDS_HPP

#include "HexBoard.hpp"
#include "HexHtpEngine.hpp"
#include "HexEnvironment.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Commands for building/inspecting virtual connections. */
class VCCommands
{
public:
    VCCommands(HexHtpEngine& engine, HexEnvironment& env);

    ~VCCommands();

    void Register();

private:
    HexHtpEngine& m_engine;

    HexEnvironment& m_env;
        
    VC::Type VCTypeArg(const HtpCommand& cmd, std::size_t number) const;
    void Register(const std::string& command,
                  GtpCallback<VCCommands>::Method method);

    void CmdGetVCsBetween(HtpCommand& cmd);
    void CmdGetCellsConnectedTo(HtpCommand& cmd);
    void CmdGetMustPlay(HtpCommand& cmd);
    void CmdVCIntersection(HtpCommand& cmd);
    void CmdVCUnion(HtpCommand& cmd);
    void CmdBuildStatic(HtpCommand& cmd);
    void CmdBuildIncremental(HtpCommand& cmd);
    void CmdUndoIncremental(HtpCommand& cmd);
};

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // VCCOMMANDS_HPP
