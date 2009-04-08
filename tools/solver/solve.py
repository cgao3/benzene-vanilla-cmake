#!/usr/bin/python -u

#----------------------------------------------------------------------------
# $Id: solve.py 1957 2009-03-14 20:55:49Z ph $
#----------------------------------------------------------------------------

import fcntl, getopt, math, os, socket, string, sys, time
from random import randrange
from select import select

#----------------------------------------------------------------------------

def printUsage():
    sys.stderr.write("Usage: solve.py [program] [boardsize] [numstones] [transtones] [openings]\n\n")

#----------------------------------------------------------------------------

def solve_opening(program, opening, numstones, transtones, size, sizestr, findall):
    print opening

    # create the .in file
    f = open('solver/'+sizestr+opening+'.in', 'w')
    f.write('boardsize ' + str(size) + ' ' + str(size) + '\n')
    f.write('play b ' + opening + '\n')
    if findall:
        f.write('solver-find-winning white')
    else:
        f.write('solve-state white')
        
    if (numstones != -1):
        f.write(' solver/db/' + sizestr + '.db '
                + str(transtones) + ' '
                + str(numstones))
    f.write('\n')
    
    f.write('quit' + '\n')
    f.close()

    # run solver
    command  = 'nice ' + program
    command += ' --quiet'
    command += ' --seed 1'
    command += ' --logfile-name solver/' + sizestr + opening + '.log'
    command += ' < solver/' + sizestr + opening + '.in >&'
    command += ' /dev/null'

    print command
    os.system(command);

def main():
    if len(sys.argv) < 6:
        printUsage()
        return

    program = sys.argv[1]
    size = int(sys.argv[2])
    numstones = int(sys.argv[3])
    transtones = int(sys.argv[4])
    openings = sys.argv[5]

    findall = False
    if ((len(sys.argv) == 7) and ('--find-all' == sys.argv[6])):
        print "Finding all winning moves from root state."
        findall = True
    
    sizestr = str(size) + "x" + str(size)
    
    print "Solving " + sizestr + "..."
    
    # read in openings
    f = open(openings, "r")
    openings = f.readlines()
    f.close()
    
    # solve each opening in the order given
    for opening in openings:
        opening = opening.strip()
        solve_opening(program, opening, numstones, transtones,
                      size, sizestr, findall)
    
    print "Done."

main()

#----------------------------------------------------------------------------
