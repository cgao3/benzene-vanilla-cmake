# Parses command-line options and sets variables.
#
# This file should not be run on its own. Calling script
# must define a 'usage()' function.

TEMP=`getopt -o ho:r:s:t:: --long help,openings:,rounds:,size:,type: -- "$@"`
if [ $? != 0 ] ; then echo "Terminating..." >&2 ; exit 1 ; fi
eval set -- "$TEMP"

SIZE=13
ROUNDS=10
OPENINGS=""
TYPE="iterative"

while true ; do
    case "$1" in
        -h|--help) usage; exit 1 ;;
	-o|--openings) OPENINGS=$2; shift 2;;
        -r|--rounds) ROUNDS=$2; shift 2;;
        -s|--size) SIZE=$2; shift 2;;
        -t|--type) TYPE=$2; shift 2;;
        --) shift ; break ;;
        *) echo "Internal error!" ; exit 1 ;;
    esac
done

# Set default openings if user does not specify them
if [ "$OPENINGS" == "" ]; then
    OPENINGS="openings/"$SIZE"x"$SIZE"-all-1ply"
fi

# Runs the tournament. 
run_tournament() 
{
    if [ $# != 4 ]; then
        usage;
        exit 1;
    fi

    PROGRAM1=$1
    CONFIG1=$2
    PROGNAME1=`basename $PROGRAM1`
    NAME1=$PROGNAME1-$CONFIG1
    PROGRAM2=$3
    CONFIG2=$4
    PROGNAME2=`basename $PROGRAM2`
    NAME2=$PROGNAME2-$CONFIG2

    # Distinguish between the instances if doing self-play so that the 
    # logfiles are not clobbered.
    if [ $NAME1 == $NAME2 ]; then
        NAME1=$NAME1"-a"
        NAME2=$NAME2"-b"
    fi

    DIRECTORY="jobs/"$SIZE"x"$SIZE"-"$NAME1"-vs-"$NAME2

    mkdir -p $DIRECTORY

    ./twogtp.py \
        --type $TYPE \
        --dir "$DIRECTORY" \
        --openings $OPENINGS \
        --size $SIZE --rounds $ROUNDS \
        --p1cmd "$PROGRAM1 --config $CONFIG1.htp" --p1name $NAME1 \
        --p2cmd "$PROGRAM2 --config $CONFIG2.htp" --p2name $NAME2
}
