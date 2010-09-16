noinst_LIBRARIES = libbook.a

libbook_a_SOURCES = \
Book.cpp \
BookCheck.cpp \
BookCommands.cpp \
CacheBook.cpp

noinst_HEADERS = \
Book.hpp \
BookBuilder.hpp \
BookBuilderCommands.hpp \
BookCheck.hpp \
BookCommands.hpp \
CacheBook.hpp

libbook_a_CPPFLAGS = \
$(BOOST_CPPFLAGS) \
-DABS_TOP_SRCDIR='"@abs_top_srcdir@"' \
-DDATADIR='"$(pkgdatadir)"' \
-I$(FUEGO_ROOT)/gtpengine \
-I$(FUEGO_ROOT)/smartgame \
-I@top_srcdir@/src/ \
-I@top_srcdir@/src/hex \
-I@top_srcdir@/src/util

DISTCLEANFILES = *~
