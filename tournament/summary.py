#!/usr/bin/python -u
#----------------------------------------------------------------------------
# Summarizes a twogtp tournament.
#
# TODO: - Simplify stuff. The table idea seems bad, in retrospect.
#       - Do we really care about which openings are won/lost?

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
p1Overtime = Statistics()
p2Overtime = Statistics()
p1Wins = Statistics()
p1WinsBlack = Statistics()
p1WinsWhite = Statistics()

def analyzeTourney(fname, longOpening, maxvalid, showTable, timeLimit):
    print "Analyzing: \'" + fname + "\'..."
    
    f = open(fname, "r")
    line = f.readline()
    linenum = 1

    numvalid = 0
    table = {}
    opencount = {}
    openings = []
    progs = []
    
    p1Timeouts = 0.0
    p2Timeouts = 0.0
    
    while line != "":
        if line[0] != "#":
            array = string.split(line, "\t")

            fullopening = array[2]
            black = array[3]
            white = array[4]
            bres = array[5]
            wres = array[6]
            length = array[7]
            timeBlack = float(array[8])
            timeWhite = float(array[9])

            if longOpening:
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

                if (((timeBlack > timeLimit) and (bres[0] == 'B')) or
                    ((timeWhite > timeLimit) and (bres[0] == 'W'))):
                    overtime = 0.0
                    if (bres[0] == 'B'):
                        overtime = timeBlack - timeLimit
                    else:
                        overtime = timeWhite - timeLimit
                    
                    if (winner == progs[0]):
                        p1Timeouts = p1Timeouts + 1.0
                        p1Overtime.add(overtime)
                    else:
                        p2Timeouts = p2Timeouts + 1.0
                        p2Overtime.add(overtime)

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

        
    showIterativeResults(numvalid, table, opencount,
                         openings, progs, showTable,
                         p1Timeouts, p2Timeouts)

#    for k in sorted(table.keys()):
#       print k, table[k]

def showIterativeResults(numvalid, table, opencount, openings,
                         progs, showTable, p1Timeouts, p2Timeouts):

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
        print "Statistics for " + progs[0] + ":"                
        print "  All Wins: %.1f%% (+-%.1f)" % \
              (p1Wins.mean()*100.0, p1Wins.stderror()*100.0)
        print "  As Black: %.1f%% (+-%.1f)" % \
              (p1WinsBlack.mean()*100.0, p1WinsBlack.stderror()*100.0)
        print "  As White: %.1f%% (+-%.1f)" % \
              (p1WinsWhite.mean()*100.0, p1WinsWhite.stderror()*100.0)
        
        if ((p1Timeouts > 0) or (p2Timeouts > 0)):
            print "-----------------------------------------------------------"
            print "Timeouts for " + progs[0] + ": %i/%i, %.1f (+-%.1f) max=%.1f" % \
                  (p1Timeouts, p1Wins.sum(),
                   p1Overtime.mean(), p1Overtime.stderror(), p1Overtime.max())
            print "Timeouts for " + progs[1] + ": %i/%i, %.1f (+-%.1f) max=%.1f" % \
                  (p2Timeouts, p1Wins.count() - p1Wins.sum(),
                   p2Overtime.mean(), p2Overtime.stderror(), p2Overtime.max())
        
        print "==========================================================="
    else:
        print "No valid games."
        
    
#------------------------------------------------------------------------------

def usage():
        print "Usage: ./summary [--count c] [--showTable] [--time max time] --file [tournament.result]"
        sys.exit(-1)
    
def main():

    count = 50000
    timeLimit = 123456.789
    longOpening = False
    showTable = False
    resfile = ""
    
    try:
        options = "clr:sf:"
        longOptions = ["count=","long","showTable","file=","time="]
        opts, args = getopt.getopt(sys.argv[1:], options, longOptions)
    except getopt.GetoptError:
        usage()
    
    for o, v in opts:
        if o in ("--count", "-c"):
            count = int(v)
        elif o in ("--file", "-f"):
            resfile = v
        elif o in ("--time", "-t"):
            timeLimit = float(v)
        elif o in ("--long"):
            longOpening = True
        elif o in ("--showTable"):
            showTable = True
    
    if (resfile == ""):
        usage()
    
    analyzeTourney(resfile, longOpening, count, showTable, timeLimit)
    
main()
