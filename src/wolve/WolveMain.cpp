//----------------------------------------------------------------------------
/** @file WolveMain.cpp */
//----------------------------------------------------------------------------

#include "SgSystem.h"

#include "config.h"
#include "Misc.hpp"
#include "WolveEngine.hpp"
#include "WolvePlayer.hpp"
#include "WolveProgram.hpp"
#include "SwapCheck.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

/** @page wolvemainpage Wolve

    Wolve is a traditional alpha-beta Hex player. 

    Wolve uses a truncated iterative deepening alpha-beta search with
    an electric ciruit evaluation function. 

    Much of Wolve's design was inspired by the program <a
    href="http://six.retes.hu/">Six</a> written by <a
    href="http://quotenil.com/">Gabor Melis</a>.

    @section classes Classes
    - WolveEngine
    - WolvePlayer
    - WolveProgram
    - WolveSearch
    - WolveTimeControl
    - Resistance

    @section htpcommands HTP Commands
    - @ref hexhtpenginecommands
    - @ref benzenehtpenginecommands
    - @ref wolvehtpenginecommands

    @see
    - @ref wolvetimecontrol
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

    WolveProgram program(VERSION, build_date);
    BenzeneEnvironment::Get().RegisterProgram(program);
    program.Initialize(argc, argv);
    WolvePlayer player;
    try
    {
        WolveEngine gh(program.BoardSize(), player);
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
