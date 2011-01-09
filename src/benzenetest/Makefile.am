bin_PROGRAMS = benzenetest

benzenetest_SOURCES = \
BenzeneTestEngine.cpp \
BenzeneTestMain.cpp \
BenzeneTestProgram.cpp

noinst_HEADERS = \
BenzeneTestEngine.hpp \
BenzeneTestProgram.hpp

benzenetest_LDADD = \
../commonengine/libcommonengine.a \
../simpleplayers/libsimpleplayers.a \
../solver/libsolver.a \
../book/libbook.a \
../hex/libhex.a \
../util/libutil.a \
$(FUEGO_BUILD)/smartgame/libfuego_smartgame.a \
$(FUEGO_BUILD)/gtpengine/libfuego_gtpengine.a \
$(DB_LIBS) \
$(BOOST_FILESYSTEM_LIB) \
$(BOOST_PROGRAM_OPTIONS_LIB) \
$(BOOST_SYSTEM_LIB) \
$(BOOST_THREAD_LIB)

benzenetest_DEPENDENCIES = \
../util/libutil.a \
../hex/libhex.a \
../book/libbook.a \
../solver/libsolver.a \
../simpleplayers/libsimpleplayers.a \
../commonengine/libcommonengine.a \
$(FUEGO_BUILD)/smartgame/libfuego_smartgame.a \
$(FUEGO_BUILD)/gtpengine/libfuego_gtpengine.a

benzenetest_LDFLAGS = $(BOOST_LDFLAGS)

benzenetest_CPPFLAGS = \
$(BOOST_CPPFLAGS) \
-I$(FUEGO_ROOT)/smartgame \
-I$(FUEGO_ROOT)/gtpengine \
-I@top_srcdir@/src/ \
-I@top_srcdir@/src/util \
-I@top_srcdir@/src/hex \
-I@top_srcdir@/src/book \
-I@top_srcdir@/src/uct \
-I@top_srcdir@/src/solver \
-I@top_srcdir@/src/simpleplayers \
-I@top_srcdir@/src/commonengine

DISTCLEANFILES = *~
