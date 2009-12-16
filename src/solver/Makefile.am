noinst_LIBRARIES = libsolver.a

libsolver_a_SOURCES = \
DfpnCommands.cpp \
DfpnSolver.cpp \
DfsCommands.cpp \
DfsData.cpp \
DfsSolver.cpp \
SolverDB.cpp

noinst_HEADERS = \
DfpnCommands.hpp \
DfpnSolver.hpp \
DfsCommands.hpp \
DfsData.hpp \
DfsSolver.hpp \
SolverDB.hpp

libsolver_a_CPPFLAGS = \
$(BOOST_CPPFLAGS) \
-DABS_TOP_SRCDIR='"@abs_top_srcdir@"' \
-I$(FUEGO_ROOT)/gtpengine \
-I$(FUEGO_ROOT)/smartgame \
-I@top_srcdir@/src/ \
-I@top_srcdir@/src/util \
-I@top_srcdir@/src/hex

DISTCLEANFILES = *~
