#!/bin/bash
#----------------------------------------------------------------------------
# Script for building the book in batches of M iterations.
#
# Usage example:
#   build-book.sh 220 230 240 250 260
#
# This would build the book for 50k iterations in five steps of 10k
# iterations. After each 10k run the book is backed-up to an appropriately
# named backup book (eg: 9x9.db.220k, then 9x9.db.230k, etc for the 
# example above). 
#----------------------------------------------------------------------------

ITERATIONS=10000
BASENAME=9x9
BASECOMMAND="src/mohex/mohex --boardsize 9 --config 9x9-book-config.htp"

BOOKFILE=$BASENAME.db
LOGFILE=$BASENAME.log

# Create 'book.input'
cat > book.input <<EOF
book-open $BOOKFILE
book-expand $ITERATIONS
book-refresh
quit
EOF

# Build the book in steps
for i in $@
do
    logfile=$LOGFILE"."$i"k"
    dbfile=$BOOKFILE"."$i"k"
    if [ -e $dbfile ]
    then
        echo "Error: "$dbfile" already exists!";
        break;
    fi
    $BASECOMMAND" --logfile-name=$logfile < book.input"
    cp $BOOKFILE $dbfile
done
