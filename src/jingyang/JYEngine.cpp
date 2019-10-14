//----------------------------------------------------------------------------
/** @file JYEngine.cpp */
//----------------------------------------------------------------------------

#include "SgSystem.h"

#include "BitsetIterator.hpp"
#include "Misc.hpp"
#include "PlayAndSolve.hpp"
#include "SwapCheck.hpp"
#include "JYEngine.hpp"
#include "BoardUtil.hpp"


using namespace benzene;

/*
 * print the current pattern list, which always covers the whole board
 * regions out of coveraged of those pattern list are not of interest, since
 * if white plays there, a random counter move can be responded by black
 */
std::string PrintCurrentPatternList(const HexState& state, JYPlayer &player)
{
    std::ostringstream os;
    if (player.m_cur_pattern_list.size()==1 && player.m_cur_pattern_list[0].RN==1){
        //since RN=1 contains only half empty cells on board
        int mid_point=player.m_boardsize*player.m_boardsize/2;
        for(int i=1;i<=player.m_boardsize*player.m_boardsize;i++){
            if(i==mid_point) continue;//center is occupied by first black move
            int local=i;
            if(i>mid_point) local=i-1;
            HexPoint hexpoint=RN1GlobalPoint(local, player.m_boardsize);
            if(state.Position().IsEmpty(hexpoint)){
                os <<' '<<hexpoint<<' '<<local<<"@1";
            }
        }
        return os.str();
    }
    for(JYPattern &pattern:player.m_cur_pattern_list){
        for(int i=0;i<pattern.BT;i++){
            for(size_t j=0;j<pattern.branchs[i].WM.size();j++){
                int local=pattern.branchs[i].WM[j].m_local;
                HexPoint hexPoint= static_cast<HexPoint>(pattern.branchs[i].WM[j].m_global);
                if(player.m_is_rotate180){
                    hexPoint=BoardUtil::Rotate(state.Position().Const(), hexPoint);
                }
                os << ' '<<hexPoint<<' '<<local<<'@'<<pattern.RN;
            }
        }
    }

    return os.str();
}

JYEngine::JYEngine(int boardsize, JYPlayer& player)
    : CommonHtpEngine(boardsize),
      m_player(player)
{
    RegisterCmd("load_pattern_file", &JYEngine::CmdLoadPatternFile);
    RegisterCmd("show_jypattern_list", &JYEngine::CmdShowJYPatternList);
    std::string default_pattern=std::string(ABS_TOP_SRCDIR)+"/share/hex99-3.txt";
    m_player.LoadPatterns(default_pattern);
    LogInfo()<<"\nDefault pattern file: "+default_pattern+"\n";
    LogInfo()<<"\nNum of lines in pattern file:"<< m_player.m_vc_str.size()<<"\n";
}

JYEngine::~JYEngine()
{
}

//----------------------------------------------------------------------------

void JYEngine::RegisterCmd(const std::string& name,
                              GtpCallback<JYEngine>::Method method)
{
    Register(name, new GtpCallback<JYEngine>(this, method));
}

//----------------------------------------------------------------------------

/** Generates a move. */
HexPoint JYEngine::GenMove(HexColor color, bool useGameClock)
{
    SG_UNUSED(useGameClock);
    if (SwapCheck::PlaySwap(m_game, color))
        return SWAP_PIECES;
    HexState state(m_game.Board(), color);
    double maxTime = 10.0;
    return DoSearch(color, maxTime);
}

void JYEngine::CmdUndo(benzene::HtpCommand &cmd) {
    if (m_player.m_prev_pattern_list_stack.size()>0 && m_game.Board().WhoseTurn() == HexColor::WHITE){
        m_player.m_cur_pattern_list=m_player.m_prev_pattern_list_stack.back();
        m_player.m_prev_pattern_list_stack.pop_back();
    }
    //Board is rotated after first white move, so undoing first white move must undo the rotation.
    if(m_game.History().size() < 3) 
        m_player.m_is_rotate180=false;
    HexHtpEngine::CmdUndo(cmd);
}

void JYEngine::CmdNewGame(HtpCommand& cmd){
    HexHtpEngine::CmdNewGame(cmd);
    m_player.m_cur_pattern_list.clear();
    m_player.m_prev_pattern_list_stack.clear();
    m_player.m_cur_pattern_list.push_back(m_player.m_all_patterns[1]);//add the first pattern
    m_player.m_is_rotate180=false;
}

void JYEngine::CmdClearBoard(HtpCommand& cmd){
    HexHtpEngine::CmdClearBoard(cmd);
    m_player.m_cur_pattern_list.clear();
    m_player.m_prev_pattern_list_stack.clear();
    m_player.m_cur_pattern_list.push_back(m_player.m_all_patterns[1]);//add the first pattern
    m_player.m_is_rotate180=false;
}

void JYEngine::CmdShowJYPatternList(HtpCommand& cmd){
    HexState state(m_game.Board(), m_game.Board().WhoseTurn());
    cmd<<PrintCurrentPatternList(state, m_player);
}

HexPoint JYEngine::DoSearch(HexColor color, double maxTime)
{
    HexState state(m_game.Board(), color);
    if (m_useParallelSolver)
    {
        PlayAndSolve ps(*m_pe.brd, *m_se.brd, m_player, m_dfpnSolver, 
                        m_dfpnPositions, m_game);
        return ps.GenMove(state, maxTime);
    }
    //return m_player.GenMove(state, m_game, m_pe.SyncBoard(m_game.Board()), maxTime, score);
    HexPoint move = INVALID_POINT;
    double score;
    Groups groups;
    GroupBuilder::Build(state.Position(), groups);
    if (groups.IsGameOver())
    {
        score = IMMEDIATE_LOSS;
        move=RESIGN;
    }
    if (move != INVALID_POINT)
        return move;
    return m_player.JYSearch(state, m_game);
}


void JYEngine::CmdAnalyzeCommands(HtpCommand& cmd)
{
    CommonHtpEngine::CmdAnalyzeCommands(cmd);
    cmd <<
        "pspairs/Show JY Pattern List/show_jypattern_list\n";
}

void JYEngine::CmdLoadPatternFile(HtpCommand& cmd)
{
    HexState state(m_game.Board(), m_game.Board().WhoseTurn());
    if(cmd.NuArg() ==0){
        cmd << '\n'
            << "Loaded pattern file: " << "\n";
    }
    if(cmd.NuArg()==1){
        std::string pattern_file=cmd.Arg(0);
        std::string full_path=std::string(ABS_TOP_SRCDIR)+"/share/"+pattern_file;
        m_player.LoadPatterns(full_path);
    }
}


//----------------------------------------------------------------------------
// Pondering

#if GTPENGINE_PONDER

void JYEngine::InitPonder()
{
    SgSetUserAbort(false);
}

void JYEngine::Ponder()
{

}

void JYEngine::StopPonder()
{
    SgSetUserAbort(true);
}

#endif // GTPENGINE_PONDER

//----------------------------------------------------------------------------
