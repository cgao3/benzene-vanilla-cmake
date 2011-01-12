//----------------------------------------------------------------------------
/** @file CommonProgram.hpp */
//----------------------------------------------------------------------------

#ifndef COMMONPROGRAM_HPP
#define COMMONPROGRAM_HPP

#include "BenzeneProgram.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Program for playing Hex. */
class CommonProgram : public BenzeneProgram
{
public:
    CommonProgram();

    virtual ~CommonProgram();

    //-----------------------------------------------------------------------

    virtual void RegisterCmdLineArguments();

    virtual void HandleCmdLineArguments();

    virtual void InitializeSystem();

    virtual void ShutdownSystem();

    //-----------------------------------------------------------------------

    /** Boardsize as parsed from the cmd-line options. */
    int BoardSize() const;

private:
    int m_boardsize;
};

inline int CommonProgram::BoardSize() const
{
    return m_boardsize;
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // COMMONPROGRAM_HPP
