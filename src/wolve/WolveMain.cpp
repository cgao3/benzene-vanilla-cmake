//----------------------------------------------------------------------------
/** @file WolveMain.cpp */
//----------------------------------------------------------------------------

#include "SgSystem.h"

#include "config.h"
#include "WolveEngine.hpp"
#include "WolvePlayer.hpp"
#include "WolveProgram.hpp"
#include "SwapCheck.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

/** @page wolvemainpage Wolve

    @section overview Overview

    Wolve is a traditional Six-like Hex player. It uses a truncated
    iterative deepening alpha-beta search with an electric ciruit
    evaluation function.

    @section search Search
    - WolveEngine
    - WolvePlayer
    - HexAbSearch
    - Resistance

    @section htpcommands HTP Commands
    - @ref hexhtpenginecommands
    - @ref benzenehtpenginecommands

    @todo Add more documentation about Wolve!
*/

//----------------------------------------------------------------------------

namespace {

const char* build_date = __DATE__;

}

//----------------------------------------------------------------------------

int main(int argc, char** argv)
{
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
