noinst_LIBRARIES = libsimpleplayers.a

libsimpleplayers_a_SOURCES = \
HandicapPlayer.cpp \
PerfectPlayer.cpp \
VulPreCheck.cpp 

noinst_HEADERS = \
HandicapPlayer.hpp \
PerfectPlayer.hpp \
VulPreCheck.hpp 

libsimpleplayers_a_CPPFLAGS = \
$(BOOST_CPPFLAGS) \
-DABS_TOP_SRCDIR='"@abs_top_srcdir@"' \
-DDATADIR='"$(pkgdatadir)"' \
-I$(FUEGO_ROOT)/gtpengine \
-I$(FUEGO_ROOT)/smartgame \
-I@top_srcdir@/src/ \
-I@top_srcdir@/src/util \
-I@top_srcdir@/src/hex \
-I@top_srcdir@/src/solver

DISTCLEANFILES = *~

