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
GIT_BACKUP_DIR=~broderic/hex/benzene.git.sf.backup/
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

# Fuego trunk

cd $TEST_DIR || exit 1
rm -rf fuego
svn co https://fuego.svn.sourceforge.net/svnroot/fuego/trunk fuego || exit 1
cd fuego || exit 1
autoreconf -i
./configure --enable-assert
run-checked "make" "FUEGO COMPILATION"

# Backup the benzene repository
rsync -av benzene.git.sourceforge.net::gitroot/benzene/* $GIT_BACKUP_DIR

# Clone and build benzene

cd $TEST_DIR || exit 1
rm -rf benzene
git clone git://benzene.git.sourceforge.net/gitroot/benzene/benzene benzene || exit 1
cd benzene || exit 1
autoreconf -i
./configure --enable-assert --with-fuego-root=$TEST_DIR/fuego
run-checked "make" "BENZENE COMPILATION"
run-checked "make check" "BENZENE MAKE CHECK"

# Run unit and regresion tests
run-checked "src/test/benzene_unittest" "BENZENE UNIT TESTS"

cd $TEST_DIR/benzene/regression
run-checked "./run.sh" "BENZENE BASIC REGRESSION TESTS"

cd $TEST_DIR/benzene/doc
make -s

# Build documentation and upload to server
tar czf benzene-doc.tar.gz benzene-doc/
scp benzene-doc.tar.gz $WEBPAGE
cd $WEBPAGE && rm -rf benzene-doc && tar xzf benzene-doc.tar.gz && chmod a+rX -R benzene-doc

echo -e "$(date +'%F')\tSUCCESS" >> $RESULT_TABLE_LOCAL
cp $RESULT_TABLE_LOCAL $RESULT_TABLE
echo "Success"
