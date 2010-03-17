#!/bin/bash
# Runs a tournament between two hex programs.
source common.sh

me=$0
function usage()
{
    echo "Usage:"
    echo "    $me [OPTIONS] [exec1] [config1].htp [exec2] [config2].htp"
    echo ""
    echo "Where OPTIONS is any of:"
    echo "-o | --openings=name     set of openings to use"
    echo "-r | --rounds=#          number of rounds to play"
    echo "-s | --size=#            boardsize to play on"
    echo 
}
if [ $# != 4 ]; then
    usage;
    exit 1;
fi

run_tournament $1 $2 $3 $4
