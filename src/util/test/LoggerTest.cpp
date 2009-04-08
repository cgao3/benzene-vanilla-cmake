//---------------------------------------------------------------------------
/** @file
 */
//---------------------------------------------------------------------------
#include <boost/test/auto_unit_test.hpp>

#include "Logger.hpp"

//---------------------------------------------------------------------------

namespace {

BOOST_AUTO_UNIT_TEST(Logger_LogLevelOrdering)
{
    // urgency ordering of log levels
    BOOST_CHECK(OFF > SEVERE);
    BOOST_CHECK(SEVERE > WARNING);
    BOOST_CHECK(WARNING > INFO);
    BOOST_CHECK(INFO > CONFIG);
    BOOST_CHECK(CONFIG > FINE);
    BOOST_CHECK(FINE > ALL);
}

BOOST_AUTO_UNIT_TEST(Logger_LogLevelUtil)
{
    // valid levels
    BOOST_CHECK(LogLevelUtil::IsValidLevel(OFF));
    BOOST_CHECK(LogLevelUtil::IsValidLevel(SEVERE));
    BOOST_CHECK(LogLevelUtil::IsValidLevel(WARNING));
    BOOST_CHECK(LogLevelUtil::IsValidLevel(INFO));
    BOOST_CHECK(LogLevelUtil::IsValidLevel(CONFIG));
    BOOST_CHECK(LogLevelUtil::IsValidLevel(FINE));
    BOOST_CHECK(LogLevelUtil::IsValidLevel(ALL));
    BOOST_CHECK(!LogLevelUtil::IsValidLevel((LogLevel)(OFF+1)));
    BOOST_CHECK(!LogLevelUtil::IsValidLevel((LogLevel)(ALL - 1)));
    
    // conversion to string
    BOOST_CHECK_EQUAL(LogLevelUtil::toString(OFF), "off");
    BOOST_CHECK_EQUAL(LogLevelUtil::toString(SEVERE), "severe");
    BOOST_CHECK_EQUAL(LogLevelUtil::toString(WARNING), "warning");
    BOOST_CHECK_EQUAL(LogLevelUtil::toString(INFO), "info");
    BOOST_CHECK_EQUAL(LogLevelUtil::toString(CONFIG), "config");
    BOOST_CHECK_EQUAL(LogLevelUtil::toString(FINE), "fine");
    BOOST_CHECK_EQUAL(LogLevelUtil::toString(ALL), "all");
    
    // conversion from string
    BOOST_CHECK_EQUAL(LogLevelUtil::fromString("off"), OFF);
    BOOST_CHECK_EQUAL(LogLevelUtil::fromString("severe"), SEVERE);
    BOOST_CHECK_EQUAL(LogLevelUtil::fromString("warning"), WARNING);
    BOOST_CHECK_EQUAL(LogLevelUtil::fromString("info"), INFO);
    BOOST_CHECK_EQUAL(LogLevelUtil::fromString("config"), CONFIG);
    BOOST_CHECK_EQUAL(LogLevelUtil::fromString("fine"), FINE);
    BOOST_CHECK_EQUAL(LogLevelUtil::fromString("all"), ALL);
    BOOST_CHECK_EQUAL(LogLevelUtil::fromString("random string!!"), OFF);
}

BOOST_AUTO_UNIT_TEST(Logger_AllTests)
{
    std::string s = "testing";
    Logger hl(s);
}

}

//---------------------------------------------------------------------------
