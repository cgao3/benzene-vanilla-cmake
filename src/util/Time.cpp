//----------------------------------------------------------------------------
/** @file Time.cpp */
//----------------------------------------------------------------------------

#include <cstdio>
#include <iomanip>
#include <sstream>
#include "Time.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

std::string Time::Formatted(double elapsed)
{
    std::ostringstream os;

    int days = static_cast<int>(elapsed / ONE_DAY);
    elapsed -= days * ONE_DAY;
    int hours = static_cast<int>(elapsed / ONE_HOUR);
    elapsed -= hours * ONE_HOUR;
    int mins = static_cast<int>(elapsed / ONE_MINUTE);
    elapsed -= mins * ONE_MINUTE;
    
    if (days) os << days << "d";
    if (hours) os << hours << "h";
    if (mins) os << mins << "m";
    os << std::setprecision(4) << elapsed << "s";

    return os.str();
}

//----------------------------------------------------------------------------
