//----------------------------------------------------------------------------
// $Id: Hex.hpp 1994 2009-04-06 00:57:12Z broderic $
//----------------------------------------------------------------------------

#ifndef HEX_HPP
#define HEX_HPP

//----------------------------------------------------------------------------
// c++ and stl stuff

#include <set>
#include <map>
#include <list>
#include <vector>
#include <queue>
#include <bitset>
#include <string>
#include <utility>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <functional>
#include <limits>

/** Marks a parameter as unusable and suppresses compiler warning to
    that it is unused. */
#ifdef UNUSED
#elif defined(__GNUC__)
# define UNUSED(x) UNUSED_ ## x __attribute__((unused))
#elif defined(__LCLINT__)
# define UNUSED(x) /*@unused@*/ x
#else
# define UNUSED(x) x
#endif

//----------------------------------------------------------------------------
// stuff from src/util/

#include "Types.hpp"
#include "Hash.hpp"
#include "Logger.hpp"
#include "Bitset.hpp"

//----------------------------------------------------------------------------
// hex stuff

#include "HexAssert.hpp"
#include "HexColor.hpp"
#include "HexPoint.hpp"

inline Logger& LogFine()
{
    Logger::Global().SetLevel(LOG_LEVEL_FINE);
    return Logger::Global();
}

inline Logger& LogConfig()
{
    Logger::Global().SetLevel(LOG_LEVEL_CONFIG);
    return Logger::Global();
}

inline Logger& LogInfo()
{
    Logger::Global().SetLevel(LOG_LEVEL_INFO);
    return Logger::Global();
}

inline Logger& LogWarning()
{
    Logger::Global().SetLevel(LOG_LEVEL_WARNING);
    return Logger::Global();
}

inline Logger& LogSevere()
{
    Logger::Global().SetLevel(LOG_LEVEL_SEVERE);
    return Logger::Global();
}

//----------------------------------------------------------------------------

#endif // HEX_HPP
