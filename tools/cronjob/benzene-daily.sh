#!/bin/bash

# Script to build Benzene and run unit and basic regression tests
# as a daily cron job.

# This script will check out Benzene in the parent directory
# TEST_DIR. The files gogui-regress and gogui-regress.jar must already
# be in TEST_DIR. 
#
# Any errors will be reported to REPORT_EMAIL.
#
# Documentation will be copied to WEBPAGE.

TEST_DIR=/local/scratch/broderic/benzene-daily
#REPORT_EMAIL="hexml@lists.cs.ualberta.ca"
REPORT_EMAIL="broderic@cs.ualberta.ca"

WEBPAGE=~broderic/web_docs/hex/local
RESULT_TABLE=$WEBPAGE/autotest-result.dat
RESULT_TABLE_LOCAL=$TEST_DIR/result.dat

mail-error() {
  FAILURE="$1"
  OUTPUT="$2"
  DATE=$(date)
  /bin/mail -s "Benzene Error Report" "$REPORT_EMAIL" <<EOF
----------------------------------------------------------------------------
Benzene Error Report
----------------------------------------------------------------------------
Failure: $FAILURE
Date: $DATE
----------------------------------------------------------------------------
This is an automatically generated email for
reporting compilation, unit test or basic
regression test failures detected by a nightly
cron-job on tawayik. If you are responsible for
the error, please fix it soon !!! If not, bug
the person who is.
----------------------------------------------------------------------------
Output:

$OUTPUT
EOF
  echo -e "$(date +'%F')\t$FAILURE" >> $RESULT_TABLE_LOCAL
  cp $RESULT_TABLE_LOCAL $RESULT_TABLE
  echo "Failure: $FAILURE"
  exit 1
}

run-checked() {
  CMD=$1
  TITLE=$2
  OUTPUT=$($CMD 2>&1) || mail-error "$TITLE ($CMD)" "$OUTPUT"
}

# Fuego-0.3

cd $TEST_DIR || exit 1
rm -rf fuego-0.3
svn co https://fuego.svn.sourceforge.net/svnroot/fuego/branches/VERSION_0_3_FIXES/ fuego-0.3 || exit 1
cd fuego-0.3 || exit 1
aclocal
autoheader
autoreconf -i
./configure
run-checked "make" "FUEGO COMPILATION"

# Benzene

cd $TEST_DIR || exit 1
rm -rf benzene
svn co svn+ssh://games/usr/svnroot/project_wolve2.0/trunk/ benzene || exit 1
cd benzene || exit 1
aclocal
autoheader
autoreconf -i
env FUEGO_ROOT=$TEST_DIR/fuego-0.3 ./configure
run-checked "make" "BENZENE COMPILATION"

run-checked "make check" "BENZENE MAKE CHECK"
run-checked "src/test/benzene_unittest" "BENZENE UNIT TESTS"

cd $TEST_DIR/benzene/regression
run-checked "./run.sh" "BENZENE BASIC REGRESSION TESTS"

cd $TEST_DIR/benzene/doc
make -s

tar czf benzene-doc.tar.gz benzene-doc/
scp benzene-doc.tar.gz $WEBPAGE
cd $WEBPAGE && rm -rf benzene-doc && tar xzf benzene-doc.tar.gz && chmod a+rx -R benzene-doc

echo -e "$(date +'%F')\tSUCCESS" >> $RESULT_TABLE_LOCAL
cp $RESULT_TABLE_LOCAL $RESULT_TABLE
echo "Success"
