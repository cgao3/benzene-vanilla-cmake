//----------------------------------------------------------------------------
/** @file Logger.hpp */
//----------------------------------------------------------------------------

#ifndef HEXLOGGER_HPP
#define HEXLOGGER_HPP

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <pthread.h>
#include "Benzene.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Levels for messages to a Logger object. */
enum LogLevel
{
    /** All messages with levels higher than OFF are ignored. */
    LOG_LEVEL_OFF = 65536,

    /** Message level indicating a serious failure. 
        In general a severe message should describe events of considerable
        importance which will prevent normal program execution. 
    */
    LOG_LEVEL_SEVERE = 1000,

    /** Message level indicating a potential problem. 
        Should be used to describe events of interest to users and
        developers.
    */
    LOG_LEVEL_WARNING = 900,

    /** Message level for informational messages.
        Messages of this level or higher are typically output on the
        console.
    */
    LOG_LEVEL_INFO = 800,
    
    /** Message level for configuration purposes. 
        Should be used to provide a variety of static configuration
	information to assist in debugging.
    */
    LOG_LEVEL_CONFIG = 700,

    /** Message level providing tracing information. */
    LOG_LEVEL_FINE = 500,

    /** Message level providing more tracing information. */
    LOG_LEVEL_FINER = 300, 

    /** Special level indicating that all messages should be
        logged. */
    LOG_LEVEL_ALL = 0
};

/** Utilities on LogLevel. */
namespace LogLevelUtil
{
    /** Returns true if the given level is valid. */
    bool IsValidLevel(LogLevel level);
    
    /** Returns a string representation of the level. */
    std::string toString(LogLevel level);

    /** Converts a string to a LogLevel. */
    LogLevel fromString(std::string);
}

//----------------------------------------------------------------------------

/** Logger supporting message levels and multiple output streams
    received from multiple threads.

    A Logger can have several streams. Each stream is assigned a level.
    When the Logger receives a message at level L, all streams with level 
    at least L will be sent a copy of the message.

    Logger handles messages coming from multiple threads internally. A
    LogLevel for each thread is maintained, and separate buffers are
    used for messages being formed simulataneously in different
    threads.  Blocking is used to grant a thread exclusive access to
    the streams when printing; this potentially could cause slowdown
    if several threads are dumping lots of text simultaneously.
*/
class Logger
{
public:

    /** Creates a logger object. By default, log outputs to std::cerr
        at LOG_LEVEL_INFO. */
    Logger();

    /** Destructor. */
    ~Logger();

    /** Returns the global Logger object. */
    static Logger& Global();

    /** Adds a handler to this logger at the given level. */
    void AddStream(std::ostream& stream, LogLevel level);

    /** Removes all output streams. */
    void ClearStreams();
    
    /** Sets the level of all messages this logger receives from now
        on. */
    void SetLevel(LogLevel level);

    /** Flushes the log. */
    void Flush();

    /** Pipes text into the log. If text ends in a '\n', log is
        flushed. */
    template<typename TYPE>
    Logger& operator<<(const TYPE& type);

private:

    /** Maximum number of threads handled at once. */
    static const int MAX_THREADS = 16;

    /** Buffer for a thread of execution. */
    struct ThreadBuffer
    {
        std::ostringstream buffer;
        pthread_t id;
    };

    /** Returns the buffer for the current thread. */
    ThreadBuffer& GetThreadBuffer();

    /** Returns the current loglevel for the thread. */
    LogLevel GetThreadLevel();

    /** Streams for this log. */
    std::vector<std::ostream*> m_streams;

    /** Level for each stream. */
    std::vector<LogLevel> m_levels;
   
    /** Threads must grab this mutex before modifying m_thread_buffer
        or printing output. */
    pthread_mutex_t m_buffer_mutex;

    /** Buffers for up to MAX_THREADS active threads. */
    ThreadBuffer m_thread_buffer[MAX_THREADS];

    /** Threads must grab this mutex before modifying
        m_thread_level. */
    pthread_mutex_t m_map_mutex;

    /** Current log level for each thread. */
    std::map<pthread_t, LogLevel> m_thread_level;
};

template<typename TYPE>
Logger& Logger::operator<<(const TYPE& type)
{
    ThreadBuffer& tb = GetThreadBuffer();
    tb.buffer << type;
    std::string str(tb.buffer.str());
    if (!str.empty() && str[str.size() - 1] == '\n')
        Flush();
    return *this;
}

//----------------------------------------------------------------------------

/** Sets global logger to LOG_LEVEL_FINE. */
inline Logger& LogFine()
{
    Logger::Global().SetLevel(LOG_LEVEL_FINE);
    return Logger::Global();
}

/** Similar to LogFine() */
inline Logger& LogConfig()
{
    Logger::Global().SetLevel(LOG_LEVEL_CONFIG);
    return Logger::Global();
}

/** Similar to LogFine() */
inline Logger& LogInfo()
{
    Logger::Global().SetLevel(LOG_LEVEL_INFO);
    return Logger::Global();
}

/** Similar to LogFine() */
inline Logger& LogWarning()
{
    Logger::Global().SetLevel(LOG_LEVEL_WARNING);
    return Logger::Global();
}

/** Similar to LogFine() */
inline Logger& LogSevere()
{
    Logger::Global().SetLevel(LOG_LEVEL_SEVERE);
    return Logger::Global();
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // LOGGER_HPP
