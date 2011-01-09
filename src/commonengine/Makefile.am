noinst_LIBRARIES = libcommonengine.a

libcommonengine_a_SOURCES = \
CommonHtpEngine.cpp \
CommonProgram.cpp \
PlayAndSolve.cpp \
SwapCheck.cpp

noinst_HEADERS = \
CommonHtpEngine.hpp \
CommonProgram.hpp \
PlayAndSolve.hpp \
SwapCheck.hpp

libcommonengine_a_CPPFLAGS = \
$(BOOST_CPPFLAGS) \
-DABS_TOP_SRCDIR='"@abs_top_srcdir@"' \
-DDATADIR='"$(pkgdatadir)"' \
-I$(FUEGO_ROOT)/gtpengine \
-I$(FUEGO_ROOT)/smartgame \
-I@top_srcdir@/src/ \
-I@top_srcdir@/src/util \
-I@top_srcdir@/src/hex \
-I@top_srcdir@/src/book \
-I@top_srcdir@/src/uct \
-I@top_srcdir@/src/solver

DISTCLEANFILES = *~

