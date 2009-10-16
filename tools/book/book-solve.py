#!/usr/bin/python
#----------------------------------------------------------------------------
# Solves given set of positions using dfpn.
#
# Results are dumped to [thread-name].solved and [thread-name].unsolved
# files for processing by other scripts. 
#----------------------------------------------------------------------------

import getopt, sys
from program import Program

#----------------------------------------------------------------------------

class Positions:
    def __init__(self, positionsFile):
        f = open(positionsFile, "r")
        self._lines = f.readlines()
        f.close()

    def getPosition(self):
        if len(self._lines) > 0:
            ret = self._lines[0].strip()
            self._lines.pop(0)
            return ret
        return ""

#----------------------------------------------------------------------------

class DfpnSolver:
    class Error:
        pass

    def __init__(self, name, command, positions, verbose):
        self._name = name
        self._positions = positions
        self._verbose = verbose
        self._program = Program(command, verbose)

    def sendCommand(self, command):
        try:
            return self._program.sendCommand(command);
        except Program.CommandDenied:
            reason = self._program.getDenyReason()
            self._errorMessage = _name + ": "  + reason
            raise DfpnSolver.Error
        except Program.Died:
            self._errorMessage = _name + ": program died"
            raise DfpnSolver.Error

    def playVariation(self, variation):
        self.sendCommand("clear_board");
        moves = variation.split(' ');
        color = "B";
        for move in moves:
            cmd = "play " + color + " " + move.strip();
            self.sendCommand(cmd);
            if color == "B":
                color = "W";
            else:
                color = "B";
            
    def solvePosition(self, variation):
        print "#####################################"
        print self._name + ": " + variation
        print "#####################################"
        self.playVariation(variation);
        return self.sendCommand("dfpn-solve-state");

    def addResult(self, variation, winner):
        if (winner == "empty"):
            f = open(self._name + ".unsolved", "a");
            print >> f, variation;
            f.close();
        else:
            f = open(self._name + ".solved", "a");
            print >> f, variation + " " + winner
            f.close();

    def solve(self):
        while True:
            variation = self._positions.getPosition();
            if variation == "":
                return;
            else:
                winner = self.solvePosition(variation).strip();
                self.addResult(variation, winner);
                print "Result = " + winner
                
#----------------------------------------------------------------------------

def printUsage():
    sys.stderr.write(
        "Usage: book-solve.py [options]\n"
        "Options:\n"
        "  --help      |-h: print help\n"
        "  --positions |-p: openings to use (required)\n"
        "  --program   |-c: program to run (required)\n"
        "  --threads   |-n: number of threads (not implemented)\n"
        "  --quiet     |-q: be quiet\n");
    
#----------------------------------------------------------------------------

def main():
    verbose = True
    program = ""
    positionFile = ""
    numThreads = 1
    
    try:
        options = "hp:c:n:q"
        longOptions = ["help", "positions=", "program=", "threads=", "quiet"];
        opts, args = getopt.getopt(sys.argv[1:], options, longOptions)
    except getopt.GetoptError:
        printUsage()
        sys.exit(1)
        
    for o, v in opts:
        if o in ("-h", "--help"):
            printUsage();
            sys.exit();
        elif o in ("-p", "--positions"):
            positionFile = v;
        elif o in ("-c", "--program"):
            program = v;
        elif o in ("-n", "--threads"):
            numThreads = int(v);
        elif o in ("-q", "--quiet"):
            verbose = False

    if (positionFile == "" or program == ""):
        printUsage()
        sys.exit(1)

    positions = Positions(positionFile);
    solver = DfpnSolver("thread0", program, positions, verbose);
    solver.solve()

main()
