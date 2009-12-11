//----------------------------------------------------------------------------
/** @file MoHexMain.cpp
 */
//----------------------------------------------------------------------------

#include "SgSystem.h"

#include "config.h"
#include "HexProgram.hpp"
#include "MoHexEngine.hpp"
#include "MoHexPlayer.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

/** @page mohexmainpage MoHex

    @section overview Overview

    MoHex is Hex player that uses monte-carlo tree search with
    knowledge computation. It links with the UCT search classes from
    the Fuego library.
    
    MoHex uses HexBoard to compute VCs/fillin for nodes in the search
    tree that have been visited more than a certain threshold number
    of times.

    @section search Classes
    - MoHexEngine
    - MoHexPlayer
    - HexUctSearch

    @section htpcommands HTP Commands
    - @ref hexhtpenginecommands
    - @ref benzenehtpenginecommands

    @todo Add more documentation about MoHex!
*/

//----------------------------------------------------------------------------

namespace {

const char* build_date = __DATE__;

}

//----------------------------------------------------------------------------

int main(int argc, char** argv)
{
    HexProgram& program = HexProgram::Get();
    program.SetInfo("Mohex", VERSION, build_date);
    program.PrintStartupMessage();
    program.Initialize(argc, argv);
    MoHexPlayer player;
    try
    {
        GtpInputStream gin(std::cin);
        GtpOutputStream gout(std::cout);
        MoHexEngine gh(gin, gout, program.BoardSize(), player);
    
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
