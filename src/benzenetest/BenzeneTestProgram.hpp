//----------------------------------------------------------------------------
/** @file BenzeneTestProgram.hpp */
//----------------------------------------------------------------------------

#ifndef BENZENETESTPROGRAM_HPP
#define BENZENETESTPROGRAM_HPP

#include "CommonProgram.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Starts up a BenzeneTest program. */
class BenzeneTestProgram : public CommonProgram
{
public:
    BenzeneTestProgram(std::string version, std::string buildDate);

    virtual ~BenzeneTestProgram();

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

#endif // BENZENETESTPROGRAM_HPP
