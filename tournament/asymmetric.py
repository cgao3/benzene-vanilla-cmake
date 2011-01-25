#!/usr/bin/python -u

import os, sys, getopt, re, string

#----------------------------------------------------------------------------

def listAsymmetries(filename):
    print "Analyzing: ", filename
    f = open(filename, "r")
    line1 = f.readline()
    line2 = line1
    linenum = 1
    numsteals = 0
    numpairs = 0
    p1steals = 0
    p2steals = 0
    
    while (line1[0] == "#"):
        line1 = f.readline()
        linenum = linenum + 1
    line2 = f.readline()
    linenum = linenum + 1
    while (line2[0] == "#"):
        line2 = f.readline()
        linenum = linenum + 1
    
    print "Stolen games:"
    while ((line1 != "") and (line2 != "")):
        array1 = string.split(line1, "\t")
        gamenum1 = array1[0]
        fullopening1 = array1[2]
        black1 = array1[3]
        white1 = array1[4]
        bres1 = array1[5]
        wres1 = array1[6]
        
        array2 = string.split(line2, "\t")
        gamenum2 = array2[0]
        fullopening2 = array2[2]
        black2 = array2[3]
        white2 = array2[4]
        bres2 = array2[5]
        wres2 = array2[6]
        
        if (fullopening1 != fullopening2):
            print "Openings mismatch at line", linenum
            f.close()
            sys.exit(-1)
        
	if ((bres1 != wres1) or (bres2 != wres2)):
	    print "Results mismatch at line", linenum
            f.close()
	    sys.exit(-1)

	numpairs = numpairs + 1

	# if consecutive games go BW or WB, then one program won both games. 
	# If BB or WW, then they split. 
        if (bres1 != bres2):
            if (bres1 == "B+"):
                p1steals = p1steals + 1
                print "Games " + str(gamenum1) + ", " + str(gamenum2) + " - P1"
            elif (bres1 == "W+"):
                p2steals = p2steals + 1
                print "Games " + str(gamenum1) + ", " + str(gamenum2) + " - P2"
            
            numsteals = numsteals + 1
        
        line1 = f.readline()
        linenum = linenum + 1
        while ((line1 != "") and (line1[0] == "#")):
            line1 = f.readline()
            linenum = linenum + 1
        if (line1 != ""):
            line2 = f.readline()
            linenum = linenum + 1
            while ((line2 != "") and (line2[0] == "#")):
                line2 = f.readline()
                linenum = linenum + 1
        
    
    f.close()
    print "Number of pairs: %d" % numpairs
    print "Number of steals: %d (%d%%)" % (numsteals, 100.0*numsteals/numpairs)
    print "Player 1 steals: %d (%d%%)" % (p1steals, 100.0*p1steals/numpairs)
    print "Player 2 steals: %d (%d%%)" % (p2steals, 100.0*p2steals/numpairs)

#------------------------------------------------------------------------------

def usage():
        print "Usage: ./asymmetric --file [tournament.result]"
        sys.exit(-1)

def main():
    resfile = ""
    
    try:
        options = "f:"
        longOptions = ["file="]
        opts, args = getopt.getopt(sys.argv[1:], options, longOptions)
    except getopt.GetoptError:
        usage()
    
    for o, v in opts:
        if o in ("--file", "-f"):
            resfile = v
    
    if (resfile == ""):
        usage()
    
    listAsymmetries(resfile)
    
main()
