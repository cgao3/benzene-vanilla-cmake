#!/usr/bin/python -u
#----------------------------------------------------------------------------
# Plays a tournament between two gtp programs.
#----------------------------------------------------------------------------

import getopt, sys;

from tournament import IterativeTournament, RandomTournament

#----------------------------------------------------------------------------

def printUsage():
    sys.stderr.write(
        "Usage: twogtp.py [options]\n"
        "Options:\n"
        "  --dir      |-o: directory for to store results\n"
        "  --help     |-h: print help\n"
        "  --openings |-l: openings to use\n"        
        "  --p1cmd    |-b: command of first program\n"
        "  --p1name   |  : name of first program\n"
        "  --p2cmd    |-w: command of second program\n"
        "  --p2name   |  : name of second program\n"
        "  --quiet    |-q: do not show log and board after each move\n"
        "  --rounds   |-r: number of rounds (default 1)\n"
        "  --size     |-s: boardsize (default 11)\n"
        "  --type     |-t: type of tournament ('iterative' or 'random')\n")
    
#----------------------------------------------------------------------------

def main():
    rounds = 1
    size = 11
    verbose = True
    p1name = ""
    p1cmd = ""
    p2name = ""
    p2cmd = ""
    openings = ""
    outdir = ""
    type = "iterative"
    
    try:
        options = "b:ho:s:w:ql:t:"
        longOptions = ["p1cmd=", "p1name=", "p2cmd=", "p2name=", \
                       "rounds=", "help", "dir=", "size=", \
                       "quiet", "openings=", "type="]
        opts, args = getopt.getopt(sys.argv[1:], options, longOptions)
    except getopt.GetoptError:
        printUsage()
        sys.exit(1)

    for o, v in opts:
        print "o = '" + o + "', v = '" + v + "'"
        if o in ("-b", "--p1cmd"):
            p1cmd = v
        elif o in ("--p1name"):
            p1name = v
        elif o in ("-w", "--p2cmd"):
            p2cmd = v
        elif o in ("--p2name"):
            p2name = v
        elif o in ("-r", "--rounds"):
            rounds = int(v)
        elif o in ("-h", "--help"):
            printUsage()
            sys.exit()
        elif o in ("-o", "--dir"):
            outdir = v
        elif o in ("-s", "--size"):
            size = int(v)
        elif o in ("-q", "--quiet"):
            verbose = False
        elif o in ("-l", "--openings"):
            openings = v
        elif o in ("-t", "--type"):
            type = v;

    if (p1cmd == "" or p2cmd == "" or p1name == "" or p2name == "" or
        openings == "" or outdir == ""):
        printUsage()
        sys.exit(1)

    if type == "random":
        RandomTournament(p1name, p1cmd, p2name, p2cmd, size, rounds, \
                         outdir, openings, verbose).playTournament()
    elif type == "iterative":
        IterativeTournament(p1name, p1cmd, p2name, p2cmd, size, rounds, \
                            outdir, openings, verbose).playTournament()
    else:
        print "Unknown tournament type!\n"

main()

#----------------------------------------------------------------------------
