//----------------------------------------------------------------------------
/** @file MoHexProgram.hpp */
//----------------------------------------------------------------------------

#ifndef MOHEXPROGRAM_HPP
#define MOHEXPROGRAM_HPP

#include "CommonProgram.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Starts up a MoHex-player program. */
class MoHexProgram : public CommonProgram
{
public:
    MoHexProgram(std::string version, std::string buildDate);

    virtual ~MoHexProgram();

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

#endif // MOHEXPROGRAM_HPP
