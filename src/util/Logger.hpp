//----------------------------------------------------------------------------
// $Id: Logger.hpp 1994 2009-04-06 00:57:12Z broderic $
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
    OFF = 65536,

    /** SEVERE is a message indicating a serious failure. 
        In general a SEVERE message should describe events of considerable
        importance which will prevent normal program execution. 
    */
    SEVERE = 1000,

    /** WARNING is a message level indicating a potential problem. 
        WARNING should be used to describe events of interest to users
        and developers. 
    */
    WARNING = 900,

    /** INFO is a message level for informational messages.
        Messages of INFO level or higher are typically output on the console.
    */
    INFO = 800,
    
    /** CONFIG is a message level for configuration purposes. 
        CONFIG should be used to provide a variety of static configuration
	information to assist in debugging. 
    */
    CONFIG = 700,

    /** FINE is a message level providing tracing information. */
    FINE = 500,

    /** FINER is a message level providing more tracing information. */
    FINER = 300, 

    /** A special level indicating that all messages should be logged. */
    ALL = 0
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

    /** Sets the level of all messages this logger receives from now on.
        
        You can use the hex::severe, hex::warning, etc, functions to do this
        for you.  For example, to log a severe message you can do:

           [...]
           hex::log << hex::severe << "CRITICAL ERROR" << hex::endl;
           [...]

           This is equivalent to:

           hex::log.setLevel(SEVERE);
           hex::log << "CRITICAL ERROR" << hex::endl;

       You can mix the levels between hex::endl's, but the LAST one specified
       will determine at what level the message is logged. For example, 
          
           hex::log << hex::severe << "SEVERE " 
                    << hex::info << "REVERE"
                    << hex::endl;
                    
           will send "SEVERE REVERE" as an INFO level message. 
    */
    void SetLevel(LogLevel level);

    /** Flushes the log. */
    void Flush();

    /** Executes a function on a Logger object; typically used to
        change the loggers level. An example would be 'hex::info'. */
    Logger& operator<<(Logger& ( *pf )(Logger& log));

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

namespace hex {

    /** Sends eol and flushes the buffer. */
    Logger& endl(Logger& log);

    /** Sets the log's level to FINER. */
    Logger& finer(Logger& log);

    /** Sets the log's level to FINE. */
    Logger& fine(Logger& log);

    /** Sets the log's level to CONFIG. */
    Logger& config(Logger& log);

    /** Sets the log's level to INFO. */
    Logger& info(Logger& log);

    /** Sets the logs level to WARNING. */
    Logger& warning(Logger& log);

    /** Sets the logs level to SEVERE. */
    Logger& severe(Logger& log);
}

//----------------------------------------------------------------------------

#endif // LOGGER_HPP
