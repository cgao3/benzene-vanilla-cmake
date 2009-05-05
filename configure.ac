dnl Process this file with autoconf to produce a configure script.

AC_INIT([benzene], [0.9.GIT])
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

dnl TODO: Find a better (more portable) way to enable more optimizations by
dnl default.
AC_ARG_ENABLE([optimize],
   AS_HELP_STRING([--enable-optimize], [set CXXFLAGS to -O3 -g (default is yes)]),
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

if test "x$FUEGO_ROOT" == "x"; then
   AC_MSG_ERROR(Environment variable FUEGO_ROOT must be set to define the location of the Fuego source package.)
fi
if test "x$FUEGO_BUILD_ROOT" == "x"; then
   FUEGO_BUILD_ROOT="$FUEGO_ROOT"
fi
AC_SUBST(FUEGO_ROOT)
AC_SUBST(FUEGO_BUILD_ROOT)

AX_CXXFLAGS_WARN_ALL
AX_CXXFLAGS_GCC_OPTION(-Wextra)

AC_OUTPUT([
Makefile 
regression/Makefile
share/Makefile
tools/Makefile
tournament/Makefile
src/Makefile 
src/util/Makefile 
src/hex/Makefile
src/uct/Makefile 
src/solver/Makefile 
src/player/Makefile 
src/wolve/Makefile 
src/mohex/Makefile
src/six/Makefile
src/test/Makefile
])
