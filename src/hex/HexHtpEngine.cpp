//----------------------------------------------------------------------------
/** @file HexHtpEngine.cpp
 */
//----------------------------------------------------------------------------

#include "SgSystem.h"

#include <cmath>
#include <iomanip>
#include <sstream>
#include <limits>
#include <time.h>
#include <signal.h>
#include <iostream>

#include "SgGameReader.h"
#include "SgNode.h"
#include "SgTimer.h"

#include "BitsetIterator.hpp"
#include "BoardUtils.hpp"
#include "Groups.hpp"
#include "HexSgUtil.hpp"
#include "HexProgram.hpp"
#include "HexHtpEngine.hpp"
#include "Time.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

HexHtpEngine::HexHtpEngine(GtpInputStream& in, GtpOutputStream& out,
                           int boardsize)
    : GtpEngine(in, out),
      m_board(boardsize, boardsize),
      m_game(m_board)
{
    RegisterCmd("all_legal_moves", &HexHtpEngine::CmdAllLegalMoves);
    RegisterCmd("board_id", &HexHtpEngine::CmdBoardID);
    RegisterCmd("boardsize", &HexHtpEngine::CmdNewGame);
    RegisterCmd("clear_board", &HexHtpEngine::CmdClearBoard);
    RegisterCmd("exec", &HexHtpEngine::CmdExec);
    RegisterCmd("final_score", &HexHtpEngine::CmdFinalScore);
    RegisterCmd("genmove", &HexHtpEngine::CmdGenMove);
    RegisterCmd("reg_genmove", &HexHtpEngine::CmdRegGenMove);    
#if GTPENGINE_INTERRUPT
    RegisterCmd("gogui-interrupt", &HexHtpEngine::CmdInterrupt);
#endif
    RegisterCmd("loadsgf", &HexHtpEngine::CmdLoadSgf);
    RegisterCmd("name", &HexHtpEngine::CmdName);
    RegisterCmd("param_game", &HexHtpEngine::CmdParamGame);
    RegisterCmd("play", &HexHtpEngine::CmdPlay);
    RegisterCmd("showboard", &HexHtpEngine::CmdShowboard);
    RegisterCmd("time_left", &HexHtpEngine::CmdTimeLeft);
    RegisterCmd("undo", &HexHtpEngine::CmdUndo);
    RegisterCmd("version", &HexHtpEngine::CmdVersion);

    NewGame(m_board.Width(), m_board.Height());
}

HexHtpEngine::~HexHtpEngine()
{
}

//----------------------------------------------------------------------------

void HexHtpEngine::RegisterCmd(const std::string& name,
                               GtpCallback<HexHtpEngine>::Method method)
{
    Register(name, new GtpCallback<HexHtpEngine>(this, method));
}

void HexHtpEngine::Play(HexColor color, HexPoint move)
{
    bool illegal = false;
    std::string reason = "";

    // do nothing if a resign move
    if (move == RESIGN)
        return;

    Game::ReturnType result = m_game.PlayMove(color, move);
    if (result == Game::INVALID_MOVE) {
        illegal = true;
        reason = " (invalid)";
    } else if (result == Game::OCCUPIED_CELL) {
        illegal = true;
        reason = " (occupied)";
    }
    
    if (illegal) {
        throw HtpFailure() << "illegal move: " << ' '
                           << color << ' ' << move << reason;
    }
}

void HexHtpEngine::NewGame(int width, int height)
{
    if (width != m_game.Board().Width() || 
        height != m_game.Board().Height()) 
    {
        m_board = StoneBoard(width, height);
        m_game.SetBoard(m_board);
    } 
    m_game.NewGame();
}

void HexHtpEngine::BeforeHandleCommand()
{
    SgSetUserAbort(false);
}

void HexHtpEngine::BeforeWritingResponse()
{
}

//----------------------------------------------------------------------------

/** Returns program's name. */
void HexHtpEngine::CmdName(HtpCommand& cmd)
{
    cmd << HexProgram::Get().getName();
}

/** Returns program's version. */
void HexHtpEngine::CmdVersion(HtpCommand& cmd)
{
    cmd << HexProgram::Get().getVersion();
}

/** Executes HTP commands contained in given file. */
void HexHtpEngine::CmdExec(HtpCommand& cmd)
{
    cmd.CheckNuArg(1);
    std::string filename = cmd.Arg(0);

    try {
        ExecuteFile(filename, std::cerr);
    }
    catch (std::exception& e) {
        LogInfo() << "Errors occured." << '\n';
    }
}

#if GTPENGINE_INTERRUPT

/** Does nothing, but lets gogui know we can be interrupted with the 
    "# interrupt" gtp command. */
void HexHtpEngine::CmdInterrupt(HtpCommand& cmd)
{
    cmd.CheckArgNone();
}

#endif

/** Starts new game with the given board size. */
void HexHtpEngine::CmdNewGame(HtpCommand& cmd)
{
    cmd.CheckNuArgLessEqual(2);
    if (cmd.NuArg() == 0)
        throw HtpFailure() << "Must specify board dimensions!";
    int width = cmd.IntArg(0, 1, MAX_WIDTH);
    int height = width;
    if (cmd.NuArg() == 2)
        height = cmd.IntArg(1, 1, MAX_HEIGHT);
    NewGame(width, height);
}

/** Starts a new game with the same board size. */
void HexHtpEngine::CmdClearBoard(HtpCommand& cmd)
{
    cmd.CheckArgNone();
    NewGame(m_board.Width(), m_board.Height());
}

/** Plays a move. */
void HexHtpEngine::CmdPlay(HtpCommand& cmd)
{
    cmd.CheckNuArg(2);
    Play(HtpUtil::ColorArg(cmd, 0), HtpUtil::MoveArg(cmd, 1));
}

/** Generates a move and handles time remaining. */
void HexHtpEngine::CmdGenMove(HtpCommand& cmd)
{
    cmd.CheckNuArg(1);
    if (GameUtil::IsGameOver(m_game))
        cmd << RESIGN;
    else
    {
        HexColor color = HtpUtil::ColorArg(cmd, 0);
        SgTime::SetDefaultMode(SG_TIME_REAL);
        SgTimer timer;
        timer.Start();
        double oldTimeRemaining = m_game.TimeRemaining(color);
        HexPoint move = GenMove(color, true);
        timer.Stop();
        
        m_game.SetTimeRemaining(color, oldTimeRemaining - timer.GetTime());
        if (m_game.TimeRemaining(color) < 0)
            LogWarning() << "**** FLAG DROPPED ****\n";
        
        Play(color, move);
        cmd << move;
    }
}

/** Generates a move, but does not play it. Sets random seed. */
void HexHtpEngine::CmdRegGenMove(HtpCommand& cmd)
{
    cmd.CheckNuArg(1);
    SgRandom::SetSeed(SgRandom::Seed());
    if (GameUtil::IsGameOver(m_game))
        cmd << RESIGN;
    else
    {
        HexPoint move = GenMove(HtpUtil::ColorArg(cmd, 0), false);
        cmd << move;
    }
}

/** Undo the last move. */
void HexHtpEngine::CmdUndo(HtpCommand& cmd)
{
    cmd.CheckNuArg(0);
    m_game.UndoMove();
}

/** Displays the board. */
void HexHtpEngine::CmdShowboard(HtpCommand& cmd)
{
    cmd << "\n";
    cmd << m_game.Board();
}

/** Outputs BoardID of current position. */
void HexHtpEngine::CmdBoardID(HtpCommand& cmd)
{
    cmd.CheckNuArg(0);
    cmd << m_game.Board().GetBoardIDString();
}

/** Displays time left for both players or given player. */
void HexHtpEngine::CmdTimeLeft(HtpCommand& cmd)
{
    cmd.CheckNuArgLessEqual(2);
    if (cmd.NuArg() == 0) 
    {
        cmd << "Black: " << Time::Formatted(m_game.TimeRemaining(BLACK)) 
            << ", "
            << "White: " << Time::Formatted(m_game.TimeRemaining(WHITE));
    }
    else if (cmd.NuArg() == 1) 
    {
        HexColor color = HtpUtil::ColorArg(cmd, 0);
        cmd << Time::Formatted(m_game.TimeRemaining(color));
    } 
    else 
    {
        HexColor color = HtpUtil::ColorArg(cmd, 0);
        m_game.SetTimeRemaining(color, cmd.IntArg(1));
    }
}

/** Returns a string with what we think the outcome of the game is.
    Value will be "B+" for a black win, and "W+" for a white win.
 */
void HexHtpEngine::CmdFinalScore(HtpCommand& cmd)
{
    Groups groups;
    GroupBuilder::Build(m_game.Board(), groups);
    HexColor winner = groups.GetWinner();
    std::string ret = "cannot score";
    if (winner == BLACK)
        ret = "B+";
    else if (winner == WHITE)
        ret = "W+";
    cmd << ret;
}

/** Returns a list of all legal moves on current board position. */
void HexHtpEngine::CmdAllLegalMoves(HtpCommand& cmd)
{
    int c = 0;
    bitset_t legal = m_game.Board().GetLegal();
    for (BitsetIterator i(legal); i; ++i) 
    {
        cmd << " " << *i;
        if ((++c % 10) == 0) cmd << "\n";
    }
}

/** @bug This won't work if we're overwriting previosly played stones! */
void HexHtpEngine::SetPosition(const SgNode* node)
{
    std::vector<HexPoint> black, white, empty;
    HexSgUtil::GetSetupPosition(node, m_game.Board().Height(), 
                                black, white, empty);
    for (unsigned i=0; ; ++i) 
    {
        bool bdone = (i >= black.size());
        bool wdone = (i >= white.size());
        if (!bdone) Play(BLACK, black[i]);
        if (!wdone) Play(WHITE, white[i]);
        if (bdone && wdone) break;
    }
}

/** Loads game or position from given sgf. Sets position to given move
    number or the last move of the game if none is given.
*/
void HexHtpEngine::CmdLoadSgf(HtpCommand& cmd)
{
    cmd.CheckNuArgLessEqual(2);
    std::string filename = cmd.Arg(0);
    int movenumber = 1024;
    if (cmd.NuArg() == 2) 
        movenumber = cmd.IntArg(1, 0);

    std::ifstream file(filename.c_str());
    if (!file) {
        throw HtpFailure() << "cannot load file";
        return;
    }

    SgGameReader sgreader(file, 11);
    SgNode* root = sgreader.ReadGame(); 
    if (root == 0) {
        throw HtpFailure() << "cannot load file";
        return;
    }
    sgreader.PrintWarnings(std::cerr);

    int size = root->GetIntProp(SG_PROP_SIZE);

    NewGame(size, size);

    const StoneBoard& brd = m_game.Board();

    if (HexSgUtil::NodeHasSetupInfo(root)) {
        LogWarning() << "Root has setup info!" << '\n';
        SetPosition(root);
    }

    // play movenumber moves; stop if we hit the end of the game
    SgNode* cur = root;
    for (int mn=0; mn<movenumber;) {
        cur = cur->NodeInDirection(SgNode::NEXT);
        if (!cur) break;

        if (HexSgUtil::NodeHasSetupInfo(cur)) {
            SetPosition(cur);
            continue;
        } else if (!cur->HasNodeMove()) {
            continue;
        }

        HexColor color = HexSgUtil::SgColorToHexColor(cur->NodePlayer());
        HexPoint point = HexSgUtil::SgPointToHexPoint(cur->NodeMove(), 
                                                      brd.Height());
        Play(color, point);
        mn++;
    }
}

/** Displays/changes parameters relating to the current game. 

    Parameters:
    @arg @c allow_swap See Game::AllowSwap
    @arg @c game_time See Game::GameTime
*/
void HexHtpEngine::CmdParamGame(HtpCommand& cmd)
{
    if (cmd.NuArg() == 0)
    {
        cmd << "\n"
            << "[bool] allow_swap "
            << m_game.AllowSwap() << '\n'
            << "[string] game_time "
            << m_game.GameTime() << '\n';
    }
    else if (cmd.NuArg() == 2)
    {
        std::string name = cmd.Arg(0);
        if (name == "allow_swap")
            m_game.SetAllowSwap(cmd.BoolArg(1));
        else if (name == "game_time")
        {
            if (!m_game.History().empty())
                throw HtpFailure("Cannot set game time if game started!");
            m_game.SetGameTime(cmd.FloatArg(1));
        }
    }
    else
        throw HtpFailure("Expected 0 or 2 arguments");
}

#if GTPENGINE_INTERRUPT

void HexHtpEngine::Interrupt()
{
    SgSetUserAbort(true);
}

#endif // GTPENGINE_INTERRUPT

//----------------------------------------------------------------------------

HexColor HtpUtil::ColorArg(const HtpCommand& cmd, std::size_t number)
{
    std::string value = cmd.ArgToLower(number);
    if (value == "e" || value == "empty")
            return EMPTY;
    if (value == "b" || value == "black")
	    return BLACK;
    if (value == "w" || value == "white")
	    return WHITE;
    throw HtpFailure() << "argument " << (number + 1) << " must be color";
}

HexPoint HtpUtil::MoveArg(const HtpCommand& cmd, std::size_t number)
{
    return HexPointUtil::FromString(cmd.ArgToLower(number));
}

//----------------------------------------------------------------------------
