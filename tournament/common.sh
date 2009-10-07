# Parses command-line options and sets variables.
#
# This file should not be run on its own. Calling script
# must define a 'usage()' function.

TEMP=`getopt -o ho:r:s:: --long help,openings:,rounds:,size: -- "$@"`
if [ $? != 0 ] ; then echo "Terminating..." >&2 ; exit 1 ; fi
eval set -- "$TEMP"

SIZE=11
ROUNDS=10
OPENINGS=""

while true ; do
    case "$1" in
        -h|--help) usage; exit 1 ;;
	-o|--openings) OPENINGS=$2; shift 2;;
        -r|--rounds) ROUNDS=$2; shift 2;;
        -s|--size) SIZE=$2; shift 2;;
        --) shift ; break ;;
        *) echo "Internal error!" ; exit 1 ;;
    esac
done

# Set default openings if user does not specify them
if [$OPENINGS == ""]; then
    OPENINGS="openings/"$SIZE"x"$SIZE"-all-1ply"
fi
