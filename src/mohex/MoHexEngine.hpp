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
    void PriorValues(HtpCommand& cmd);
    void Bounds(HtpCommand& cmd);
    void CellStats(HtpCommand& cmd);
    void DoPlayouts(HtpCommand& cmd);
    void PlayoutMove(HtpCommand& cmd);
    void PlayoutWeights(HtpCommand& cmd);
    void PlayoutGlobalWeights(HtpCommand& cmd);
    void PlayoutLocalWeights(HtpCommand& cmd);
    void FindTopMoves(HtpCommand& cmd);
    void SelfPlay(HtpCommand& cmd);
    void MarkPrunablePatterns(HtpCommand& cmd);

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

    const SgUctNode* FindState(const Game& game) const;

    void PerformPlayout(MoHexThreadState* thread,
                        const HexState& state,
                        const HexPoint lastMovePlayed);
};

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // MOHEXENGINE_HPP
