#!/bin/bash

# Script to build Benzene and run unit and basic regression tests
# as a daily cron job.

# This script will check out Benzene in the parent directory
# TEST_DIR. The files gogui-regress and gogui-regress.jar must already
# be in TEST_DIR. 
#
# Any errors will be reported to REPORT_EMAIL.

TEST_DIR=/local/scratch/broderic/benzene-daily
GIT_BACKUP_DIR=~broderic/hex/benzene.git.sf.backup/
REPORT_EMAIL="hexml@lists.cs.ualberta.ca"
RESULT_TABLE_LOCAL=$TEST_DIR/result.dat
MAIL_EXEC=`which mail`

mail-error() {
  FAILURE="$1"
  OUTPUT="$2"
  DATE=$(date)
  cat <<EOF > $TEST_DIR/last_error
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
  $MAIL_EXEC -s "Benzene Error Report" "$REPORT_EMAIL" < $TEST_DIR/last_error
  echo -e "$(date +'%F')\t$FAILURE" >> $RESULT_TABLE_LOCAL
  echo "Failure: $FAILURE"
  exit 1
}

run-checked() {
  CMD=$1
  TITLE=$2
  OUTPUT=$($CMD 2>&1) || mail-error "$TITLE ($CMD)" "$OUTPUT"
}

run-function()
{
    FUNC=$1
    echo "**** Running '$FUNC' ****"
    OUT=$($FUNC 2>&1)
    RET=$?
    if [ $RET == 0 ]; then
        echo "GOOD"
        return 0
    elif [ $RET == 1 ]; then
        exit 1
    fi
    echo "ERROR"
    mail-error "Error in $FUNC()" "$OUT"
}

# Each of these functions is called by run-function() as a command.
# If the run fails a run-checked(), run-checked() will mail the error
# and abort with error 1. If there is some other error in the
# function that is not inside a run-checked(), the function should
# abort with error code 2.  run-function() will report error code 2s with
# mail-error() and stop the testing.
build-fuego()
{
    PATH=$TEST_DIR:/usr/local/bin:$PATH
    cd $TEST_DIR || return 2
    echo "Checking for gogui-regress..."
    test -x ./gogui-regress || return 2
    test -f ./gogui-regress.jar || return 2
    echo "Found!"

    cd $TEST_DIR || return 2
    rm -rf fuego
    svn -q co https://fuego.svn.sourceforge.net/svnroot/fuego/trunk fuego || return 2
    cd fuego || return 2
    autoreconf -i || return 2
    run-checked "./configure --enable-assert" "FUEGO CONFIGURE"
    run-checked "make -s" "FUEGO COMPILATION"
    return 0
}

build-benzene()
{
    # Backup the benzene repository
    rsync -av benzene.git.sourceforge.net::gitroot/benzene/* $GIT_BACKUP_DIR

    # Clone and build benzene
    cd $TEST_DIR || return 2
    rm -rf benzene
    git clone git://benzene.git.sourceforge.net/gitroot/benzene/benzene benzene || return 2
    cd benzene || return 2
    autoreconf -i
    run-checked "./configure --enable-assert --with-fuego-root=$TEST_DIR/fuego" "BENZENE CONFIGURE"
    run-checked "make -s" "BENZENE COMPILATION"
    run-checked "make -s check" "BENZENE MAKE CHECK"
    cd $TEST_DIR/benzene/regression || return 2
    run-checked "./run.sh" "BENZENE BASIC REGRESSION TESTS"
}

rm -f $TEST_DIR/last_error
run-function "build-fuego" \
    && run-function "build-benzene"

echo -e "$(date +'%F')\tSUCCESS" >> $RESULT_TABLE_LOCAL
echo "Success"

