#!/bin/bash
# Runs a tournament between two instances of wolve.

source common.sh

me=$0
function usage()
{
    echo "Usage:"
    echo "    $me [OPTIONS] [config1].htp [config2].htp"
    echo ""
    echo "Where OPTIONS is any of:"
    echo "-o | --openings=name     set of openings to use"
    echo "-r | --rounds=#          number of rounds to play"
    echo "-s | --size=#            boardsize to play on"
    echo 
}
if [ $# != 2 ]; then
    usage;
    exit 1;
fi

run_tournament ../src/wolve/wolve $1 ../src/wolve/wolve $2

