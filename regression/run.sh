#!/bin/sh
#-----------------------------------------------------------------------------
# Run regression tests.
#-----------------------------------------------------------------------------

PROGRAM="../src/mohex/mohex"
RESULT_DIR="html"

#-----------------------------------------------------------------------------
# Define functions

usage() {
  cat >&2 <<EOF
Usage: $0 [options] [test suite or single test file]
Options:
  -h Print help and exit
  -l Long output
  -p [program name]
Test suites:
  basics (*default*)
  all (all regression tests, no performance tests)
  *.tst (run single test)
EOF
}

#-----------------------------------------------------------------------------
# Parse options and arguments

USE_LONG_OUTPUT=0
while getopts "hlp:" O; do
case "$O" in
  h)   usage; exit 0;;
  l)   USE_LONG_OUTPUT=1;;
  p)   PROGRAM=$OPTARG;;
  [?]) usage; exit -1;;
esac
done

shift $(($OPTIND - 1))
if [ $# -gt 1 ]; then
  usage
  exit -1
fi
if [ $# -eq 0 ]; then
  NAME="basics"
else
  NAME="$1"
fi

#-----------------------------------------------------------------------------
# Select tests

case "$NAME" in
all)
  TST="\
  basics.tst \
  htp.tst \
  play.tst \
  ice.tst \
  vc.tst \
  dfs-solve-6x6.tst \
  dfs-solve-6x7.tst \
  dfs-solve-7x7.tst \
  dfs-solve-7x8.tst \
  dfpn-solve-6x6.tst \
  dfpn-solve-7x7.tst \
  ";;
basics)
  # Tests in basics should be fast, specific and produce no unexpected fails
  # or passes.
  TST="\
  basics.tst \
  htp.tst \
  play.tst \
  ice.tst \
  dfs-solve-6x6.tst \
  dfs-solve-6x7.tst \
  dfpn-solve-6x6.tst
  ";;
*.tst)
  TST=$NAME;;
*)
  usage
  exit -1;;
esac

#-----------------------------------------------------------------------------
# Run tests

echo >&2 "Program: '$PROGRAM'"

TEST_SUBDIR=$NAME
if [ -f "$NAME" ]; then
  # If the test case name is the name of a single test file, use only the file
  # name for the name of the subdirectory in RESULT_DIR, not the full path.
  TEST_SUBDIR=$(basename $NAME)
fi
TEST_DIR="$RESULT_DIR/$TEST_SUBDIR"
mkdir -p "$TEST_DIR"
echo >&2 "Output:  $TEST_DIR/index.html"

if [ $USE_LONG_OUTPUT -ne 0 ]; then
  OPT="-long"
fi
./gtpregress $OPT -output "$TEST_DIR" "$PROGRAM" $TST

#-----------------------------------------------------------------------------
