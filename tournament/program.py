#----------------------------------------------------------------------------
# Connects to a Hex program.
#----------------------------------------------------------------------------

import os, string, sys, subprocess
from random import randrange
from select import select

#----------------------------------------------------------------------------

class Program:
    class CommandDenied(Exception):
        pass

    class Died(Exception):
        pass

    def __init__(self, color, command, logName, verbose):
        command = command.replace("%SRAND", `randrange(0, 1000000)`)
        self._command = command
        self._color = color
        self._verbose = verbose
        if self._verbose:
            print "Creating program:", command
        p = subprocess.Popen(command, shell=True,
                             stdin=subprocess.PIPE, 
                             stdout=subprocess.PIPE, 
                             stderr=subprocess.PIPE);
        (self._stdin, 
         self._stdout, 
         self._stderr) = (p.stdin, p.stdout, p.stderr)
        self._isDead = 0
        self._log = open(logName, "w")
        self._log.write("# " + self._command + "\n")

    def getColor(self):
        return self._color

    def getCommand(self):
        return self._command

    def getDenyReason(self):
        return self._denyReason

    def getName(self):
        name = "?"
        try:
            name = string.strip(self.sendCommand("name"))
            version = string.strip(self.sendCommand("version"))
            name += " " + version
        except Program.CommandDenied:
            pass
        return name

    def getResult(self):
        try:
            l = self.sendCommand("final_score")
            #s = string.split(l)[0]
            #return s
            return l.strip()
        except Program.CommandDenied:
            return "?"

    def getTimeRemaining(self):
        try:
            l = self.sendCommand("time_left");
            return l.strip();
        except Program.CommandDenied:
            return "?"

    def isDead(self):
        return self._isDead

    def sendCommand(self, cmd):
        try:
            self._log.write(">" + cmd + "\n")
            if self._verbose:
                print self._color + "< " + cmd
            self._stdin.write(cmd + "\n")
            self._stdin.flush()
            return self._getAnswer()
        except IOError:
            self._programDied()

    def _getAnswer(self):
        self._logStdErr()
        answer = ""
        done = 0
        numberLines = 0
        while not done:
            line = self._stdout.readline()
            if line == "":
                self._programDied()
            self._log.write("<" + line)
            if self._verbose:
                sys.stdout.write(self._color + "> " + line)
            numberLines += 1
            done = (line == "\n")
            if not done:
                answer += line
        if answer[0] != '=':
            self._denyReason = string.strip(answer[2:])
            raise Program.CommandDenied
        if numberLines == 1:
            return string.strip(answer[1:])
        return answer[2:]

    def _logStdErr(self):
        list = select([self._stderr], [], [], 0)[0]
        for s in list:
            self._log.write(os.read(s.fileno(), 8192))
        self._log.flush()

    def _programDied(self):
        self._isDead = 1
        self._logStdErr()
        raise Program.Died
