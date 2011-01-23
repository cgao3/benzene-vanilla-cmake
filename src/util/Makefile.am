noinst_LIBRARIES = libutil.a

libutil_a_SOURCES = \
BenzeneAssert.hpp \
BenzeneException.cpp \
BenzeneProgram.cpp \
Bitset.cpp \
Logger.cpp \
lssolve.cpp \
Misc.cpp

noinst_HEADERS = \
AtomicMemory.hpp \
BenzeneBitset.hpp \
BenzeneException.hpp \
BenzeneProgram.hpp \
Benzene.hpp \
Bitset.hpp \
ChangeLog.hpp \
Digraph.hpp \
HashDB.hpp \
HashMap.hpp \
LinkedList.hpp \
Logger.hpp \
lssolve.h \
mat.hpp \
Misc.hpp \
SafeBool.hpp \
SortedSequence.hpp \
TransTable.hpp \
Types.hpp \
UnionFind.hpp \
vec.hpp

libutil_a_CPPFLAGS = \
$(BOOST_CPPFLAGS) \
-DABS_TOP_SRCDIR='"@abs_top_srcdir@"' \
-DDATADIR='"$(pkgdatadir)"' \
-I$(FUEGO_ROOT)/gtpengine \
-I$(FUEGO_ROOT)/smartgame \
-I@top_srcdir@/src/

DISTCLEANFILES = *~
