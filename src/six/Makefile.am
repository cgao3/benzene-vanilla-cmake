bin_PROGRAMS = six

six_SOURCES = \
batch.cpp \
carrier.cpp \
circuit.cpp \
connector.cpp \
group.cpp \
hexboard.cpp \
hexgame.cpp \
hexgamestate.cpp \
hexmark.cpp \
hexmove.cpp \
lssolve.cpp \
main.cpp \
misc.cpp \
sixplayer.cpp \
SixHtpEngine.cpp

noinst_HEADERS = \
batch.h \
carrier.h \
circuit.h \
connector.h \
group.h \
hexboard.h \
hexfield.h \
hexgame.h \
hexgamestate.h \
hexmark.h \
hexmatch.h \
hexmove.h \
hexplayer.h \
lssolve.h \
mat.h \
misc.h \
poi.h \
sixplayer.h \
slicedtask.h \
vec.h \
SixHtpEngine.hpp

six_LDADD = \
../hex/libhex.a \
../util/libutil.a \
$(FUEGO_BUILD_ROOT)/smartgame/libfuego_smartgame.a \
$(FUEGO_BUILD_ROOT)/gtpengine/libfuego_gtpengine.a \
$(BOOST_FILESYSTEM_LIB) \
$(BOOST_PROGRAM_OPTIONS_LIB) \
$(BOOST_SYSTEM_LIB) \
$(BOOST_THREAD_LIB)

six_DEPENDENCIES = \
../hex/libhex.a \
../util/libutil.a \
$(FUEGO_BUILD_ROOT)/smartgame/libfuego_smartgame.a \
$(FUEGO_BUILD_ROOT)/gtpengine/libfuego_gtpengine.a

six_LDFLAGS = $(BOOST_LDFLAGS)

six_CPPFLAGS = \
$(BOOST_CPPFLAGS) \
-I$(FUEGO_ROOT)/smartgame \
-I$(FUEGO_ROOT)/gtpengine \
-I@top_srcdir@/src/ \
-I@top_srcdir@/src/util \
-I@top_srcdir@/src/hex 

DISTCLEANFILES = *~
