#!/usr/bin/python -u
#----------------------------------------------------------------------------

import os, sys, getopt, re, string

#----------------------------------------------------------------------------

def listDifferences(filename1, filename2):
    print "Distinguishing: ", filename1, " and ", filename2
    f1 = open(filename1, "r")
    f2 = open(filename2, "r")
    line1 = f1.readline()
    line2 = f2.readline()
    linenum1 = 1
    linenum2 = 1
    totaldiff = 0
    morep1wins = 0
    morep2wins = 0
    evengame = 1
    
    while ((line1 != "") and (line1[0] == "#")):
        line1 = f1.readline()
        linenum1 = linenum1 + 1
    while ((line2 != "") and (line2[0] == "#")):
        line2 = f2.readline()
        linenum2 = linenum2 + 1
    
    print "Different result games:"
    while ((line1 != "") and (line2 != "")):
        array1 = string.split(line1, "\t")
        gamenum1 = array1[0]
        fullopening1 = array1[2]
        bres1 = array1[5]
        wres1 = array1[6]
        
        array2 = string.split(line2, "\t")
        gamenum2 = array2[0]
        fullopening2 = array2[2]
        bres2 = array2[5]
        wres2 = array2[6]
        
        if (gamenum1 != gamenum2):
            print "Games do not align at lines", linenum1, ", ", linenum2
            f1.close()
            f2.close()
            sys.exit(-1)
        if (fullopening1 != fullopening2):
            print "Openings mismatch at lines", linenum1, ", ", linenum2
            f1.close()
            f2.close()
            sys.exit(-1)
        
        if ((bres1 != wres1) or (bres2 != wres2)):
            print "Players disagree on result in game " + str(gamenum1) + "!"
        elif (bres1 != bres2):
            totaldiff = totaldiff + 1
            oldmorep1wins = morep1wins
            
            if (bres2 == "B+"):
                morep1wins = morep1wins + evengame
                morep2wins = morep2wins + 1 - evengame
            elif (bres2 == "W+"):
                morep1wins = morep1wins + 1 - evengame
                morep2wins = morep2wins + evengame
            else:
                print "Invalid result for game " + str(gamenum1) + "!"
            
            if (oldmorep1wins < morep1wins):
                print "Game " + str(gamenum1) + ": P1 gain"
            else:
                print "Game " + str(gamenum1) + ": P2 gain"
        
        line1 = f1.readline()
        linenum1 = linenum1 + 1
        while ((line1 != "") and (line1[0] == "#")):
            line1 = f1.readline()
            linenum1 = linenum1 + 1
        line2 = f2.readline()
        linenum2 = linenum2 + 1
        while ((line2 != "") and (line2[0] == "#")):
            line2 = f2.readline()
            linenum2 = linenum2 + 1
        evengame = 1 - evengame
    
    f1.close()
    f2.close()
    print "Total different outcomes: %d" % totaldiff
    print "New Player 1 wins: %d" % morep1wins
    print "New Player 2 wins: %d" % morep2wins

#------------------------------------------------------------------------------

def usage():
        print "Usage: ./difference --file1 [tournament1.result] --file2 [tournament2.result]"
        print "Note: cannot be used to compare random tournaments"
        sys.exit(-1)

def main():
    resfile1 = ""
    resfile2 = ""
    
    try:
        options = "f:g:"
        longOptions = ["file1=", "file2="]
        opts, args = getopt.getopt(sys.argv[1:], options, longOptions)
    except getopt.GetoptError:
        usage()
    
    for o, v in opts:
        if o in ("--file1", "-f"):
            resfile1 = v
        elif o in ("--file2", "-g"):
            resfile2 = v
    
    if ((resfile1 == "") or (resfile2 == "")):
        usage()
    
    listDifferences(resfile1, resfile2)
    
main()
