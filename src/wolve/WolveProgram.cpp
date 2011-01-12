//----------------------------------------------------------------------------
/** @file WolveProgram.cpp */
//----------------------------------------------------------------------------

#include "WolveProgram.hpp"

#include <boost/program_options/cmdline.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>

namespace po = boost::program_options;

using namespace benzene;

//----------------------------------------------------------------------------

WolveProgram::WolveProgram(std::string version,
                           std::string buildDate)
{
    SetInfo("Wolve", version, buildDate);
    RegisterCmdLineArguments();
}

WolveProgram::~WolveProgram()
{
}

//----------------------------------------------------------------------------

void WolveProgram::RegisterCmdLineArguments()
{
    CommonProgram::RegisterCmdLineArguments();
}

void WolveProgram::HandleCmdLineArguments()
{
    CommonProgram::HandleCmdLineArguments();
}

void WolveProgram::InitializeSystem()
{
    LogConfig() << "WolveProgram:: InitializeSystem()\n";
    CommonProgram::InitializeSystem();
}

void WolveProgram::ShutdownSystem()
{ 
    LogConfig() << "WolveProgram:: ShutdownSystem()\n";
    CommonProgram::ShutdownSystem();
}

//----------------------------------------------------------------------------
