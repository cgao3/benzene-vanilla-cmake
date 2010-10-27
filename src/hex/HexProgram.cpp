//----------------------------------------------------------------------------
/** @file HexProgram.cpp
 */
//----------------------------------------------------------------------------

#include "SgSystem.h"

#include "BoardUtils.hpp"
#include "HexProp.hpp"
#include "HexProgram.hpp"
#include "Resistance.hpp"
#include "RingGodel.hpp"
#include "Time.hpp"

#include <boost/program_options/cmdline.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>

namespace po = boost::program_options;

using namespace benzene;

//----------------------------------------------------------------------------

namespace 
{
    std::ofstream g_logfile;
}

//----------------------------------------------------------------------------

HexProgram::HexProgram()
    : m_initialized(false),
      m_options_desc("Options")
{
}

HexProgram::~HexProgram()
{
}

HexProgram& HexProgram::Get()
{
    static HexProgram program;
    return program;
}

void HexProgram::SetInfo(std::string name, std::string version, 
                         std::string date)
{
    m_name = name;
    m_version = version;
    m_date = date;
}

void HexProgram::PrintStartupMessage()
{
    std::cerr <<
        m_name << " " << m_version << " " << m_date << "\n"
        "Copyright (C) 2010 by the authors of the Benzene project.\n"
        "This program comes with ABSOLUTELY NO WARRANTY. This is\n"
        "free software and you are welcome to redistribute it under\n"
        "certain conditions. Type `benzene-license' for details.\n\n";
}

//----------------------------------------------------------------------------

void HexProgram::RegisterCmdLineArguments()
{
    m_options_desc.add_options()
        ("help", "Displays this usage information.")
        ("usage", "Displays this usage information.")
        ("version", "Displays version information.")
        ("quiet", "Suppresses log output to stderr.")
        ("verbose", "Displays more logging output to stderr.")
        ("use-logfile", 
         po::value<bool>(&m_use_logfile)->default_value(true),
         "Whether to use a .log file or not.")
        ("logfile-name", 
         po::value<std::string>(&m_logfile_name)->default_value("default.log"),
         "Specify name of log file.")
        ("logfile-level",
         po::value<std::string>(&m_logfile_level)->default_value("config"),
         "Message level for log file.")
        ("boardsize", 
         po::value<int>(&m_boardsize)->default_value(11),
         "Sets the size of the board.")
        ("config", 
         po::value<std::string>(&m_config_file)->default_value(""),
         "Sets the config file to parse.")
        ("seed", 
         po::value<int>(&m_random_seed)->default_value(-1),
         "Sets the seed for the random number generator. "
         "(-1 for current time)");
}

void HexProgram::InitLog()
{
    // remove default streams and add our own
    Logger::Global().ClearStreams();
    Logger::Global().AddStream(std::cerr, m_stderr_level);

    if (m_use_logfile)
    {
        g_logfile.open(m_logfile_name.c_str());
        if (!g_logfile)
            LogWarning() 
                     << "Could not open log file ('" << m_logfile_name << "') "
                     << "for writing! No log file will be used." << '\n';
    }
    if (g_logfile) 
    {
        LogLevel level = LogLevelUtil::fromString(m_logfile_level);
        Logger::Global().AddStream(g_logfile, level);
    } 
}

void HexProgram::InitRandom()
{
    LogConfig()<< "HexProgram::InitRandom()" << '\n';
    if (m_random_seed == -1) {
        m_random_seed = time(NULL);
    }
    LogConfig() << "Seed = " << m_random_seed << '\n';
    SgRandom::SetSeed(m_random_seed);
}

void HexProgram::InitializeHexSystem()
{
    InitLog();
    LogConfig() << m_name << " v" << m_version << " " << m_date << "." << '\n';
    LogConfig() << "============ InitializeHexSystem ============" << '\n';
    SgProp::Init();
    HexProp::Init();
    InitRandom();
    BoardUtils::InitializeDecompositions();
    ResistanceUtil::Initialize();
}

void HexProgram::Initialize(int argc, char **argv)
{
    if (m_initialized)
        return;

    // store the name of the executable
    m_executable_name = argv[0];
    
    // determine the executable directory
    {
        std::string path = m_executable_name;    
        std::string::size_type loc = path.rfind('/', path.length()-1);
        if (loc == std::string::npos) {
            path = "";
        } else {
            path = path.substr(0, loc);
            path.append("/");
        }
        m_executable_path = path;
    }

    RegisterCmdLineArguments();
    ProcessCmdLineArguments(argc, argv);
    InitializeHexSystem();
}

//----------------------------------------------------------------------------

void HexProgram::ShutdownLog()
{
    Logger::Global().Flush();
    if (g_logfile)
    {
        g_logfile << "Flushing and closing this stream...\n";
        g_logfile.flush();
    }
    g_logfile.close();
}

void HexProgram::Shutdown()
{
    LogConfig() << "============ HexShutdown =============" << '\n';
    ShutdownLog();
}

//----------------------------------------------------------------------------

void HexProgram::ProcessCmdLineArguments(int argc, char** argv)
{
    po::variables_map vm;
    try
    {
        po::store(po::parse_command_line(argc, argv, m_options_desc), vm);
        po::notify(vm);
    }
    catch(...)
    {
        Usage();
        Shutdown();
        exit(1);
    }
    
    if (vm.count("usage") || vm.count("help"))
    {
        Usage();
        Shutdown();
        exit(1);
    }

    if (vm.count("version"))
    {
        std::cout << m_name;
        std::cout << " v" << m_version;
        std::cout << " " << m_date << "." << std::endl;
        Shutdown();
        exit(0);
    }

    m_stderr_level = LOG_LEVEL_INFO;
    if (vm.count("quiet"))
        m_stderr_level = LOG_LEVEL_OFF;
    if (vm.count("verbose"))
        m_stderr_level = LOG_LEVEL_ALL;
    
}

void HexProgram::Usage() const
{
    std::cout << std::endl
              << "Usage: " 
              << std::endl 
              << "       " << m_executable_name << " [Options]" 
              << std::endl << std::endl;
    
    std::cout << "[OPTIONS] is any number of the following:"
              << std::endl << std::endl;
    
    std::cout << m_options_desc << std::endl;

    std::cout << std::endl;
}

//----------------------------------------------------------------------------

void HexAssertShutdown(const char* assertion, const char* file, int line,
                       const char* function)
{
    std::ostringstream os;
    os << file << ":" << line << ": " << function << ": "
       << "Assertion `" << assertion << "' failed.";
    LogSevere() << os.str() << '\n';
    
    HexProgram::Get().Shutdown();

    abort();
}

//----------------------------------------------------------------------------
