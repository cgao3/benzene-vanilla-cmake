//----------------------------------------------------------------------------
/** @file JYProgram.hpp */
//----------------------------------------------------------------------------

#ifndef JYPROGRAM_HPP
#define JYPROGRAM_HPP

#include "CommonProgram.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Starts up a Jing Yang's player program. */
class JYProgram : public CommonProgram
{
public:
    JYProgram(std::string version, std::string buildDate);

    virtual ~JYProgram();

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

#endif // JYPROGRAM_HPP
