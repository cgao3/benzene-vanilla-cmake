//----------------------------------------------------------------------------
/** @file HexProgram.hpp
 */
//----------------------------------------------------------------------------

#ifndef HEXPROGRAM_HPP
#define HEXPROGRAM_HPP

#include <map>
#include <string>

#include <boost/utility.hpp>
#include <boost/program_options/options_description.hpp>

#include "Logger.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Program for playing Hex. 
    Parses command-line arguments and initializes the Hex system.
*/
class HexProgram : private boost::noncopyable
{
public:

    /** Creates new HexProgram if non exists, or returns existing
        instance. */
    static HexProgram& Get();

    //-----------------------------------------------------------------------

    /** Parses cmd-line arguments, starts up Hex system, etc.  Does
        nothing if called a second time. */
    virtual void Initialize(int argc, char** argv);

    /** Shuts down the program and the Hex system. */
    virtual void Shutdown();

    //-----------------------------------------------------------------------

    /** Sets the name, version, etc, for this program. */
    void SetInfo(std::string name, std::string version, std::string date);

    /** Prints program information and some license details. */
    void PrintStartupMessage();

    /** Returns the name of the program. */
    std::string getName() const;

    /** Returns the version string of the program. */
    std::string getVersion() const;

    /** Returns the build number of the program. */
    std::string getBuild() const;

    /** Returns the build date of the program. */
    std::string getDate() const;

    //-----------------------------------------------------------------------

    /** Boardsize as parsed from the cmd-line options. */
    int BoardSize() const;

    /** Returns the configuration file that should be parsed.  This
        will be non-empty if the cmd-line option '--config' was parsed
        during the call to Initialize(). */
    std::string ConfigFileToExecute() const;

protected:

    /** Registers all command-line arguments. */
    virtual void RegisterCmdLineArguments();

    /** Prints all registered cmd-line arguments and their usage. */
    void Usage() const;

    //----------------------------------------------------------------------

    std::string m_name;
    std::string m_version;
    std::string m_date;

    std::string m_executable_name;
    std::string m_executable_path;

    /** Cmd-line options. */
    bool m_initialized;
    boost::program_options::options_description m_options_desc;

    int m_boardsize;
    int m_random_seed;
    bool m_use_logfile;
    std::string m_logfile_name;
    std::string m_logfile_level;
    std::string m_config_file;
    LogLevel m_stderr_level;

private:

    /** Constructor. */
    HexProgram();

    /** Destructor. */
    virtual ~HexProgram();

    void InitLog();
    
    void InitRandom();

    /** Start the Hex system. */
    void InitializeHexSystem();

    void ProcessCmdLineArguments(int argc, char** argv);

    void ShutdownLog();

    //----------------------------------------------------------------------
};

inline std::string HexProgram::getName() const
{
    return m_name;
}

inline std::string HexProgram::getVersion() const
{
    return m_version;
}

inline std::string HexProgram::getDate() const
{
    return m_date;
}

inline int HexProgram::BoardSize() const
{
    return m_boardsize;
}

inline std::string HexProgram::ConfigFileToExecute() const
{
    return m_config_file;
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // HEXPROGRAM_HPP
