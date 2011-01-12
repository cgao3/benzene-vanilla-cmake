//----------------------------------------------------------------------------
/** @file WolveProgram.hpp */
//----------------------------------------------------------------------------

#ifndef WOLVEPROGRAM_HPP
#define WOLVEPROGRAM_HPP

#include "CommonProgram.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Starts up a Wolver-player program. */
class WolveProgram : public CommonProgram
{
public:
    WolveProgram(std::string version, std::string buildDate);

    virtual ~WolveProgram();

    //-----------------------------------------------------------------------

    virtual void RegisterCmdLineArguments();

    virtual void HandleCmdLineArguments();

    virtual void InitializeSystem();

    virtual void ShutdownSystem();

    //-----------------------------------------------------------------------

private:
};

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // WOLVEPROGRAM_HPP
