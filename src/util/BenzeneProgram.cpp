//----------------------------------------------------------------------------
/** @file BenzeneProgram.cpp */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "SgRandom.h"

#include "BenzeneException.hpp"
#include "BenzeneProgram.hpp"

#include <boost/program_options/cmdline.hpp>
#include <boost/program_options/parsers.hpp>

namespace po = boost::program_options;

using namespace benzene;

//----------------------------------------------------------------------------

namespace 
{
    std::ofstream g_logfile;
}

//----------------------------------------------------------------------------

BenzeneProgram::BenzeneProgram()
    : m_options_desc("Available Options"),
      m_use_logfile(false)
{
}

BenzeneProgram::~BenzeneProgram()
{
}

void BenzeneProgram::SetInfo(std::string name, std::string version, 
                             std::string date)
{
    m_name = name;
    m_version = version;
    m_date = date;
}

void BenzeneProgram::PrintStartupMessage()
{
    std::cerr <<
        m_name << " " << m_version << " " << m_date << "\n"
        "Copyright (C) 2007-2011 by the authors of the Benzene project.\n"
        "This program comes with ABSOLUTELY NO WARRANTY. This is\n"
        "free software and you are welcome to redistribute it under\n"
        "certain conditions. Type `benzene-license' for details.\n\n";
}

//----------------------------------------------------------------------------

void BenzeneProgram::RegisterCmdLineArguments()
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
        ("config", 
         po::value<std::string>(&m_config_file)->default_value(""),
         "Sets the config file to parse.")
        ("seed", 
         po::value<int>(&m_random_seed)->default_value(-1),
         "Sets the seed for the random number generator. "
         "(-1 for current time)");
}

void BenzeneProgram::InitLog()
{
    // remove default streams and add our own
    Logger::Global().ClearStreams();
    Logger::Global().AddStream(std::cerr, m_stderr_level);

    if (m_use_logfile)
    {
        g_logfile.open(m_logfile_name.c_str());
        if (!g_logfile)
            LogWarning() << "Could not open log file ('" 
                         << m_logfile_name << "') "
                         << "for writing! No log file will be used.\n";
    }
    if (g_logfile) 
    {
        LogLevel level = LogLevelUtil::fromString(m_logfile_level);
        Logger::Global().AddStream(g_logfile, level);
    } 
}

void BenzeneProgram::InitRandom()
{
    LogConfig() << "BenzeneProgram::InitRandom()\n";
    if (m_random_seed == -1)
        m_random_seed = static_cast<int>(time(NULL));
    LogConfig() << "Seed = " << m_random_seed << '\n';
    SgRandom::SetSeed(m_random_seed);
}

void BenzeneProgram::InitializeSystem()
{
    InitLog();
    LogConfig() << m_name << " v" << m_version << " " << m_date << ".\n";
    LogConfig() << "============ InitializeSystem ============\n";
    InitRandom();
}

void BenzeneProgram::Initialize(int argc, char **argv)
{
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
    ProcessCmdLineArguments(argc, argv);
    HandleCmdLineArguments();
    PrintStartupMessage();
    InitializeSystem();
}

void BenzeneProgram::ShutdownSystem()
{
    LogConfig() << "BenzeneProgram:: ShutdownSystem()\n";
}

//----------------------------------------------------------------------------

void BenzeneProgram::ShutdownLog()
{
    Logger::Global().Flush();
    if (g_logfile)
    {
        g_logfile << "Flushing and closing this stream...\n";
        g_logfile.flush();
    }
    g_logfile.close();
}

void BenzeneProgram::Shutdown()
{
    LogConfig() << "============ BenzeneShutdown =============\n";
    ShutdownSystem();
    ShutdownLog();
}

//----------------------------------------------------------------------------

void BenzeneProgram::ProcessCmdLineArguments(int argc, char** argv)
{
    try
    {
        po::store(po::parse_command_line(argc, argv, m_options_desc), m_vm);
        po::notify(m_vm);
    }
    catch(...)
    {
        Usage();
        exit(1);
    }
}

void BenzeneProgram::HandleCmdLineArguments()
{
    if (m_vm.count("usage") || m_vm.count("help"))
    {
        Usage();
        exit(1);
    }
    if (m_vm.count("version"))
    {
        std::cout << m_name << " v" << m_version << ' ' << m_date << "\n";
        exit(0);
    }
    m_stderr_level = LOG_LEVEL_INFO;
    if (m_vm.count("quiet"))
        m_stderr_level = LOG_LEVEL_OFF;
    if (m_vm.count("verbose"))
        m_stderr_level = LOG_LEVEL_ALL;
}

void BenzeneProgram::Usage() const
{
    std::cout << "Usage:\n" 
              << "       " << m_executable_name << " [Options]\n\n"
              << "[OPTIONS] is any number of the following:\n\n"
              << m_options_desc << '\n';
}

//----------------------------------------------------------------------------

BenzeneEnvironment::BenzeneEnvironment()
    : m_program(0)
{
}

BenzeneEnvironment::~BenzeneEnvironment()
{
}

BenzeneEnvironment& BenzeneEnvironment::Get()
{
    static BenzeneEnvironment env;
    return env;
}

void BenzeneEnvironment::RegisterProgram(BenzeneProgram& program)
{
    if (m_program)
        throw BenzeneException("Program already registered!");
    m_program = &program;
}

BenzeneProgram& BenzeneEnvironment::GetProgram() const
{
    if (m_program == 0)
        throw BenzeneException("No registered program!");
    return *m_program;
}

//----------------------------------------------------------------------------

void BenzeneAssertShutdown(const char* assertion, const char* file, int line,
                           const char* function)
{
    std::ostringstream os;
    os << file << ":" << line << ": " << function << ": "
       << "Assertion `" << assertion << "' failed.";
    LogSevere() << os.str() << '\n';
    BenzeneEnvironment::Get().GetProgram().Shutdown();
    abort();
}

//----------------------------------------------------------------------------
