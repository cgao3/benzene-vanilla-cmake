//----------------------------------------------------------------------------
// $Id: MoHexMain.cpp 1877 2009-01-29 00:57:27Z broderic $
//----------------------------------------------------------------------------

#include "SgSystem.h"

#include "config.h"
#include "HexProgram.hpp"
#include "PlayerFactory.hpp"
#include "MoHexEngine.hpp"
#include "MoHexPlayer.hpp"

const char* build_date = __DATE__;

//----------------------------------------------------------------------------

int main(int argc, char** argv)
{
    HexProgram& program = HexProgram::Get();
    program.SetInfo("Mohex", VERSION, build_date);
    program.Initialize(argc, argv);

    boost::scoped_ptr<BenzenePlayer> 
        player(PlayerFactory::CreatePlayerWithBook(new MoHexPlayer()));
   
    try
    {
        MoHexEngine gh(std::cin, std::cout, program.BoardSize(), *player);
    
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
