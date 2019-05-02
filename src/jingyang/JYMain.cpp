//----------------------------------------------------------------------------
/** @file JYMain.cpp */
//----------------------------------------------------------------------------

#include "SgSystem.h"

#include "config.h"
#include "Misc.hpp"
#include "JYEngine.hpp"
#include "JYPlayer.hpp"
#include "JYProgram.hpp"
#include "SwapCheck.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

/** @page JY refers to Jing Yang's player using patterns

    @section classes Classes
    - JYEngine
    - JYPlayer
    - JYProgram
    - JYSearch

    @section htpcommands HTP Commands
    - @ref hexhtpenginecommands
    - @ref benzenehtpenginecommands
    - @ref jyhtpenginecommands
*/
//----------------------------------------------------------------------------

namespace {

const char* build_date = __DATE__;

}

//----------------------------------------------------------------------------

int main(int argc, char** argv)
{
    MiscUtil::FindProgramDir(argc, argv);

    CommonProgram com;
    com.Shutdown();

    JYProgram program(VERSION, build_date);
    BenzeneEnvironment::Get().RegisterProgram(program);
    program.Initialize(argc, argv);
    JYPlayer player;
    try
    {
        JYEngine gh(program.BoardSize(), player);
        std::string config = program.ConfigFileToExecute();
        if (config != "")
            gh.ExecuteFile(config);
        GtpInputStream gin(std::cin);
        GtpOutputStream gout(std::cout);
        gh.MainLoop(gin, gout);
    
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
