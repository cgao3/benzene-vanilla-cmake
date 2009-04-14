//----------------------------------------------------------------------------
/** @file Logger.hpp
 */
//----------------------------------------------------------------------------

#ifndef HEXLOGGER_HPP
#define HEXLOGGER_HPP

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <pthread.h>

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
};

//----------------------------------------------------------------------------

/** An output stream for a Logger object. */
class LogHandler
{
public:

    /** Creates a Handler that sends output to an existing output stream. */
    LogHandler(std::ostream& stream) 
        : stream(stream)
    {}

    /** The stream for this handler. */
    std::ostream& stream;
};

//----------------------------------------------------------------------------

/** Logger supporting message levels and multiple output streams
    received from multiple threads.

    A Logger can have several LogHandlers with each LogHandler
    associated with a LogLevel.  When the Logger receives a message
    with LogLevel L, all handlers with level at least L will be sent a
    copy of the message.

    An example Logger with two LogHandlers: LogHandler1's output stream
    is std::cerr, and LogHandler2's output stream is a std::ofstream 
    pointing to the file 'example.log'.  LogHandler1 has LogLevel 'info', 
    and LogLevel2 has LogLevel 'all'.  Any message if level 'info' or
    higher will be echoed to std::cerr and 'example.log'.  A message of
    level 'fine' or lower will ONLY go to 'example.log'. 

    Logger handles messages coming from multiple threads internally. A
    LogLevel for each thread is maintained, and separate buffers are
    used for messages being formed simulataneously in different
    threads.  Blocking is used to grant a thread exclusive access to
    the LogHandlers when printing; this potentially could cause
    slowdown if several threads are dumping lots of text
    simultaneously.
*/
class Logger
{
public:

    /** Creates a logger object. */
    Logger();

    /** Destructor. */
    virtual ~Logger();

    /** Adds a handler to this logger at the given level.  Returns
        true if handler was successfully added; false if it is a
        duplicate. */
    bool AddHandler(LogHandler* handler, LogLevel level);

    /** Removes the given handler. Returns false if handler doesn't
        exist. */
    bool RemoveHandler(LogHandler* handler);

    /** Sets the level of all messages this logger receives from now
        on. */
    void SetLevel(LogLevel level);

    /** Flushes the log. */
    void Flush();

    /** Pipes text into the log. */
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

    /** LogHandlers for this log. */
    std::vector<LogHandler*> m_handlers;

    /** Level for each handler. */
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
    GetThreadBuffer().buffer << type;
    return *this;
}

template<>
inline Logger& Logger::operator<< <char> (const char& type)
{
    if (type == '\n')
	Flush();
    else
	GetThreadBuffer().buffer << type;
    return *this;
}

//----------------------------------------------------------------------------

#endif // LOGGER_HPP
