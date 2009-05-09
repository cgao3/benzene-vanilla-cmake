# Runs a tournament between wolve and six.
#
# Usage:
#     run-wolve-vs-six [config1].htp
#
NAME1=wolve-$1

DIRECTORY="jobs/"$NAME1"-vs-six
mkdir -p $DIRECTORY

./twogtp.py \
--dir $DIRECTORY \
--openings openings/11x11-all-1ply \
--size 11 --rounds 10 \
--p1cmd "../src/wolve/wolve --quiet --config $1.htp" --p1name $NAME1 \
--p2cmd "../src/six/six --quiet" --p2name six

