# Runs a tournament between two instances of mohex.
#
# Usage: 
#     run-mohex-vs-mohex [config1].htp [config2].htp
# 
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
--openings openings/11x11-all-1ply \
--size 11 --rounds 10 \
--p1cmd "../src/mohex/mohex --quiet --config $1.htp" --p1name $NAME1 \
--p2cmd "../src/mohex/mohex --quiet --config $2.htp" --p2name $NAME2

