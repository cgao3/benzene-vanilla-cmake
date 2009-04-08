noinst_LIBRARIES = libuct.a

libuct_a_SOURCES = \
HexUctKnowledge.cpp \
HexUctPolicy.cpp \
HexUctSearch.cpp \
HexUctState.cpp \
HexUctUtil.cpp

noinst_HEADERS = \
HexUctKnowledge.hpp \
HexUctPolicy.hpp \
HexUctSearch.hpp \
HexUctState.hpp \
HexUctUtil.hpp

libuct_a_CPPFLAGS = \
$(BOOST_CPPFLAGS) \
-DABS_TOP_SRCDIR='"@abs_top_srcdir@"' \
-DDATADIR='"$(pkgdatadir)"' \
-I$(FUEGO_ROOT)/smartgame \
-I@top_srcdir@/src/ \
-I@top_srcdir@/src/util \
-I@top_srcdir@/src/hex

DISTCLEANFILES = *~
