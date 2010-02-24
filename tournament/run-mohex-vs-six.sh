#!/bin/bash
# Runs a tournament between mohex and six.

me=$0
function usage()
{
    echo "Usage:"
    echo "    $me [OPTIONS] [six executable] [mohex config].htp"
    echo ""
    echo "Where OPTIONS is any of:"
    echo "-o | --openings=name     set of openings to use"
    echo "-r | --rounds=#          number of rounds to play"
    echo "-s | --size=#            boardsize to play on"
    echo 
}

source common.sh
if [ $# != 2 ]; then
    usage;
    exit 1;
fi

SIX=$1
NAME1=mohex-$2

DIRECTORY="jobs/"$NAME1"-vs-six"
mkdir -p $DIRECTORY

./twogtp.py \
--type $TYPE \
--dir $DIRECTORY \
--openings $OPENINGS \
--size $SIZE --rounds $ROUNDS \
--p1cmd "../src/mohex/mohex --quiet --config $2.htp" --p1name $NAME1 \
--p2cmd "$SIX --quiet" --p2name six

