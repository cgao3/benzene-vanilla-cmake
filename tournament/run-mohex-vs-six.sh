# Runs a tournament between mohex and six.
#
# Usage:
#     run-mohex-vs-six [six executable] [config1].htp
#
SIX=$1
NAME1=mohex-$2

DIRECTORY="jobs/"$NAME1"-vs-six"
mkdir -p $DIRECTORY

./twogtp.py \
--dir $DIRECTORY \
--openings openings/11x11-all-1ply \
--size 11 --rounds 10 \
--p1cmd "../src/mohex/mohex --quiet --config $2.htp" --p1name $NAME1 \
--p2cmd "$SIX --quiet" --p2name six

