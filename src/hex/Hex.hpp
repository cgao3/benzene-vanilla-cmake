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

namespace hex
{
    /** Master logger. */
    extern Logger log;
};

inline Logger& LogFine()
{
    return hex::log << hex::fine;
}

inline Logger& LogConfig()
{
    return hex::log << hex::config;
}

inline Logger& LogInfo()
{
    return hex::log << hex::info;
}

inline Logger& LogWarning()
{
    return hex::log << hex::warning;
}

inline Logger& LogSevere()
{
    return hex::log << hex::severe;
}

//----------------------------------------------------------------------------

#endif // HEX_HPP
