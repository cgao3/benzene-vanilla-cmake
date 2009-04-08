//----------------------------------------------------------------------------
// $Id: main.cpp 1870 2009-01-28 00:45:23Z broderic $
//----------------------------------------------------------------------------

#include "SgSystem.h"

#include "config.h"

#include "carrier.h"
#include "misc.h"

#include "HexProgram.hpp"
#include "SixHtpEngine.hpp"

int main(int argc, char** argv)
{
    // Six specific initializations
    Carrier::init();  

    // start it up
    HexProgram& program = HexProgram::Get();
    program.SetInfo("Six", "0.5.4", __DATE__);
    program.Initialize(argc, argv);

    SixHtpEngine gh(std::cin, std::cout, program.BoardSize());
    gh.MainLoop();

    program.Shutdown();
    return 0;
}

//----------------------------------------------------------------------------
