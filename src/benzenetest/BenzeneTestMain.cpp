//----------------------------------------------------------------------------
/** @file BenzeneTestMain.cpp
 */
//----------------------------------------------------------------------------

#include "SgSystem.h"

#include "config.h"
#include "HexProgram.hpp"
#include "BenzeneTestEngine.hpp"
#include "SwapCheck.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

/** @page benzenetestmainpage BenzeneTest

    @section overview Overview

    @todo Add more documentation about BenzeneTest!
*/

//----------------------------------------------------------------------------

namespace {

const char* build_date = __DATE__;

}

//----------------------------------------------------------------------------

int main(int argc, char** argv)
{
    HexProgram& program = HexProgram::Get();
    program.SetInfo("BenzeneTest", VERSION, build_date);
    program.PrintStartupMessage();
    program.Initialize(argc, argv);
    try
    {
        GtpInputStream gin(std::cin);
        GtpOutputStream gout(std::cout);
        BenzeneTestEngine gh(gin, gout, program.BoardSize());
    
        std::string config = program.ConfigFileToExecute();
        if (config != "")
            gh.ExecuteFile(config);
        gh.MainLoop();
    
        program.Shutdown();
    }
    catch (const GtpFailure& f)
    {
        std::cerr << f.Response() << std::endl;
        return 1;
    }
    return 0;
}

//----------------------------------------------------------------------------
