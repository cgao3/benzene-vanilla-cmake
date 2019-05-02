//----------------------------------------------------------------------------
/** @file JYProgram.cpp */
//----------------------------------------------------------------------------

#include "JYProgram.hpp"

#include <boost/program_options/cmdline.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>

namespace po = boost::program_options;

using namespace benzene;

//----------------------------------------------------------------------------

JYProgram::JYProgram(std::string version,
                           std::string buildDate)
{
    SetInfo("JY", version, buildDate);
    RegisterCmdLineArguments();
}

JYProgram::~JYProgram()
{
}

//----------------------------------------------------------------------------

void JYProgram::RegisterCmdLineArguments()
{
    CommonProgram::RegisterCmdLineArguments();
}

void JYProgram::HandleCmdLineArguments()
{
    CommonProgram::HandleCmdLineArguments();
}

void JYProgram::InitializeSystem()
{
    LogConfig() << "JYProgram:: InitializeSystem()\n";
    CommonProgram::InitializeSystem();
}

void JYProgram::ShutdownSystem()
{ 
    LogConfig() << "JYProgram:: ShutdownSystem()\n";
    CommonProgram::ShutdownSystem();
}

//----------------------------------------------------------------------------
