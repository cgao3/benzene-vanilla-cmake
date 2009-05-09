# Runs a tournament between mohex and wolve.
#
# Usage:
#     run-mohex-vs-wolve [config1].htp [config2].htp
#
NAME1=mohex-$1
NAME2=wolve-$2

DIRECTORY="jobs/"$NAME1"-vs-"$NAME2
mkdir -p $DIRECTORY

./twogtp.py \
--dir $DIRECTORY \
--openings openings/11x11-all-1ply \
--size 11 --rounds 10 \
--p1cmd "../src/mohex/mohex --quiet --config $1.htp" --p1name $NAME1 \
--p2cmd "../src/wolve/wolve --quiet --config $2.htp" --p2name $NAME2

