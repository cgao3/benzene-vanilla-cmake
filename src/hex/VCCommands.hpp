//----------------------------------------------------------------------------
/** @file VCCommands.hpp
 */
//----------------------------------------------------------------------------

#ifndef VCCOMMANDS_HPP
#define VCCOMMANDS_HPP

#include "Game.hpp"
#include "HexBoard.hpp"
#include "HexHtpEngine.hpp"
#include "HexEnvironment.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Commands for building/inspecting virtual connections. */
class VCCommands
{
public:
    VCCommands(Game& game, HexEnvironment& env);

    ~VCCommands();

    void Register(GtpEngine& engine);

private:
    Game& m_game;

    HexEnvironment& m_env;
        
    VC::Type VCTypeArg(const HtpCommand& cmd, std::size_t number) const;
    void Register(GtpEngine& engine, const std::string& command,
                  GtpCallback<VCCommands>::Method method);

    void CmdGetVCsBetween(HtpCommand& cmd);
    void CmdGetCellsConnectedTo(HtpCommand& cmd);
    void CmdGetMustPlay(HtpCommand& cmd);
    void CmdVCIntersection(HtpCommand& cmd);
    void CmdVCUnion(HtpCommand& cmd);
    void CmdBuildStatic(HtpCommand& cmd);
    void CmdBuildIncremental(HtpCommand& cmd);
    void CmdUndoIncremental(HtpCommand& cmd);
    void CmdSetInfo(HtpCommand& cmd);
    void CmdBuilderStats(HtpCommand& cmd);
};

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // VCCOMMANDS_HPP
