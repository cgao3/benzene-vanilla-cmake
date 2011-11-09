#!/bin/bash
#==================================================================
# Edit the options below to run a tournament by hand.
#
# Below is an example tournament with mohex on the local machine and
# wolve on torrington (i.e. a different machine on the network).  Note
# that the config files and the tournament directory should both be on
# a path that both machines can access (eg: somewhere under your home
# directory).
#
# The easiest way to use this file is to make a copy of it, edit it,
# and use that version to run your tournament.
# ==================================================================

TYPE=iterative
ROUNDS=10
SIZE=11
OPENINGS=openings/11x11-all-1ply

NAME1=mohex-10k
PROGCMD1=../src/mohex/mohex
CONFIG1=10k.htp

NAME2=wolve-2ply
PROGCMD2="ssh torrington ~/hex/benzene/src/wolve/wolve"
CONFIG2=~/2ply.htp

#==================================================================
# Do not edit below this line
#==================================================================

# Distinguish between the instances if doing self-play so that the 
# logfiles are not clobbered.
if [ $NAME1 == $NAME2 ]; then
    NAME1=$NAME1"-a"
    NAME2=$NAME2"-b"
fi
DIRECTORY="jobs/"$SIZE"x"$SIZE"-"$NAME1"-vs-"$NAME2
mkdir -p $DIRECTORY
./twogtp.py \
        --type "$TYPE" \
        --dir "$DIRECTORY" \
        --openings $OPENINGS \
        --size "$SIZE" --rounds "$ROUNDS" \
        --p1cmd "$PROGCMD1 --config $CONFIG1" --p1name $NAME1 \
        --p2cmd "$PROGCMD2 --config $CONFIG2" --p2name $NAME2
