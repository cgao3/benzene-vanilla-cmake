//----------------------------------------------------------------------------
/** @file MoHexProgram.cpp */
//----------------------------------------------------------------------------

#include "MoHexProgram.hpp"

#include <boost/program_options/cmdline.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>

namespace po = boost::program_options;

using namespace benzene;

//----------------------------------------------------------------------------

MoHexProgram::MoHexProgram(std::string version,
                           std::string buildDate)
{
    SetInfo("MoHex", version, buildDate);
    RegisterCmdLineArguments();
}

MoHexProgram::~MoHexProgram()
{
}

//----------------------------------------------------------------------------

void MoHexProgram::RegisterCmdLineArguments()
{
    CommonProgram::RegisterCmdLineArguments();
}

void MoHexProgram::HandleCmdLineArguments()
{
    CommonProgram::HandleCmdLineArguments();
}

void MoHexProgram::InitializeSystem()
{
    LogConfig() << "MoHexProgram:: InitializeSystem()\n";
    CommonProgram::InitializeSystem();
}

void MoHexProgram::ShutdownSystem()
{ 
    LogConfig() << "MoHexProgram:: ShutdownSystem()\n";
    CommonProgram::ShutdownSystem();
}

//----------------------------------------------------------------------------
