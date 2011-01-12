//----------------------------------------------------------------------------
/** @file CommonProgram.cpp */
//----------------------------------------------------------------------------

#include "Decompositions.hpp"
#include "HexProp.hpp"
#include "CommonProgram.hpp"

#include <boost/program_options/cmdline.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>

namespace po = boost::program_options;

using namespace benzene;

//----------------------------------------------------------------------------

CommonProgram::CommonProgram()
{
}

CommonProgram::~CommonProgram()
{
}

//----------------------------------------------------------------------------

void CommonProgram::RegisterCmdLineArguments()
{
    m_options_desc.add_options()
        ("boardsize", 
         po::value<int>(&m_boardsize)->default_value(11),
         "Sets the size of the board.");
    BenzeneProgram::RegisterCmdLineArguments();
}

void CommonProgram::HandleCmdLineArguments()
{
    BenzeneProgram::HandleCmdLineArguments();
}

void CommonProgram::InitializeSystem()
{
    LogConfig() << "CommonProgram:: InitializeSystem()\n";
    BenzeneProgram::InitializeSystem();
    SgProp::Init();
    HexProp::Init();
    Decompositions::Initialize();
}

void CommonProgram::ShutdownSystem()
{
    LogConfig() << "CommonProgram:: ShutdownSystem()\n";
    BenzeneProgram::ShutdownSystem();
}

//----------------------------------------------------------------------------
