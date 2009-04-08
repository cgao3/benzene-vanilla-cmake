//----------------------------------------------------------------------------
/** @file
 */
//----------------------------------------------------------------------------

#include "Logger.hpp"

//----------------------------------------------------------------------------

Logger::Logger(std::string callsign)
    : m_callsign(callsign)
{
    for (int i=0; i<MAX_THREADS; i++) {
        m_thread_buffer[i].id = 0;
    }
}

Logger::~Logger()
{
}

//----------------------------------------------------------------------------

bool Logger::AddHandler(LogHandler* handler, LogLevel level)
{
    for (unsigned i=0; i<m_handlers.size(); ++i) {
        if (m_handlers[i] == handler)
            return false;
    }
    m_handlers.push_back(handler);
    m_levels.push_back(level);
    return true;
}

bool Logger::RemoveHandler(LogHandler* handler)
{
    std::vector<LogHandler*>::iterator ih = m_handlers.begin();
    std::vector<LogLevel>::iterator il = m_levels.begin();
    for (; ih != m_handlers.end(); ++ih, ++il) {
        if (*ih == handler) {
            m_handlers.erase(ih);
            m_levels.erase(il);
            return true;
        }
    }
    return false;
}

Logger::ThreadBuffer& Logger::GetThreadBuffer()
{
    int first_empty = -1;
    pthread_t self = pthread_self();

    // see if thread already has a buffer open
    for (int i=0; i<MAX_THREADS; ++i) {
        if (m_thread_buffer[i].id == self)
            return m_thread_buffer[i];
        else if (m_thread_buffer[i].id == 0 && first_empty == -1)
            first_empty = i;
    }

    // if not, set this free buffer for use by thread
    if (first_empty != -1) {
        pthread_mutex_lock(&m_buffer_mutex);
        m_thread_buffer[first_empty].id = self;
        pthread_mutex_unlock(&m_buffer_mutex);
        return m_thread_buffer[first_empty];
    }

    // no free buffer... just return first buffer. This sucks!!
    std::cerr << "####### LOG HAS NO FREE BUFFER! #######" << std::endl;
    return m_thread_buffer[0];
}

void Logger::SetLevel(LogLevel level)
{
    pthread_mutex_lock(&m_map_mutex);
    m_thread_level[pthread_self()] = level;
    pthread_mutex_unlock(&m_map_mutex);
}

LogLevel Logger::GetThreadLevel()
{
    LogLevel level;
    pthread_mutex_lock(&m_map_mutex);    
    level = m_thread_level[pthread_self()];
    pthread_mutex_unlock(&m_map_mutex);
    return level;
}

void Logger::Flush()
{
    ThreadBuffer& buffer = GetThreadBuffer();
    if (buffer.buffer.str() == "") 
        return;

    // dump msg to handlers of appropriate level; grab the
    // buffer mutex so no one else can do this or change the
    // buffers in any way. 
    pthread_mutex_lock(&m_buffer_mutex);
    for (unsigned i=0; i<m_handlers.size(); ++i) {
        LogLevel level = GetThreadLevel();
        if (level < m_levels[i]) continue;
        LogHandler& handler = *m_handlers[i];
        handler.stream << pthread_self() << " " 
                       << LogLevelUtil::toString(level) << ": " 
                       << buffer.buffer.str() << std::endl;
        handler.stream.flush();
    }

    // clear buffer and set it to unused
    buffer.buffer.str("");
    buffer.id = 0;
    pthread_mutex_unlock(&m_buffer_mutex);
}

Logger& Logger::operator<<(Logger& ( *pf )(Logger& log))
{
    pf(*this);
    return *this;
}

//----------------------------------------------------------------------------

Logger& hex::endl(Logger& log)
{
    log.Flush();
    return log;
}

Logger& hex::finer(Logger& log)
{
    log.SetLevel(FINER);
    return log;
}

Logger& hex::fine(Logger& log)
{
    log.SetLevel(FINE);
    return log;
}
 
Logger& hex::config(Logger& log)
{
    log.SetLevel(CONFIG);
    return log;
}

Logger& hex::info(Logger& log)
{
    log.SetLevel(INFO);
    return log;
}

Logger& hex::warning(Logger& log)
{
    log.SetLevel(WARNING);
    return log;
}

Logger& hex::severe(Logger& log)
{
    log.SetLevel(SEVERE);
    return log;
}

//----------------------------------------------------------------------------

bool LogLevelUtil::IsValidLevel(LogLevel level)
{
    if (level == ALL || level == FINER || level == FINE || level == CONFIG ||
        level == INFO || level == WARNING || level == SEVERE || 
        level == OFF)
        return true;
    return false;
}
    
std::string LogLevelUtil::toString(LogLevel level)
{
    if (level == ALL) return "all";
    if (level == FINER) return "finer";
    if (level == FINE) return "fine";
    if (level == CONFIG) return "config";
    if (level == INFO) return "info";
    if (level == WARNING) return "warning";
    if (level == SEVERE) return "severe";
    return "off";
}

LogLevel LogLevelUtil::fromString(std::string level)
{
    if (level == "all") return ALL;
    if (level == "finer") return FINER;
    if (level == "fine") return FINE;
    if (level == "config") return CONFIG;
    if (level == "info") return INFO;
    if (level == "warning") return WARNING;
    if (level == "severe") return SEVERE;
    return OFF;
}

//----------------------------------------------------------------------------
