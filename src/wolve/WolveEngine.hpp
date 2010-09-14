//----------------------------------------------------------------------------
/** @file WolveEngine.hpp
 */
//----------------------------------------------------------------------------

#ifndef WOLVEENGINE_HPP
#define WOLVEENGINE_HPP

#include "BenzeneHtpEngine.hpp"
#include "BookCheck.hpp"
#include "BookCommands.hpp"
#include "CacheBook.hpp"
#include "WolvePlayer.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Htp engine for Wolve. */
class WolveEngine : public BenzeneHtpEngine
{
public:

    WolveEngine(GtpInputStream& in, GtpOutputStream& out, 
                int boardsize, WolvePlayer& player);
    
    ~WolveEngine();

    /** @name Command Callbacks */
    // @{

    // The callback functions are documented in the cpp file
    void WolveParam(HtpCommand& cmd);

    // @} // @name

#if GTPENGINE_PONDER
    virtual void Ponder();
    virtual void InitPonder();
    virtual void StopPonder();
#endif

private:

    WolvePlayer& m_player;

    boost::scoped_ptr<Book> m_book;

    BookCheck m_bookCheck;

    BookCommands m_bookCommands;

    CacheBook m_cacheBook;

    double TimeForMove(HexColor color);

    HexPoint GenMove(HexColor color, bool useGameClock);

    void RegisterCmd(const std::string& name,
                     GtpCallback<WolveEngine>::Method method);

};

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // WOLVEENGINE_HPP
