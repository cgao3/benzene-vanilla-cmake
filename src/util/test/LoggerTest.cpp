//---------------------------------------------------------------------------
/** @file
 */
//---------------------------------------------------------------------------
#include <boost/test/auto_unit_test.hpp>

#include "Logger.hpp"

using namespace benzene;

//---------------------------------------------------------------------------

namespace {

BOOST_AUTO_TEST_CASE(Logger_LogLevelOrdering)
{
    // urgency ordering of log levels
    BOOST_CHECK(LOG_LEVEL_OFF > LOG_LEVEL_SEVERE);
    BOOST_CHECK(LOG_LEVEL_SEVERE > LOG_LEVEL_WARNING);
    BOOST_CHECK(LOG_LEVEL_WARNING > LOG_LEVEL_INFO);
    BOOST_CHECK(LOG_LEVEL_INFO > LOG_LEVEL_CONFIG);
    BOOST_CHECK(LOG_LEVEL_CONFIG > LOG_LEVEL_FINE);
    BOOST_CHECK(LOG_LEVEL_FINE > LOG_LEVEL_ALL);
}

BOOST_AUTO_TEST_CASE(Logger_LogLevelUtil)
{
    // valid levels
    BOOST_CHECK(LogLevelUtil::IsValidLevel(LOG_LEVEL_OFF));
    BOOST_CHECK(LogLevelUtil::IsValidLevel(LOG_LEVEL_SEVERE));
    BOOST_CHECK(LogLevelUtil::IsValidLevel(LOG_LEVEL_WARNING));
    BOOST_CHECK(LogLevelUtil::IsValidLevel(LOG_LEVEL_INFO));
    BOOST_CHECK(LogLevelUtil::IsValidLevel(LOG_LEVEL_CONFIG));
    BOOST_CHECK(LogLevelUtil::IsValidLevel(LOG_LEVEL_FINE));
    BOOST_CHECK(LogLevelUtil::IsValidLevel(LOG_LEVEL_ALL));
    
    // conversion to string
    BOOST_CHECK_EQUAL(LogLevelUtil::toString(LOG_LEVEL_OFF), "off");
    BOOST_CHECK_EQUAL(LogLevelUtil::toString(LOG_LEVEL_SEVERE), "severe");
    BOOST_CHECK_EQUAL(LogLevelUtil::toString(LOG_LEVEL_WARNING), "warning");
    BOOST_CHECK_EQUAL(LogLevelUtil::toString(LOG_LEVEL_INFO), "info");
    BOOST_CHECK_EQUAL(LogLevelUtil::toString(LOG_LEVEL_CONFIG), "config");
    BOOST_CHECK_EQUAL(LogLevelUtil::toString(LOG_LEVEL_FINE), "fine");
    BOOST_CHECK_EQUAL(LogLevelUtil::toString(LOG_LEVEL_ALL), "all");
    
    // conversion from string
    BOOST_CHECK_EQUAL(LogLevelUtil::fromString("off"), LOG_LEVEL_OFF);
    BOOST_CHECK_EQUAL(LogLevelUtil::fromString("severe"), LOG_LEVEL_SEVERE);
    BOOST_CHECK_EQUAL(LogLevelUtil::fromString("warning"), LOG_LEVEL_WARNING);
    BOOST_CHECK_EQUAL(LogLevelUtil::fromString("info"), LOG_LEVEL_INFO);
    BOOST_CHECK_EQUAL(LogLevelUtil::fromString("config"), LOG_LEVEL_CONFIG);
    BOOST_CHECK_EQUAL(LogLevelUtil::fromString("fine"), LOG_LEVEL_FINE);
    BOOST_CHECK_EQUAL(LogLevelUtil::fromString("all"), LOG_LEVEL_ALL);
    BOOST_CHECK_EQUAL(LogLevelUtil::fromString("random string!!"), LOG_LEVEL_OFF);
}

}

//---------------------------------------------------------------------------
