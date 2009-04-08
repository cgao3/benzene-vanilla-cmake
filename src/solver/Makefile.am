noinst_LIBRARIES = libsolver.a

libsolver_a_SOURCES = \
SolvedState.cpp \
SolverDB.cpp \
Solver.cpp

noinst_HEADERS = \
SolvedState.hpp \
SolverDB.hpp \
Solver.hpp

libsolver_a_CPPFLAGS = \
$(BOOST_CPPFLAGS) \
-I$(FUEGO_ROOT)/smartgame \
-I@top_srcdir@/src/ \
-I@top_srcdir@/src/util \
-I@top_srcdir@/src/hex

DISTCLEANFILES = *~
