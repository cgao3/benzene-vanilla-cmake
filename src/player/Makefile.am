noinst_LIBRARIES = libplayer.a

libplayer_a_SOURCES = \
BenzeneHtpEngine.cpp \
BenzenePlayer.cpp \
BookCheck.cpp \
EndgameCheck.cpp \
HandBookCheck.cpp \
HandicapPlayer.cpp \
LadderCheck.cpp \
PerfectPlayer.cpp \
PlayerFactory.cpp \
SwapCheck.cpp \
VulPreCheck.cpp 

noinst_HEADERS = \
BenzeneHtpEngine.hpp \
BenzenePlayer.hpp \
BookBuilder.hpp \
BookCheck.hpp \
EndgameCheck.hpp \
HandBookCheck.hpp \
HandicapPlayer.hpp \
LadderCheck.hpp \
PerfectPlayer.hpp \
PlayerFactory.hpp \
SwapCheck.hpp \
VulPreCheck.hpp 

libplayer_a_CPPFLAGS = \
$(BOOST_CPPFLAGS) \
-DABS_TOP_SRCDIR='"@abs_top_srcdir@"' \
-DDATADIR='"$(pkgdatadir)"' \
-I$(FUEGO_ROOT)/gtpengine \
-I$(FUEGO_ROOT)/smartgame \
-I@top_srcdir@/src/ \
-I@top_srcdir@/src/util \
-I@top_srcdir@/src/hex \
-I@top_srcdir@/src/uct \
-I@top_srcdir@/src/solver

DISTCLEANFILES = *~

