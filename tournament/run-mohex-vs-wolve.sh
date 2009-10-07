# Runs a tournament between mohex and wolve.

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
NAME2=wolve-$2

DIRECTORY="jobs/"$NAME1"-vs-"$NAME2
mkdir -p $DIRECTORY

./twogtp.py \
--type $TYPE \
--dir $DIRECTORY \
--openings $OPENINGS \
--size $SIZE --rounds $ROUNDS \
--p1cmd "../src/mohex/mohex --quiet --config $1.htp" --p1name $NAME1 \
--p2cmd "../src/wolve/wolve --quiet --config $2.htp" --p2name $NAME2

