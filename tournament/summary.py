#!/usr/bin/python -u
#----------------------------------------------------------------------------
# $Id: summary.py 1875 2009-01-28 01:43:42Z broderic $
#----------------------------------------------------------------------------
#
# TODO: - Simplify stuff. The table idea seems bad, in retrospect.
#       - Do we really need the random tourney crap?
#       - Do we really care about which openings are won/lost?
#         Isn't that a little results oriented? :)
#         This script would be soo small if both were removed. 

import os, sys, getopt, re, string
from statistics import Statistics

#----------------------------------------------------------------------------

def get_value(table, key):
    if (table.has_key(key)):
        return table[key]
    return 0    

def add_if_new(list, value):
    if (list.count(value)==0):
        list.append(value)

# Yes, I'm using globals. Yes, I'm lazy.
gamelen = Statistics()
elapsedP1 = Statistics()
elapsedP2 = Statistics()
p1Wins = Statistics()
p1WinsBlack = Statistics()
p1WinsWhite = Statistics()
   
def analyzeTourney(filename, random, longOpening, maxvalid, showTable):
    print "Analyzing: \'" + filename + "\'..."
    
    f = open(filename, "r")
    line = f.readline()
    linenum = 1

    numvalid = 0
    table = {}
    opencount = {}
    openings = []
    progs = []
  
    while line != "":
        if line[0] != "#":
            array = string.split(line, "\t")

            fullopening = array[2]
            black = array[3]
            white = array[4]
            bres = array[5]
            wres = array[6]
            length = array[7]
            timeBlack = array[8]
            timeWhite = array[9]

            if random or longOpening:
                opening = string.strip(fullopening)
            else:
                moves = string.split(string.strip(fullopening), ' ')
                opening = moves[0]

            # TODO: check that results are of the form "C+", where C
            # is one of 'B' or 'W', instead of just checking the first
            # character.
            if ((numvalid < maxvalid) and (bres == wres) and
                ((bres[0] == 'B') or (bres[0] == 'W'))):
                add_if_new(openings, opening)
                add_if_new(progs, black)
                add_if_new(progs, white)

                colors = ['B', 'W']
                names = [black, white]
                winner = names[colors.index(bres[0])]
                valueForP1 = 0.0
                if (winner == progs[0]):
                    valueForP1 = 1.0
                
                gamelen.add(float(length))
                p1Wins.add(valueForP1)
                if (progs[0] == black):
                    p1WinsBlack.add(valueForP1)
                else:
                    p1WinsWhite.add(valueForP1)

                if (progs[0] == black):
                    elapsedP1.add(float(timeBlack))
                    elapsedP2.add(float(timeWhite))
                else:
                    elapsedP1.add(float(timeWhite))
                    elapsedP2.add(float(timeBlack))

                opencount[opening] = get_value(opencount, opening) + 1
           
                for color in colors:
                    who = names[colors.index(color)]
                    if (bres[0] == color):
                        key = opening, color, who, 'win'
                        table[key] = get_value(table, key) + 1
                    else:
                        key = opening, color, who, 'loss'
                        table[key] = get_value(table, key) + 1

                numvalid = numvalid + 1
            elif (numvalid >= maxvalid):
                print "Line " + str(linenum) + ": Past max game limit."
            else:
                print "Line " + str(linenum) + ": Ignoring bad game result."
                
        line = f.readline()
        linenum = linenum+1
    f.close()

    print ""
    for p in progs:
        print "p" + str(progs.index(p)+1)+ " = " + p

    if random:
        showRandomResults(numvalid, table, opencount,
                          openings, progs, showTable)
    else:
        showIterativeResults(numvalid, table, opencount,
                             openings, progs, showTable)

#    for k in sorted(table.keys()):
#       print k, table[k]

def showRandomResults(numvalid, table, opencount, openings, progs):

    split = 0
    nonsplit = 0

    p1nsw = 0
    p1nsl = 0
    p2nsw = 0
    p2nsl = 0

    p1w = 0
    p2w = 0
    
    # count split/non-split openings
    for o in openings:
        if ((opencount[o] % 2) != 0):
            print "Skipping corrupted/unfinished opening '" + o + "'"
            continue

        if (opencount[o] > 2):
            print "'" + o + "' played more than once!" 

        cb1w = get_value(table, (o, 'B', progs[0], 'win'))
        cb1l = get_value(table, (o, 'B', progs[0], 'loss'))
        cb2w = get_value(table, (o, 'B', progs[1], 'win'))        
        cb2l = get_value(table, (o, 'B', progs[1], 'loss'))
        cw1w = get_value(table, (o, 'W', progs[0], 'win'))
        cw1l = get_value(table, (o, 'W', progs[0], 'loss'))
        cw2w = get_value(table, (o, 'W', progs[1], 'win'))
        cw2l = get_value(table, (o, 'W', progs[1], 'loss'))

        p1w = p1w + cb1w + cw1w
        p2w = p2w + cb2w + cw2w
        
        # non-split?
        if (cb1w != cb2w):
            print o
            if (cb1w > cb2w):
                p1nsw = p1nsw + cb1w
                p2nsl = p2nsl + cb1w
            else:
                p1nsl = p1nsl + cb2w
                p2nsw = p2nsw + cb2w
            nonsplit = nonsplit+1
        else:
            split = split+1


    print "+--------------+-------+-------+"
    print "|   Non-split  |  p1   |  p2   |"
    print "+--------------+-------+-------+"
    print "|%4d\t\t%3i/%i\t%3i/%i  |" % (nonsplit, p1nsw, p1nsl, p2nsw, p2nsl)
    print "+------------------------------+"

    if (p1nsw+p1nsl != 0):
        print "Winning PCT in Non-Split Openings:"
        print "%s:\t%.1f%%" % (progs[0], (p1nsw*100.0/(p1nsw+p1nsl)))
        print "%s:\t%.1f%%" % (progs[1], (p2nsw*100.0/(p2nsw+p2nsl)))
    print ""

    if (nonsplit + split > 0):
        print "Non-Split/Total: %i/%i (%.1f%%)" % \
              (nonsplit, nonsplit+split, (nonsplit*100.0/(nonsplit+split)))
        print ""
    
    if (numvalid != 0):
        print "Total Win PCT:"
        print "%s:\t%.1f%%" % (progs[0], (p1w*100.0/(p1w+p2w)))
        print "%s:\t%.1f%%" % (progs[1], (p2w*100.0/(p1w+p2w)))
    print ""

def showIterativeResults(numvalid, table, opencount, openings,
                         progs, showTable):

    if showTable:
        print "+-------------+--------+-------+-------+"
        print "+   OPENING   | COUNT  |  p1   |  p2   |"
        print "+-------------+--------+-------+-------+"

    b1w = 0
    b1l = 0
    b2w = 0
    b2l = 0
    openings.sort()    
    for o in openings:
        cb1w = get_value(table, (o, 'B', progs[0], 'win'))
        cb1l = get_value(table, (o, 'B', progs[0], 'loss'))
        cb2w = get_value(table, (o, 'B', progs[1], 'win'))        
        cb2l = get_value(table, (o, 'B', progs[1], 'loss'))

        b1w = b1w + cb1w
        b1l = b1l + cb1l
        b2w = b2w + cb2w
        b2l = b2l + cb2l

        if showTable:
            print "|%s \t\t%4d\t%3i/%i\t%3i/%i  |" % \
                  (o, opencount[o], cb1w, cb1l, cb2w, cb2l)
       

    if showTable:
        print "+--------------------------------------+"    
        print "| \t\t \t%3i/%i\t%3i/%i |" % (b1w, b1l, b2w, b2l)
        print "+--------------------------------------+"    

    if (numvalid != 0):
        print
        print "==========================================================="
        print "  NumGames: " + str(numvalid)
        print "   GameLen: " + gamelen.dump()
        print "    p1Time: " + elapsedP1.dump()
        print "    p2Time: " + elapsedP2.dump()
        print "-----------------------------------------------------------"
        print "Statistics for \'" + progs[0] + "\':"                
        print "  All Wins: %.1f%% (+-%.1f)" % \
              (p1Wins.mean()*100.0, p1Wins.stderror()*100.0)
        print "  As Black: %.1f%% (+-%.1f)" % \
              (p1WinsBlack.mean()*100.0, p1WinsBlack.stderror()*100.0)
        print "  As White: %.1f%% (+-%.1f)" % \
              (p1WinsWhite.mean()*100.0, p1WinsWhite.stderror()*100.0)
        print "==========================================================="
    else:
        print "No valid games."
        
    
#------------------------------------------------------------------------------

def usage():
        print "Usage: ./summary [--random] [--count c] --file [tournament.result]"
        sys.exit(-1)
    
def main():

    count = 50000
    longOpening = False
    random = False
    showTable = False
    resfile = ""
    
    try:
        options = "clr:sf:"
        longOptions = ["count=", "long", "random","showTable", "file="]
        opts, args = getopt.getopt(sys.argv[1:], options, longOptions)
    except getopt.GetoptError:
        usage()
    
    for o, v in opts:
        if o in ("--count", "-c"):
            count = int(v)
        elif o in ("--file", "-f"):
            resfile = v
        elif o in ("--long"):
            longOpening = True
        elif o in ("--random"):
            random = True
        elif o in ("--showTable"):
            showTable = True
    
    if (resfile == ""):
        usage()
    
    analyzeTourney(resfile, random, longOpening, count, showTable)
    
main()
