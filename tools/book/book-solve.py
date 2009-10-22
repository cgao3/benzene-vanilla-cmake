#!/usr/bin/python
#----------------------------------------------------------------------------
# Solves given set of positions using dfpn.
# 
# Such a set of positions can be generated using the HTP command
#   'book-dump-non-terminal [min # of stones] [output file]'
# to dump all leaf states with at least the given number of stones.
# The output file can then be used as the input file for this script.
#
# Results are dumped to [thread-name].solved and [thread-name].unsolved
# files for processing by other scripts. 
#----------------------------------------------------------------------------

import getopt, sys
from threading import Thread, Lock
from program import Program

#----------------------------------------------------------------------------

class Positions:
    def __init__(self, positionsFile):
        f = open(positionsFile, "r")
        self._lines = f.readlines()
        f.close()
        self._lock = Lock()

    def getPosition(self):
        self._lock.acquire();
        ret = "";
        if len(self._lines) > 0:
            ret = self._lines[0].strip()
            self._lines.pop(0)
        self._lock.release();
        return ret;

#----------------------------------------------------------------------------

class DfpnSolver(Thread):
    class Error:
        pass

    def __init__(self, name, command, positions, verbose):
        Thread.__init__(self)
        self._name = name
        self._positions = positions
        self._verbose = verbose
        command = command + " --logfile-name=" + name + ".log"
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
        if (self._verbose):
            print "#####################################"
            print self._name + ": " + variation
            print "#####################################"
        else:
            print self._name + ": " + variation
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

    def run(self):
        while True:
            variation = self._positions.getPosition();
            if variation == "":
                return;
            else:
                winner = self.solvePosition(variation).strip();
                self.addResult(variation, winner);
                print self._name + ": " + winner
                
#----------------------------------------------------------------------------

def printUsage():
    sys.stderr.write(
        "Usage: book-solve.py [options]\n"
        "Options:\n"
        "  --help      |-h: print help\n"
        "  --positions |-p: openings to use (required)\n"
        "  --program   |-c: program to run (required)\n"
        "  --threads   |-n: number of threads (default is 1)\n"
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

    solverlist = []
    for i in range(numThreads):
        solver = DfpnSolver("thread" + str(i), program, positions, verbose);
        solverlist.append(solver)
        solver.start()
    for solver in solverlist:
        solver.join()
    print "All threads finished."
    
main()
