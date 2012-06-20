#!/usr/bin/env python
#############################################################################
"""
 GoguiScript.py

 Run a game of Go via gogui tools.
 Parameters of the program to be optimized are set via GTP commands.
 This script will create a sub-directory for each game.
 Edit the option section to customize this script.

 see DummyScript.py for more info
"""
#############################################################################
import sys
import os
import shutil
import platform
import random
from program import Program
from gameplayer import GamePlayer
from game import Game

#############################################################################
# Preliminaries
#############################################################################

def randomOpening(size, seed):
    oldstate = random.getstate()
    random.seed(seed)
    r = random.randint(0, (size*size - 1))
    random.setstate(oldstate)
    move = str(chr(ord('a') + (r / size))) + str((r % size) + 1)
    return move

#
# Processor and seed are passed on the command line
#
processor = sys.argv[1]
seed = int(sys.argv[2])

#
# Create empty directory for this seed, and chdir to it
#
path = "/local/scratch/broderic/hex/MoHexExperiment/game-%07d" % seed
shutil.rmtree(path, ignore_errors = True)
os.makedirs(path)
os.chdir(path)

#############################################################################
# Experiment options
#############################################################################

# command prefix and suffix (typically used for distributed computation)
size = 13
command_prefix = 'ssh ' + processor + ' '
command_suffix = ''

optimized_program = '/local/scratch/broderic/hex/benzene-local/benzene/src/mohex/mohex.jun20'
opponent_program = '/local/scratch/broderic/hex/benzene-local/benzene/src/mohex/mohex.jun20'

if "joffre" in processor:
    command_prefix = 'ssh joffre "ssh ' + processor + ' '
    command_suffix = '"'
    optimized_program = '/usr/joffre/broderic/hex/benzene-local/benzene/src/mohex/mohex.jun20'
    opponent_program = '/usr/joffre/broderic/hex/benzene-local/benzene/src/mohex/mohex.jun20'
    
# program to be optimized

# setting a parameter is done with "<gtp_prefix> <parameter_name> <value>"
gtp_prefix = 'param_mohex'

# GTP commands sent to the optimized program before starting
optimized_settings = [
    "param_mohex max_memory 2000000000",
    "param_mohex max_games 999999",
    "param_mohex max_time  999999",
    "param_mohex use_time_management 1",
    "param_game  game_time 180",
    "param_mohex num_threads 4",
    "param_mohex virtual_loss 1",
    "param_mohex reuse_subtree 1"]

# (fixed) opponent program

# GTP commands sent to the opponent program before starting
opponent_settings = [
    "param_mohex max_memory 2000000000",
    "param_mohex max_games 999999",
    "param_mohex max_time  999999",
    "param_mohex use_time_management 1",
    "param_game  game_time 180",
    "param_mohex num_threads 4",
    "param_mohex virtual_loss 1",
    "param_mohex reuse_subtree 1"]

#############################################################################
# You should not have to edit anything beyond this point
#############################################################################

# Add options to optimized programs settings
i = 4
while i < len(sys.argv):
    name = sys.argv[i - 1]
    value = sys.argv[i]
    optimized_settings.append(gtp_prefix + ' ' + name + ' ' + value + '\n')
    i += 2

#
# Protect program names with quotes
#
#optimized_program = '\\"' + optimized_program + '\\"'
#opponent_program = '\\"' + opponent_program + '\\"'

#
# Run one game
#
optcmd = optimized_program + " --seed %SRAND --use-logfile=false"
oppcmd = opponent_program  + " --seed %SRAND --use-logfile=false"
optLogName = path + "/optimized-stderr.log"
oppLogName = path + "/opponent-stderr.log"

verbose = False

bcmd = ''
wcmd = ''
bLogName = ''
wLogName = ''
bsettings = []
wsettings = []
if (seed % 2 == 0): # (black, white) = (optimized, opponent)
    bcmd = optcmd
    bLogName = optLogName
    bsettings = optimized_settings
    wcmd = oppcmd
    wLogName = oppLogName
    wsettings = opponent_settings
else:               # (black, white) = (opponent, optimized)
    bcmd = oppcmd
    bLogName = oppLogName
    bsettings = opponent_settings
    wcmd = optcmd
    wLogName = optLogName
    wsettings = optimized_settings


black = Program("B", command_prefix + bcmd + command_suffix, bLogName, verbose)
white = Program("W", command_prefix + wcmd + command_suffix, wLogName, verbose)

# pass settings to programs
try:
    for opt in bsettings:
        print opt
        black.sendCommand(opt)
    for opt in wsettings:
        white.sendCommand(opt)
except GamePlayer.Error:
    print "Error during initialization!"
    print gamePlayer.getErrorMessage()
except Program.Died:
    print "Error during initialization!"
    print "program died"
   
opening = randomOpening(size, seed / 2)
resultBlack = "?"
resultWhite = "?"
error = 0
errorMessage = ""
game = Game()  # just a temporary
gamePlayer = GamePlayer(black, white, size)
try:
    game = gamePlayer.play(opening, verbose)
    resultBlack = black.getResult()
    resultWhite = white.getResult()
    filename = path + ("/%07d.sgf" % seed)
    gamePlayer.save(filename, seed, resultBlack, resultWhite)

    result = "?"
    if resultBlack == resultWhite:
        result = resultBlack

    if result == '?':
        print "Error: could not determine game result"
    elif 'B+' in result:
        if (seed % 2 == 0):  # seed even ==> black == opt
            print "W"
        else:
            print "L"
    elif 'W+' in result:     
        if (seed % 2 == 0):  # seed even ==> black == opt
            print "L"
        else:
            print "W"

except GamePlayer.Error:
    error = 1
    errorMessage = gamePlayer.getErrorMessage()
    print errorMessage
except Program.Died:
    error = 1
    errorMessage = "program died"
    print errorMessage

