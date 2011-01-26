TESTS = benzene_unittest
check_PROGRAMS = $(TESTS)

benzene_unittest_SOURCES = \
../util/test/AtomicMemoryTest.cpp \
../util/test/BitsetTest.cpp \
../util/test/ChangeLogTest.cpp \
../util/test/DigraphTest.cpp \
../util/test/HashMapTest.cpp \
../util/test/LinkedListTest.cpp \
../util/test/LoggerTest.cpp \
../util/test/SortedSequenceTest.cpp \
../util/test/UnionFindTest.cpp \
../hex/test/BitsetIteratorTest.cpp \
../hex/test/BoardIteratorTest.cpp \
../hex/test/BoardUtilTest.cpp \
../hex/test/ConstBoardTest.cpp \
../hex/test/DecompositionsTest.cpp \
../hex/test/EndgameUtilTest.cpp \
../hex/test/GraphUtilTest.cpp \
../hex/test/GroupsTest.cpp \
../hex/test/HexBoardTest.cpp \
../hex/test/HexColorTest.cpp \
../hex/test/HexPointTest.cpp \
../hex/test/ICEngineTest.cpp \
../hex/test/InferiorCellsTest.cpp \
../hex/test/PatternStateTest.cpp \
../hex/test/RingGodelTest.cpp \
../hex/test/SequenceHashTest.cpp \
../hex/test/StateDBTest.cpp \
../hex/test/StoneBoardTest.cpp \
../hex/test/VCListTest.cpp \
../hex/test/VCSetTest.cpp \
../hex/test/VCTest.cpp \
../hex/test/VCUtilTest.cpp \
../hex/test/ZobristHashTest.cpp \
../test/TestMain.cpp

benzene_unittest_LDADD = \
../commonengine/libcommonengine.a \
../solver/libsolver.a \
../hex/libhex.a \
../util/libutil.a \
$(FUEGO_BUILD)/smartgame/libfuego_smartgame.a \
$(FUEGO_BUILD)/gtpengine/libfuego_gtpengine.a \
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
../commonengine/libcommonengine.a \
$(FUEGO_BUILD)/smartgame/libfuego_smartgame.a \
$(FUEGO_BUILD)/gtpengine/libfuego_gtpengine.a

benzene_unittest_LDFLAGS = $(BOOST_LDFLAGS)

benzene_unittest_CPPFLAGS = \
$(BOOST_CPPFLAGS) \
-I$(FUEGO_ROOT)/smartgame \
-I$(FUEGO_ROOT)/gtpengine \
-I@top_srcdir@/src/ \
-I@top_srcdir@/src/util \
-I@top_srcdir@/src/hex \
-I@top_srcdir@/src/solver \
-I@top_srcdir@/src/commonengine

DISTCLEANFILES = *~
