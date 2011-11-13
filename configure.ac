dnl Process this file with autoconf to produce a configure script.

AC_INIT([benzene], [1.0.GIT])
AC_PREREQ([2.59])
AC_CONFIG_SRCDIR([src/hex/HexBoard.cpp])
AM_CONFIG_HEADER(src/config.h)
AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE
AC_PROG_CXX
AC_PROG_RANLIB
AM_PROG_CC_C_O
AX_BOOST_BASE([1.33.1])
AX_BOOST_THREAD
AX_BOOST_SYSTEM
AX_BOOST_FILESYSTEM
AX_BOOST_PROGRAM_OPTIONS
AX_BOOST_UNIT_TEST_FRAMEWORK
AX_BERKELEY_DB

dnl TODO: Find a better (more portable) way to enable more
dnl optimizations by default.
AC_ARG_ENABLE([optimize],
   AS_HELP_STRING([--enable-optimize], 
                  [add "-O3" to CXXFLAGS (default is yes)]),
   [optimize=$enableval],
   [optimize=yes])
if test "x$optimize" = "xyes"
then
	CXXFLAGS="-O3 -g"
fi

AC_ARG_ENABLE([assert],
   AS_HELP_STRING([--enable-assert], [enable assertions (default is no)]),
   [assert=$enableval],
   [assert=no])
if test "x$assert" = "xno"
then
	CXXFLAGS="$CXXFLAGS -DNDEBUG"
fi

dnl Check for gcc atomic builtins.
AC_MSG_CHECKING([for GCC atomic builtins])
AC_LINK_IFELSE([int main (int argc, char** argv) 
                {int a; __sync_fetch_and_add(&a, 1); 
                 __sync_bool_compare_and_swap(&a, 1, 2);}],
	       [has_gccatomicbuiltins=yes],
	       [has_gccatomicbuiltins=no])
AC_MSG_RESULT([$has_gccatomicbuiltins])
		
if test "x$has_gccatomicbuiltins" = "xyes"
then
	AC_DEFINE(HAVE_GCC_ATOMIC_BUILTINS, 1,
        define if __sync_fetch_and_add() and __sync_compare_and_swap() 
        are available)
fi

dnl Enable various extra-large boardsizes
AC_ARG_ENABLE([upto19x19],
   AS_HELP_STRING([--enable-upto19x19], 
                  [support boards up to 19x19 (default is no)]),
   [upto19x19=$enableval],
   [upto19x19=no])

AC_ARG_ENABLE([upto14x14],
   AS_HELP_STRING([--enable-upto14x14], 
                  [support boards up to 14x14 (default is no)]),
   [upto14x14=$enableval],
   [upto14x14=no])

AC_ARG_ENABLE([upto13x13],
   AS_HELP_STRING([--enable-upto13x13], 
                  [support boards up to 13x13 (default is no)]),
   [upto13x13=$enableval],
   [upto13x13=no])

if test "x$upto19x19" = "xyes"
then
        CXXFLAGS="$CXXFLAGS -DSUPPORT_19x19"
elif test "x$upto14x14" = "xyes"
then
        CXXFLAGS="$CXXFLAGS -DSUPPORT_14x14"
elif test "x$upto13x13" = "xyes"
then
        CXXFLAGS="$CXXFLAGS -DSUPPORT_13x13"
fi

dnl Location of the fuego source and libraries
AC_ARG_WITH([fuego-root],
	    AS_HELP_STRING([--with-fuego-root=DIR],
                           [location of the root of the Fuego source tree]),
	    [fuegoroot=$withval],
	    [])

AC_ARG_WITH([fuego-build],
	    AS_HELP_STRING([--with-fuego-build=DIR],
                           [location of the root of the Fuego build tree]),
	    [fuegobuild=$withval],
	    [])

if test "x$fuegoroot" = "x"; then
   AC_MSG_ERROR([The path to the Fuego source tree must be set using the "--with-fuego-root" option.])
elif test "x$fuegoroot" = "xyes"; then
   AC_MSG_ERROR([The option "--with-fuego-root" requires a path to the Fuego source tree.])
elif test "x$fuegoroot" = "xno"; then
   AC_MSG_ERROR([A Fuego source tree is required to build Benzene.])
else
   FUEGO_ROOT=$fuegoroot
fi

if test "x$fuegobuild" != "x"; then
    if test "x$fuegobuild" = "xyes"; then
       AC_MSG_ERROR([The option "--with-fuego-build" requires a path to the Fuego build tree.])
    elif test "x$fuegobuild" != "xno"; then
       FUEGO_BUILD=$fuegobuild
    fi
fi

if test "x$FUEGO_BUILD" = "x"; then
   FUEGO_BUILD=$FUEGO_ROOT
fi

AC_SUBST(FUEGO_ROOT)
AC_SUBST(FUEGO_BUILD)

AX_CXXFLAGS_WARN_ALL
dnl AX_CXXFLAGS_WARN_ALL should enable -pedantic and -ansi for GCC compilers
dnl but it doesn't always work 
AX_CXXFLAGS_GCC_OPTION(-pedantic)
AX_CXXFLAGS_GCC_OPTION(-ansi)
AX_CXXFLAGS_GCC_OPTION(-Wextra)
AX_CXXFLAGS_GCC_OPTION(-Wconversion)

AC_OUTPUT([
Makefile 
regression/Makefile
share/Makefile
tournament/Makefile
src/Makefile 
src/util/Makefile 
src/hex/Makefile
src/book/Makefile
src/solver/Makefile 
src/simpleplayers/Makefile
src/commonengine/Makefile 
src/wolve/Makefile 
src/mohex/Makefile
src/benzenetest/Makefile
src/test/Makefile
tools/Makefile
tools/mergesgf/Makefile
])
