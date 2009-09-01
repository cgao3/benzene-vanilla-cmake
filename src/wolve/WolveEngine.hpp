//----------------------------------------------------------------------------
/** @file WolveEngine.hpp
 */
//----------------------------------------------------------------------------

#ifndef WOLVEENGINE_HPP
#define WOLVEENGINE_HPP

#include "BenzeneHtpEngine.hpp"
#include "BookCommands.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Htp engine for Wolve. */
class WolveEngine : public BenzeneHtpEngine
{
public:

    WolveEngine(std::istream& in, std::ostream& out, 
                int boardsize, BenzenePlayer& player);
    
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

    BookCommands m_bookCommands;

    void RegisterCmd(const std::string& name,
                     GtpCallback<WolveEngine>::Method method);

};

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // WOLVEENGINE_HPP
