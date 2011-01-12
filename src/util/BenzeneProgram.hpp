//----------------------------------------------------------------------------
/** @file BenzeneProgram.hpp */
//----------------------------------------------------------------------------

#ifndef BENZENEPROGRAM_HPP
#define BENZENEPROGRAM_HPP

#include <map>
#include <string>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>

#include "Logger.hpp"
#include "Benzene.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Program for playing Hex. 
    Parses command-line arguments and initializes the system. */
class BenzeneProgram
{
public:
    BenzeneProgram();

    virtual ~BenzeneProgram();

    //-----------------------------------------------------------------------

    virtual void InitializeSystem();

    virtual void ShutdownSystem();

    virtual void RegisterCmdLineArguments();

    virtual void HandleCmdLineArguments();

    //-----------------------------------------------------------------------

    /** Parses cmd-line arguments. 
        Calls parses command-line arguments then InitializeSystem(). */
    void Initialize(int argc, char** argv);

    /** Shuts down the program safely. 
        Calls ShutdownSystem(). */
    void Shutdown();

    //-----------------------------------------------------------------------

    /** Sets the name, version, etc, for this program. */
    void SetInfo(std::string name, std::string version, std::string date);

    /** Prints program information and some license details. */
    void PrintStartupMessage();

    /** Prints all registered cmd-line arguments and their usage. */
    void Usage() const;

    /** Returns the name of the program. */
    std::string GetName() const;

    /** Returns the version string of the program. */
    std::string GetVersion() const;

    /** Returns the build number of the program. */
    std::string GetBuild() const;

    /** Returns the build date of the program. */
    std::string GetDate() const;

    //-----------------------------------------------------------------------

    /** Returns the configuration file that should be parsed.  This
        will be non-empty if the cmd-line option '--config' was parsed
        during the call to Initialize(). */
    std::string ConfigFileToExecute() const;

protected:
    boost::program_options::options_description m_options_desc;

    boost::program_options::variables_map m_vm;

private:
    std::string m_name;

    std::string m_version;

    std::string m_date;

    std::string m_executable_name;

    std::string m_executable_path;

    int m_random_seed;

    bool m_use_logfile;

    std::string m_logfile_name;

    std::string m_logfile_level;

    std::string m_config_file;

    LogLevel m_stderr_level;

    void InitLog();
    
    void InitRandom();

    void ProcessCmdLineArguments(int argc, char** argv);

    void ShutdownLog();
};

inline std::string BenzeneProgram::GetName() const
{
    return m_name;
}

inline std::string BenzeneProgram::GetVersion() const
{
    return m_version;
}

inline std::string BenzeneProgram::GetDate() const
{
    return m_date;
}

inline std::string BenzeneProgram::ConfigFileToExecute() const
{
    return m_config_file;
}

//----------------------------------------------------------------------------

class BenzeneEnvironment
{
public:
    static BenzeneEnvironment& Get();

    void RegisterProgram(BenzeneProgram& program);

    BenzeneProgram& GetProgram() const;

private:
    BenzeneProgram* m_program;

    BenzeneEnvironment();

    ~BenzeneEnvironment();
};

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // BENZENEPROGRAM_HPP
