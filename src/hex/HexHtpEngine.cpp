//----------------------------------------------------------------------------
/** @file HexHtpEngine.cpp */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "SgGameReader.h"
#include "SgNode.h"
#include "SgTimer.h"

#include "BenzeneProgram.hpp"
#include "BitsetIterator.hpp"
#include "BoardUtil.hpp"
#include "Groups.hpp"
#include "HexSgUtil.hpp"
#include "HexHtpEngine.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

HexHtpEngine::HexHtpEngine(int boardsize)
    : GtpEngine(),
      m_board(boardsize, boardsize),
      m_game(m_board),
      m_onLittleGolem(false)
{
    RegisterCmd("all_legal_moves", &HexHtpEngine::CmdAllLegalMoves);
    RegisterCmd("board_id", &HexHtpEngine::CmdBoardID);
    RegisterCmd("boardsize", &HexHtpEngine::CmdNewGame);
    RegisterCmd("clear_board", &HexHtpEngine::CmdClearBoard);
    RegisterCmd("exec", &HexHtpEngine::CmdExec);
    RegisterCmd("final_score", &HexHtpEngine::CmdFinalScore);
    RegisterCmd("genmove", &HexHtpEngine::CmdGenMove);
    RegisterCmd("hexgui-analyze_commands", 
                &HexHtpEngine::CmdAnalyzeCommands);
    RegisterCmd("reg_genmove", &HexHtpEngine::CmdRegGenMove);    
#if GTPENGINE_INTERRUPT
    RegisterCmd("gogui-interrupt", &HexHtpEngine::CmdInterrupt);
#endif
    RegisterCmd("loadsgf", &HexHtpEngine::CmdLoadSgf);
    RegisterCmd("name", &HexHtpEngine::CmdName);
    RegisterCmd("param_game", &HexHtpEngine::CmdParamGame);
    RegisterCmd("play", &HexHtpEngine::CmdPlay);
    RegisterCmd("play-game", &HexHtpEngine::CmdPlayGame);
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

void HexHtpEngine::CmdAnalyzeCommands(GtpCommand& cmd)
{
    cmd.CheckArgNone();
    cmd <<
        "param/Game Param/param_game\n"
        "plist/All Legal Moves/all_legal_moves %c\n"
        "string/ShowBoard/showboard\n"
        "string/BoardID/board_id\n"
        "string/Final Score/final_score\n"
        "varc/Reg GenMove/reg_genmove %c\n";
}

void HexHtpEngine::Play(HexColor color, HexPoint move)
{
    bool illegal = false;
    std::string reason = "";
    // Do nothing if a resign move
    if (move == RESIGN)
        return;
    Game::ReturnType result = m_game.PlayMove(color, move);
    if (result == Game::INVALID_MOVE) 
    {
        illegal = true;
        reason = " (invalid)";
    } 
    else if (result == Game::OCCUPIED_CELL) 
    {
        illegal = true;
        reason = " (occupied)";
    }
    if (illegal)
        throw HtpFailure() << "illegal move: " << ' '<< color 
                           << ' ' << move << reason;
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

/** Writes move to gtp command.
    Handles the special case of swap moves on LittleGolem: ie, LG
    expects 'swap' instead of 'swap-pieces' or 'swap-sides'. */
void HexHtpEngine::WriteMoveToGtp(HtpCommand& cmd, HexPoint move) const
{
    if (m_onLittleGolem && move == SWAP_PIECES)
        cmd << "swap";
    else 
        cmd << move;
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
    cmd << BenzeneEnvironment::Get().GetProgram().GetName();
}

/** Returns program's version. */
void HexHtpEngine::CmdVersion(HtpCommand& cmd)
{
    cmd << BenzeneEnvironment::Get().GetProgram().GetVersion();
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
        LogInfo() << "Errors occured.\n";
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
    int width = cmd.ArgMinMax<int>(0, 1, MAX_WIDTH);
    int height = width;
    if (cmd.NuArg() == 2)
        height = cmd.ArgMinMax<int>(1, 1, MAX_HEIGHT);
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

void HexHtpEngine::CmdPlayGame(HtpCommand& cmd)
{
    NewGame(m_game.Board().Width(), m_game.Board().Height());
    HexColor color = FIRST_TO_PLAY;
    for (std::size_t i = 0; i < cmd.NuArg(); ++i)
    {
        Play(color, HtpUtil::MoveArg(cmd, i));
        color = !color;
    }
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
        WriteMoveToGtp(cmd, move);
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
    cmd << '\n';
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
        cmd << "Black: " << m_game.TimeRemaining(BLACK) << ", " 
            << "White: " << m_game.TimeRemaining(WHITE);
    }
    else if (cmd.NuArg() == 1) 
    {
        HexColor color = HtpUtil::ColorArg(cmd, 0);
        cmd <<m_game.TimeRemaining(color);
    } 
    else 
    {
        HexColor color = HtpUtil::ColorArg(cmd, 0);
        m_game.SetTimeRemaining(color, cmd.ArgMin<float>(1, 0.0f));
    }
}

/** Returns a string with what we think the outcome of the game is.
    Value will be "B+" for a black win, and "W+" for a white win. */
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
    for (BitsetIterator i(m_game.Board().GetLegal()); i; ++i) 
        cmd << ' ' << *i;
}

/** Plays setup stones to the board.
    Plays all black moves first then all white moves as actuall game
    moves.
    @bug This will not work if the setup stones intersect previously
    played stones! The current only works if we expect only a single
    node with setup information. If multiple nodes in the game tree
    are adding/removing stones this will break horribly. */
void HexHtpEngine::SetPosition(const SgNode* node)
{
    std::vector<HexPoint> black, white, empty;
    HexSgUtil::GetSetupPosition(node, m_game.Board().Height(), 
                                black, white, empty);
    for (std::size_t i = 0; ; ++i) 
    {
        bool bdone = (i >= black.size());
        bool wdone = (i >= white.size());
        if (!bdone)
            Play(BLACK, black[i]);
        if (!wdone)
            Play(WHITE, white[i]);
        if (bdone && wdone)
            break;
    }
}

/** Loads game or position from given sgf. 
    Sets position to given move number or the last move of the game if
    none is given. */
void HexHtpEngine::CmdLoadSgf(HtpCommand& cmd)
{
    cmd.CheckNuArgLessEqual(2);
    std::string filename = cmd.Arg(0);
    int moveNumber = std::numeric_limits<int>::max();
    if (cmd.NuArg() == 2) 
        moveNumber = cmd.ArgMin<int>(1, 0);
    std::ifstream file(filename.c_str());
    if (!file)
        throw HtpFailure() << "cannot load file";
    SgGameReader sgreader(file, 11);
    SgNode* root = sgreader.ReadGame(); 
    if (root == 0)
        throw HtpFailure() << "cannot load file";
    sgreader.PrintWarnings(std::cerr);

    int size = root->GetIntProp(SG_PROP_SIZE);
    NewGame(size, size);
    const StoneBoard& brd = m_game.Board();
    if (HexSgUtil::NodeHasSetupInfo(root)) 
    {
        LogWarning() << "Root has setup info!\n";
        SetPosition(root);
    }
    // Play moveNumber moves; stop if we hit the end of the game
    SgNode* cur = root;
    for (int mn = 0; mn < moveNumber;)
    {
        cur = cur->NodeInDirection(SgNode::NEXT);
        if (!cur) 
            break;
        if (HexSgUtil::NodeHasSetupInfo(cur))
        {
            SetPosition(cur);
            continue;
        } 
        else if (!cur->HasNodeMove())
            continue;
        HexColor color = HexSgUtil::SgColorToHexColor(cur->NodePlayer());
        HexPoint point = HexSgUtil::SgPointToHexPoint(cur->NodeMove(), 
                                                      brd.Height());
        Play(color, point);
        ++mn;
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
            << "[bool] on_little_golem "
            << m_onLittleGolem << '\n'
            << "[string] game_time "
            << m_game.GameTime() << '\n';
    }
    else if (cmd.NuArg() == 2)
    {
        std::string name = cmd.Arg(0);
        if (name == "allow_swap")
            m_game.SetAllowSwap(cmd.Arg<bool>(1));
        else if (name == "on_little_golem")
            m_onLittleGolem = cmd.Arg<bool>(1);
        else if (name == "game_time")
        {
            if (!m_game.History().empty())
                throw HtpFailure("Cannot set game time if game started!");
            m_game.SetGameTime(cmd.ArgMin<float>(1, 0.0f));
        }
        else
            throw HtpFailure() << "Unknown parameter: " << name;
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
