//----------------------------------------------------------------------------
/** @file BookCommands.cpp */
//----------------------------------------------------------------------------

#include "BookBuilder.hpp"
#include "BookCommands.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

BookCommands::BookCommands(Game& game, HexEnvironment& env, 
                           boost::scoped_ptr<Book>& book, 
                           BookCheck& bookCheck)
    : m_game(game),
      m_env(env),
      m_book(book), 
      m_bookCheck(bookCheck)
{
}

BookCommands::~BookCommands()
{
}

void BookCommands::Register(GtpEngine& e)
{
    Register(e, "book-open", &BookCommands::CmdBookOpen);
    Register(e, "book-close", &BookCommands::CmdBookClose);
    Register(e, "book-stat", &BookCommands::CmdBookStat);
    Register(e, "book-depths", &BookCommands::CmdBookMainLineDepth);
    Register(e, "book-counts", &BookCommands::CmdBookCounts);
    Register(e, "book-scores", &BookCommands::CmdBookScores);
    Register(e, "book-visualize", &BookCommands::CmdBookVisualize);
    Register(e, "book-dump-polarized-leafs", 
             &BookCommands::CmdBookDumpPolarizedLeafs);
    Register(e, "book-import-solved", 
             &BookCommands::CmdBookImportSolvedStates);
    Register(e, "book-set-value", &BookCommands::CmdBookSetValue);
    Register(e, "param_book", &BookCommands::CmdBookParam);
}

void BookCommands::Register(GtpEngine& engine, const std::string& command,
                          GtpCallback<BookCommands>::Method method)
{
    engine.Register(command, new GtpCallback<BookCommands>(this, method));
}

void BookCommands::AddAnalyzeCommands(HtpCommand& cmd)
{
    cmd << 
        "none/Book Open/book-open %r\n"
        "none/Book Close/book-close\n"
        "string/Book Stats/book-stat\n"
        "pspairs/Book Depths/book-depths\n"
        "pspairs/Book Counts/book-counts\n"
        "pspairs/Book Scores/book-scores\n"
        "param/Book Param/param_book\n";  
}

//----------------------------------------------------------------------------

/** Opens/Creates an opening book for the current boardsize.
    Usage: "book-expand [filename]"
*/
void BookCommands::CmdBookOpen(HtpCommand& cmd)
{
    cmd.CheckNuArgLessEqual(2);
    std::string fn = cmd.Arg(0);
    try {
        m_book.reset(new Book(fn));
    }
    catch (BenzeneException& e) {
        cmd << "Error opening book: '" << e.what() << "'\n";
    }
}

/** Closes a book if one is open. */
void BookCommands::CmdBookClose(HtpCommand& cmd)
{
    cmd.CheckArgNone();
    if (m_book.get() == 0)
        throw HtpFailure() << "No open book.";        
    m_book.reset(0);
}

void BookCommands::CmdBookStat(HtpCommand& cmd)
{
    cmd.CheckNuArg(0);
    if (m_book.get() == 0)
        throw HtpFailure("No open book!\n");
    cmd << m_book->BDBStatistics();
}

void BookCommands::CmdBookMainLineDepth(HtpCommand& cmd)
{
    if (m_book.get() == 0) 
        throw HtpFailure() << "No open book.";
    HexState state(m_game.Board(), m_game.Board().WhoseTurn());
    for (BitsetIterator p(state.Position().GetEmpty()); p; ++p) 
    {
        state.PlayMove(*p);
        cmd << " " << *p << " " << BookUtil::GetMainLineDepth(*m_book, state);
        state.UndoMove(*p);
    }
}

void BookCommands::CmdBookCounts(HtpCommand& cmd)
{
    if (m_book.get() == 0) 
        throw HtpFailure() << "No open book.";
    HexState state(m_game.Board(), m_game.Board().WhoseTurn());
    for (BitsetIterator p(state.Position().GetEmpty()); p; ++p) 
    {
        state.PlayMove(*p);
        HexBookNode node;
        if (m_book->Get(state, node))
            cmd << " " << *p << " " << node.m_count;
        state.UndoMove(*p);
    }
}

void BookCommands::CmdBookScores(HtpCommand& cmd)
{
    if (m_book.get() == 0) 
        throw HtpFailure() << "No open book.";
    float countWeight = m_bookCheck.CountWeight();
    HexState state(m_game.Board(), m_game.Board().WhoseTurn());

    std::map<HexPoint, HexEval> values;
    std::map<HexPoint, unsigned> counts;
    std::vector<std::pair<float, HexPoint> > scores;
    for (BitsetIterator p(state.Position().GetEmpty()); p; ++p) 
    {
        state.PlayMove(*p);
        HexBookNode node;
        if (m_book->Get(state, node))
        {
            counts[*p] = node.m_count;
            values[*p] = BookUtil::InverseEval(BookUtil::Value(node, state));
            scores.push_back(std::make_pair
                             (-BookUtil::Score(node, state, countWeight), *p));
        }
        state.UndoMove(*p);
    }
    std::stable_sort(scores.begin(), scores.end());
    std::vector<std::pair<float, HexPoint> >::const_iterator it 
        = scores.begin();
    for (; it != scores.end(); ++it)
    {
        HexPoint p = it->second;
        HexEval value = values[p];
        cmd << ' ' << p;
        if (HexEvalUtil::IsWin(value))
            cmd << " W";
        else if (HexEvalUtil::IsLoss(value))
            cmd << " L";
        else
            cmd << " " << std::fixed << std::setprecision(3) << value;
        cmd << '@' << counts[p];
    }
}

void BookCommands::CmdBookVisualize(HtpCommand& cmd)
{
    if (m_book.get() == 0) 
        throw HtpFailure() << "No open book.";
    cmd.CheckNuArg(1);
    std::string filename = cmd.Arg(0);
    HexState state(m_game.Board(), m_game.Board().WhoseTurn());
    std::ofstream f(filename.c_str());
    if (!f)
        throw HtpFailure() << "Could not open file for output.";
    BookUtil::DumpVisualizationData(*m_book, state, 0, f);
    f.close();
}

/** Dumps variations leading to non-terminal leafs whose value is
    polarized. The ignore file is an optional argument that lists
    states that should not be ingored (not dumped again). 
    Usage:
      book-dump-polarized-leafs [polarization] [output file] { [ignore file] }
*/
void BookCommands::CmdBookDumpPolarizedLeafs(HtpCommand& cmd)
{
    if (m_book.get() == 0) 
        throw HtpFailure() << "No open book.";
    cmd.CheckNuArgLessEqual(3);
    float polarization = cmd.Arg<float>(0);
    std::string filename = cmd.Arg(1);
    StateSet ignoreSet;
    if (cmd.NuArg() == 3u)
    {
        std::string ignoreFile = cmd.Arg(2);
        HexState state(m_game.Board(), m_game.Board().WhoseTurn());
        std::ifstream ifs(ignoreFile.c_str()); 
        if (!ifs)
            throw HtpFailure() << "Could not open ignore file for reading.";
        std::string line;
        while (std::getline(ifs, line))
        {
            PointSequence seq;
            HexPointUtil::FromString(line, seq);
            if (!seq.empty())
            {
                state.Position().StartNewGame();
                state.SetToPlay(FIRST_TO_PLAY);
                for (std::size_t i = 0; i < seq.size(); ++i)
                    state.PlayMove(seq[i]);
                ignoreSet.Insert(state);
            }
        }
        LogInfo() << "Read " << ignoreSet.Size() << " positions to ignore.\n";
    }
    HexState state(m_game.Board(), m_game.Board().WhoseTurn());
    PointSequence pv;
    GameUtil::HistoryToSequence(m_game.History(), pv);
    std::ofstream f(filename.c_str());
    if (!f)
        throw HtpFailure() << "Could not open file for output.";
    BookUtil::DumpPolarizedLeafs(*m_book, state, polarization, pv, f, 
                                 ignoreSet);
    f.close();
}

/** Imports positions from file into book. */
void BookCommands::CmdBookImportSolvedStates(HtpCommand& cmd)
{
    if (m_book.get() == 0)
        throw HtpFailure() << "No open book.";
    cmd.CheckNuArg(1);
    std::string filename = cmd.Arg(0);
    std::ifstream f(filename.c_str());
    if (!f)
        throw HtpFailure() << "Could not open file for reading.";
    BookUtil::ImportSolvedStates(*m_book, m_game.Board().Const(), f);
    f.close();
}

/** Sets value of current state in the book.
    Usage:
      book-set-value [value]
    Where [value] can be one of W, L, or value in rage [0, 1]. 
 */
void BookCommands::CmdBookSetValue(HtpCommand& cmd)
{
    if (m_book.get() == 0) 
        throw HtpFailure() << "No open book.";
    float value = 0.5;
    std::string vstr = cmd.ArgToLower(0);
    if (vstr == "w")
        value = IMMEDIATE_WIN;
    else if (vstr == "l")
        value = IMMEDIATE_LOSS;
    else
        value = cmd.Arg<float>(0);
    HexBookNode node;
    HexState state(m_game.Board(), m_game.Board().WhoseTurn());
    if (!m_book->Get(state, node))
        m_book->Put(state, HexBookNode(value));
    else
    {
        node.m_value = value;
        m_book->Put(state, node);
    }
    m_book->Flush();
}

void BookCommands::CmdBookParam(HtpCommand& cmd)
{
    if (cmd.NuArg() == 0)
    {
        cmd << '\n'
            << "[string] book_count_weight "
            << m_bookCheck.CountWeight() << '\n'
            << "[string] book_min_count "
            << m_bookCheck.MinCount() << '\n';
    }
    else if (cmd.NuArg() == 2)
    {
        std::string name = cmd.Arg(0);
        if (name == "book_min_count")
            m_bookCheck.SetMinCount(cmd.ArgMin<unsigned>(1, 0));
        else if (name == "book_count_weight")
            m_bookCheck.SetCountWeight(cmd.Arg<float>(1));
        else 
            throw HtpFailure() << "Unknown parameter: " << name;
    }
    else 
        throw HtpFailure("Expected 0 ore 2 arguments");
}

//----------------------------------------------------------------------------
