//----------------------------------------------------------------------------
/** @file BookCommands.hpp */
//----------------------------------------------------------------------------

#ifndef BOOKCOMMANDS_HPP
#define BOOKCOMMANDS_HPP

#include "Book.hpp"
#include "BookCheck.hpp"
#include "Game.hpp"
#include "HexBoard.hpp"
#include "HexHtpEngine.hpp"
#include "HexEnvironment.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Commands for inspecting opening books. */
class BookCommands
{
public:
    BookCommands(Game& game, HexEnvironment& env, 
                 boost::scoped_ptr<Book>& m_book, BookCheck& bookCheck);

    ~BookCommands();

    void Register(GtpEngine& engine);

    void AddAnalyzeCommands(HtpCommand& cmd);

protected:
    Game& m_game;

    HexEnvironment& m_env;

    boost::scoped_ptr<Book>& m_book;

    BookCheck& m_bookCheck;

private:
    void Register(GtpEngine& engine, const std::string& command,
                  GtpCallback<BookCommands>::Method method);

    void CmdBookOpen(HtpCommand& cmd);
    void CmdBookClose(HtpCommand& cmd);
    void CmdBookStat(HtpCommand& cmd);
    void CmdBookMainLineDepth(HtpCommand& cmd);
    void CmdBookCounts(HtpCommand& cmd);
    void CmdBookScores(HtpCommand& cmd);
    void CmdBookVisualize(HtpCommand& cmd);
    void CmdBookDumpPolarizedLeafs(HtpCommand& cmd);
    void CmdBookImportSolvedStates(HtpCommand& cmd);
    void CmdBookSetValue(HtpCommand& cmd);
    void CmdBookParam(HtpCommand& cmd);
};

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // BOOKCOMMANDS_HPP
