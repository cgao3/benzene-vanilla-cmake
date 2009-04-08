#!/usr/bin/python -u

#----------------------------------------------------------------------------
# $Id: twogtp.py 355 2007-05-09 19:24:45Z broderic $
#----------------------------------------------------------------------------

import fcntl, getopt, math, os, socket, string, sys, time
from random import randrange
from select import select

from program import Program 

#----------------------------------------------------------------------------

def printUsage():
    sys.stderr.write(
        "Usage: checkSgfs.py [program] [games]\n\n"
        "    Where [program] is any htp program that supports the commands\n"
        "    'loadsgf [sgf]' and 'final_score', and [games] is a non-empty\n"
        "    list of games in sgf format that need to be checked.\n\n"
        "    All games that were unfinished are reported.\n\n"
        "    FIXME: This script sometimes crashes after checking a lot\n"
        "    of games.  Just restart it near the game it crashed and it\n"
        "    should continue on fine.\n\n"
        )

#----------------------------------------------------------------------------

def checkgame(program, game):
    print "Checking " + game + "..."
    program.sendCommand("loadsgf " + game);
    res = program.sendCommand("final_score");
    res = res.strip();
    if ((res != 'B+') and (res != 'W+')):
        print game + ":" + program.sendCommand("showboard");
        print "Incomplete game!"
        print "Result: " + res
        
def main():
    if len(sys.argv) < 3:
        printUsage()
        return
    
    program = Program("B", sys.argv[1], False);
    for sgf in sys.argv[2:]:
        checkgame(program, sgf)
        
    print "Done."
main()

#----------------------------------------------------------------------------
