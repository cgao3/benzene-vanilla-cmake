//----------------------------------------------------------------------------
/** @file MoHexEngine.hpp */
//----------------------------------------------------------------------------

#ifndef MOHEXENGINE_HPP
#define MOHEXEENGINE_HPP

#include "Hex.hpp"
#include "CommonHtpEngine.hpp"
#include "BookBuilderCommands.hpp"
#include "MoHexPlayer.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Htp engine for MoHex. */
class MoHexEngine : public CommonHtpEngine
{
public:

    MoHexEngine(int boardsize, MoHexPlayer& player);
    
    ~MoHexEngine();

    /** @name Command Callbacks */
    // @{

    void CmdAnalyzeCommands(HtpCommand& cmd);
    void MoHexParam(HtpCommand& cmd);
    void MoHexPolicyParam(HtpCommand& cmd);
    void SaveTree(HtpCommand& cmd);
    void SaveGames(HtpCommand& cmd);
    void GetPV(HtpCommand& cmd);
    void Values(HtpCommand& cmd);
    void RaveValues(HtpCommand& cmd);
    void Bounds(HtpCommand& cmd);
    void FindTopMoves(HtpCommand& cmd);

    // @} // @name

#if GTPENGINE_PONDER
    virtual void Ponder();

    virtual void InitPonder();

    virtual void StopPonder();
#endif

    double TimeForMove(HexColor color);

private:

    MoHexPlayer& m_player;

    boost::scoped_ptr<Book> m_book;
    
    BookCheck m_bookCheck;

    BookBuilderCommands<MoHexPlayer> m_bookCommands;

    HexPoint GenMove(HexColor color, bool useGameClock);

    HexPoint DoSearch(HexColor color, double maxTime);

    void RegisterCmd(const std::string& name,
                     GtpCallback<MoHexEngine>::Method method);
};

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // MOHEXENGINE_HPP
