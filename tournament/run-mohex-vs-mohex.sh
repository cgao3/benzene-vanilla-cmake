# Runs a tournament between two instances of mohex.

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

source common.sh
if [ $# != 2 ]; then
    usage;
    exit 1;
fi

NAME1=mohex-$1
NAME2=mohex-$2

# Distinguish bewteen the instances if doing self-play so that the 
# logfiles are not clobbered.
if [ $NAME1 == $NAME2 ]; then
    NAME1=$NAME1"-a"
    NAME2=$NAME2"-b"
fi

DIRECTORY="jobs/"$NAME1"-vs-"$NAME2

mkdir -p $DIRECTORY

./twogtp.py \
--dir "$DIRECTORY" \
--openings $OPENINGS \
--size $SIZE --rounds $ROUNDS \
--p1cmd "../src/mohex/mohex --quiet --config $1.htp" --p1name $NAME1 \
--p2cmd "../src/mohex/mohex --quiet --config $2.htp" --p2name $NAME2

