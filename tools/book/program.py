#----------------------------------------------------------------------------
# Opens connection to GTP program. 
#
# FIXME: very similar to tournament/program.py. Make them share code?
#
#----------------------------------------------------------------------------

import string, os, sys, subprocess

#----------------------------------------------------------------------------

class Program:
    class CommandDenied(Exception):
        pass

    class Died(Exception):
        pass

    def __init__(self, command, verbose):
        self._command = command
        self._verbose = verbose
        if self._verbose:
            print "Creating program:", command
        self._stdin, self._stdout, self._stderr = os.popen3(command)
        self._isDead = 0

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

    def isDead(self):
        return self._isDead

    def sendCommand(self, cmd):
        try:
            if self._verbose:
                print "< " + cmd
            self._stdin.write(cmd + "\n")
            self._stdin.flush()
            return self._getAnswer()
        except IOError:
            self._programDied()

    def _getAnswer(self):
        answer = ""
        done = 0
        numberLines = 0
        while not done:
            line = self._stdout.readline()
            if line == "":
                self._programDied()
            if self._verbose:
                sys.stdout.write("> " + line)
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

    def _programDied(self):
        self._isDead = 1
        raise Program.Died
