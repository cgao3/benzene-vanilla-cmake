check_PROGRAMS = benzene_unittest

benzene_unittest_SOURCES = \
../util/test/BitsetTest.cpp \
../util/test/ChangeLogTest.cpp \
../util/test/DigraphTest.cpp \
../util/test/HashTest.cpp \
../util/test/HashTableTest.cpp \
../util/test/LoggerTest.cpp \
../util/test/SortedSequenceTest.cpp \
../util/test/TimeTest.cpp \
../util/test/UnionFindTest.cpp \
../hex/test/BitsetIteratorTest.cpp \
../hex/test/BoardIteratorTest.cpp \
../hex/test/BoardUtilsTest.cpp \
../hex/test/ConnectionsTest.cpp \
../hex/test/ConstBoardTest.cpp \
../hex/test/GraphUtilsTest.cpp \
../hex/test/GroupBoardTest.cpp \
../hex/test/HexBoardTest.cpp \
../hex/test/HexColorTest.cpp \
../hex/test/HexPointTest.cpp \
../hex/test/ICEngineTest.cpp \
../hex/test/InferiorCellsTest.cpp \
../hex/test/PatternBoardTest.cpp \
../hex/test/RingGodelTest.cpp \
../hex/test/SequenceHashTest.cpp \
../hex/test/StoneBoardTest.cpp \
../hex/test/VCListTest.cpp \
../hex/test/VCTest.cpp \
../hex/test/VCUtilsTest.cpp \
../hex/test/ZobristHashTest.cpp \
../test/TestMain.cpp

benzene_unittest_LDADD = \
../player/libplayer.a \
../solver/libsolver.a \
../hex/libhex.a \
../util/libutil.a \
$(FUEGO_BUILD_ROOT)/smartgame/libfuego_smartgame.a \
$(FUEGO_BUILD_ROOT)/gtpengine/libfuego_gtpengine.a \
$(DB_LIBS) \
$(BOOST_FILESYSTEM_LIB) \
$(BOOST_PROGRAM_OPTIONS_LIB) \
$(BOOST_SYSTEM_LIB) \
$(BOOST_UNIT_TEST_FRAMEWORK_LIB) \
$(BOOST_THREAD_LIB)

benzene_unittest_DEPENDENCIES = \
../util/libutil.a \
../hex/libhex.a \
../solver/libsolver.a \
../player/libplayer.a \
$(FUEGO_BUILD_ROOT)/smartgame/libfuego_smartgame.a \
$(FUEGO_BUILD_ROOT)/gtpengine/libfuego_gtpengine.a

benzene_unittest_LDFLAGS = $(BOOST_LDFLAGS)

benzene_unittest_CPPFLAGS = \
$(BOOST_CPPFLAGS) \
-I$(FUEGO_ROOT)/smartgame \
-I$(FUEGO_ROOT)/gtpengine \
-I@top_srcdir@/src/ \
-I@top_srcdir@/src/util \
-I@top_srcdir@/src/hex \
-I@top_srcdir@/src/uct \
-I@top_srcdir@/src/solver \
-I@top_srcdir@/src/player

DISTCLEANFILES = *~
