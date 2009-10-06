noinst_LIBRARIES = libsolver.a

libsolver_a_SOURCES = \
DfpnCommands.cpp \
DfpnSolver.cpp \
SolvedState.cpp \
SolverCommands.cpp \
SolverDB.cpp \
Solver.cpp

noinst_HEADERS = \
DfpnCommands.hpp \
DfpnSolver.hpp \
SolvedState.hpp \
SolverCommands.hpp \
SolverDB.hpp \
Solver.hpp

libsolver_a_CPPFLAGS = \
$(BOOST_CPPFLAGS) \
-DABS_TOP_SRCDIR='"@abs_top_srcdir@"' \
-I$(FUEGO_ROOT)/gtpengine \
-I$(FUEGO_ROOT)/smartgame \
-I@top_srcdir@/src/ \
-I@top_srcdir@/src/util \
-I@top_srcdir@/src/hex

DISTCLEANFILES = *~
