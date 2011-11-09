//----------------------------------------------------------------------------
/** @file Logger.cpp */
//----------------------------------------------------------------------------

#include <iomanip>
#include "Logger.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

Logger::Logger()
    : m_streams(),
      m_levels()
{
    for (int i = 0; i < MAX_THREADS; ++i)
        m_thread_buffer[i].id = 0;
    AddStream(std::cerr, LOG_LEVEL_INFO);
}

Logger::~Logger()
{
}

Logger& Logger::Global()
{
    static Logger s_global_logger;
    return s_global_logger;
}

//----------------------------------------------------------------------------

void Logger::AddStream(std::ostream& stream, LogLevel level)
{
    m_streams.push_back(&stream);
    m_levels.push_back(level);
}

void Logger::ClearStreams()
{
    m_streams.clear();
    m_levels.clear();
}

Logger::ThreadBuffer& Logger::GetThreadBuffer()
{
    int i;
    int first_empty = -1;
    pthread_t self = pthread_self();
    pthread_mutex_lock(&m_buffer_mutex);
    // look for a pre-existing buffer for this thread
    for (i = 0; i < MAX_THREADS; ++i) 
    {
        if (m_thread_buffer[i].id == self)
            break;
        else if (m_thread_buffer[i].id == 0 && first_empty == -1)
            first_empty = i;
    }
    if (i == MAX_THREADS)
    {
        // if one does not exist, use the first empty buffer
        if (first_empty != -1) 
        {
            m_thread_buffer[first_empty].id = self;
            i = first_empty;
        }
        else
        {
            // no free buffer... just return first buffer. This sucks!!
            std::cerr << "####### LOG HAS NO FREE BUFFER! #######\n";
            i = 0;
        }
    }
    pthread_mutex_unlock(&m_buffer_mutex);
    return m_thread_buffer[i];
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
    for (std::size_t i = 0; i < m_streams.size(); ++i) 
    {
        LogLevel level = GetThreadLevel();
        if (level < m_levels[i]) 
            continue;
        std::ostream& stream = *m_streams[i];
#ifdef __CYGWIN__
        // pthread_t is a pointer type in Cygwin
        long unsigned int self =
            reinterpret_cast<long unsigned int>(pthread_self());
#else
        long unsigned int self = (long unsigned int)pthread_self();
#endif
        stream << std::hex << std::setfill('0') 
               << std::setw(5) << ((self >> 8) & 0xfffff) << " " 
               << LogLevelUtil::toString(level) << ": " 
               << buffer.buffer.str();
        stream.flush();
    }
    // clear buffer and set it to unused
    buffer.buffer.str("");
    buffer.id = 0;
    pthread_mutex_unlock(&m_buffer_mutex);
}

//----------------------------------------------------------------------------

bool LogLevelUtil::IsValidLevel(LogLevel level)
{
    if (level == LOG_LEVEL_ALL || level == LOG_LEVEL_FINER 
        || level == LOG_LEVEL_FINE || level == LOG_LEVEL_CONFIG 
        || level == LOG_LEVEL_INFO || level == LOG_LEVEL_WARNING 
        || level == LOG_LEVEL_SEVERE || level == LOG_LEVEL_OFF)
        return true;
    return false;
}
    
std::string LogLevelUtil::toString(LogLevel level)
{
    if (level == LOG_LEVEL_ALL) return "all";
    if (level == LOG_LEVEL_FINER) return "finer";
    if (level == LOG_LEVEL_FINE) return "fine";
    if (level == LOG_LEVEL_CONFIG) return "config";
    if (level == LOG_LEVEL_INFO) return "info";
    if (level == LOG_LEVEL_WARNING) return "warning";
    if (level == LOG_LEVEL_SEVERE) return "severe";
    return "off";
}

LogLevel LogLevelUtil::fromString(std::string level)
{
    if (level == "all") return LOG_LEVEL_ALL;
    if (level == "finer") return LOG_LEVEL_FINER;
    if (level == "fine") return LOG_LEVEL_FINE;
    if (level == "config") return LOG_LEVEL_CONFIG;
    if (level == "info") return LOG_LEVEL_INFO;
    if (level == "warning") return LOG_LEVEL_WARNING;
    if (level == "severe") return LOG_LEVEL_SEVERE;
    return LOG_LEVEL_OFF;
}

//----------------------------------------------------------------------------
