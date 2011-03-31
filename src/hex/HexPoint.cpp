//----------------------------------------------------------------------------
/** @file HexPoint.cpp */
//----------------------------------------------------------------------------

#include <sstream>
#include <string>
#include <strings.h> // strcasecmp
#include <cstring>
#include <cstdio>
#include "HexPoint.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

namespace
{

    /** Static data pertaining to HexPoints. */
    struct HexPointData
    {
        /** Name for each HexPoint. */
        std::vector<std::string> name;
        
        HexPointData();
    };
    
    HexPointData::HexPointData()
    {
        name.resize(FIRST_INVALID, "--bad-point--");
        
        name[INVALID_POINT] = "invalid";
        name[RESIGN] = "resign";
        name[SWAP_PIECES] = "swap-pieces";
        
        name[NORTH] = "north";
        name[EAST]  = "east";
        name[SOUTH] = "south";
        name[WEST]  = "west";
        
        for (int y=0; y < MAX_HEIGHT; y++) 
        {
            for (int x = 0; x < MAX_WIDTH; x++) 
            {
                char buff[32];
                HexPoint p = HexPointUtil::coordsToPoint(x, y);
                sprintf(buff, "%c%d", 'a' + x, y+1);
                name[p] = std::string(buff);
            }
        }
    }

    /** Returns a constant reference to the static HexPoint data
        allocated as a static variable of this method (this way, other
        globals can be initialized safely with this data).
    */
    const HexPointData& GetHexPointData()
    {
        static HexPointData s_data;
        return s_data;
    }

} // anonymous namespace

//----------------------------------------------------------------------------

std::string HexPointUtil::ToString(HexPoint p)
{
    BenzeneAssert(0 <= p && p < FIRST_INVALID);
    return GetHexPointData().name[p];
}

HexPoint HexPointUtil::FromString(const std::string& name)
{
    const char *str = name.c_str();
    for (int p = 0; p < FIRST_INVALID; ++p) 
        if (!strcasecmp(GetHexPointData().name[p].c_str(), str)) 
            return static_cast<HexPoint>(p);
    return INVALID_POINT;
}

void HexPointUtil::FromString(const std::string& str, PointSequence& pts)
{
    std::istringstream is(str);
    std::string token;
    while (is >> token)
    {
        HexPoint p = HexPointUtil::FromString(token);
        pts.push_back(p);
    }
}

std::string HexPointUtil::ToString(const HexPointPair& p)
{
    std::ostringstream os;
    os << "(" << ToString(p.first) << ", " << ToString(p.second) << ")";
    return os.str();
}

std::string HexPointUtil::ToString(const PointSequence& lst)
{
    std::ostringstream os;
    for (std::size_t i = 0; i < lst.size(); ++i)
    {
        if (i) os << ' ';
        os << ToString(lst[i]);
    }
    return os.str();
}

std::string HexPointUtil::ToString(const bitset_t& b)
{
    std::ostringstream os;
    for (int i = 0; i < FIRST_INVALID; i++) 
        if (b.test(i)) 
            os << ' ' << ToString(static_cast<HexPoint>(i));
    return os.str();
}

//----------------------------------------------------------------------------
