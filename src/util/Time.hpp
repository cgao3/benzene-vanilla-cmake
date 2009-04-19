//----------------------------------------------------------------------------
/** @file Time.hpp
 */
//----------------------------------------------------------------------------

#ifndef TIME_HPP
#define TIME_HPP

#include <string>
#include "Benzene.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

namespace Time
{
    /** Seconds in a one minute. */
    static const double ONE_MINUTE = 60.0;
    
    /** Seconds in one hour. */
    static const double ONE_HOUR = 60*ONE_MINUTE;
    
    /** Seconds in one day. */
    static const double ONE_DAY = 24*ONE_HOUR;

    /** Returns the time. */
    double Get();

    /** Formats elapsed time as a human readable string. */
    std::string Formatted(double elapsed);
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // TIME_HPP
