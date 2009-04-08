//----------------------------------------------------------------------------
/** @file
 */
//----------------------------------------------------------------------------

#ifndef HEX_TIME_HPP
#define HEX_TIME_HPP

#include <string>

double HexGetTime();

static const double ONE_MINUTE = 60.0;
static const double ONE_HOUR = 60*ONE_MINUTE;
static const double ONE_DAY = 24*ONE_HOUR;

std::string FormattedTime(double elapsed);

#endif

//----------------------------------------------------------------------------
