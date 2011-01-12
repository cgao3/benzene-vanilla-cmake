//----------------------------------------------------------------------------
/** @file BenzeneTestProgram.cpp */
//----------------------------------------------------------------------------

#include "BenzeneTestProgram.hpp"

#include <boost/program_options/cmdline.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>

namespace po = boost::program_options;

using namespace benzene;

//----------------------------------------------------------------------------

BenzeneTestProgram::BenzeneTestProgram(std::string version,
                           std::string buildDate)
{
    SetInfo("BenzeneTest", version, buildDate);
    RegisterCmdLineArguments();
}

BenzeneTestProgram::~BenzeneTestProgram()
{
}

//----------------------------------------------------------------------------

void BenzeneTestProgram::RegisterCmdLineArguments()
{
    CommonProgram::RegisterCmdLineArguments();
}

void BenzeneTestProgram::HandleCmdLineArguments()
{
    CommonProgram::HandleCmdLineArguments();
}

void BenzeneTestProgram::InitializeSystem()
{
    LogConfig() << "BenzeneTestProgram:: InitializeSystem()\n";
    CommonProgram::InitializeSystem();
}

void BenzeneTestProgram::ShutdownSystem()
{ 
    LogConfig() << "BenzeneTestProgram:: ShutdownSystem()\n";
    CommonProgram::ShutdownSystem();
}

//----------------------------------------------------------------------------
