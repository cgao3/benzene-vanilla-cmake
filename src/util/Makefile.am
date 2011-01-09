noinst_LIBRARIES = libutil.a

libutil_a_SOURCES = \
BenzeneException.cpp \
BenzeneProgram.cpp \
Bitset.cpp \
Logger.cpp \
lssolve.cpp \
Misc.cpp

noinst_HEADERS = \
BenzeneBitset.hpp \
BenzeneException.hpp \
BenzeneProgram.hpp \
Benzene.hpp \
Bitset.hpp \
ChangeLog.hpp \
Digraph.hpp \
HashDB.hpp \
Hash.hpp \
HashMap.hpp \
LinkedList.hpp \
Logger.hpp \
lssolve.h \
mat.hpp \
Misc.hpp \
SafeBool.hpp \
SortedSequence.hpp \
Types.hpp \
UnionFind.hpp \
vec.hpp

libutil_a_CPPFLAGS = \
$(BOOST_CPPFLAGS) \
-I$(FUEGO_ROOT)/gtpengine \
-I$(FUEGO_ROOT)/smartgame \
-I@top_srcdir@/src/

DISTCLEANFILES = *~
