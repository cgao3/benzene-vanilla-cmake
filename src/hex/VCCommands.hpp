//----------------------------------------------------------------------------
/** @file VCCommands.hpp */
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

    void AddAnalyzeCommands(HtpCommand& cmd);
    
private:
    Game& m_game;

    HexEnvironment& m_env;
        
    void Register(GtpEngine& engine, const std::string& command,
                  GtpCallback<VCCommands>::Method method);

    void CmdGetBetweenFull(HtpCommand& cmd);
    void CmdGetBetweenSemi(HtpCommand& cmd);
    void CmdGetCellsConnectedToFull(HtpCommand& cmd);
    void CmdGetCellsConnectedToSemi(HtpCommand& cmd);
    void CmdGetMustPlay(HtpCommand& cmd);
    void CmdIntersectionFull(HtpCommand& cmd);
    void CmdIntersectionSemi(HtpCommand& cmd);
    void CmdUnionFull(HtpCommand& cmd);
    void CmdUnionSemi(HtpCommand& cmd);
    void CmdBuildStatic(HtpCommand& cmd);
    void CmdBuildIncremental(HtpCommand& cmd);
    void CmdUndoIncremental(HtpCommand& cmd);
    void CmdSetInfo(HtpCommand& cmd);
    void CmdBuilderStats(HtpCommand& cmd);
};

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // VCCOMMANDS_HPP
