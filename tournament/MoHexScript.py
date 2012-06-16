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
#command_prefix = 'ssh ' + processor + ' cd ' + path + ';'
command_prefix = ''
command_suffix = ''

size = 13

# program to be optimized
optimized_program = '/local/scratch/broderic/hex/benzene-local/benzene/src/mohex/mohex.jun15'

# setting a parameter is done with "<gtp_prefix> <parameter_name> <value>"
gtp_prefix = 'param_mohex'

# GTP commands sent to the optimized program before starting
optimized_settings = ''
optimized_settings += """
param_mohex max_memory 2000000000
param_mohex max_games 999999
param_mohex max_time  999999
param_mohex use_time_management 1
param_game  game_time 60
param_mohex num_threads 4
param_mohex virtual_loss 1
param_mohex reuse_subtree 1
"""
# (fixed) opponent program
opponent_program = '/local/scratch/broderic/hex/benzene-local/benzene/src/mohex/mohex.jun15'

# GTP commands sent to the opponent program before starting
opponent_settings = ''
opponent_settings += """
param_mohex max_memory 2000000000
param_mohex max_games 999999
param_mohex max_time  999999
param_mohex use_time_management 1
param_game  game_time 60
param_mohex num_threads 4
param_mohex virtual_loss 1
param_mohex reuse_subtree 1
"""

#############################################################################
# You should not have to edit anything beyond this point
#############################################################################

#
# Create initialization file for the program to be optimized
#
gtp_file = open('optimized_settings.gtp', 'w')
gtp_file.write(optimized_settings)

i = 4
params = []
while i < len(sys.argv):
    name = sys.argv[i - 1]
    value = sys.argv[i]
    gtp_file.write(gtp_prefix + ' ' + name + ' ' + value + '\n')
    i += 2

gtp_file.close()

#
# Create initialization file for the opponent
#
open('opponent_settings.gtp', 'w').write(opponent_settings)

#
# Protect program names with quotes
#
#optimized_program = '\\"' + optimized_program + '\\"'
#opponent_program = '\\"' + opponent_program + '\\"'

#
# Run one game
#

optcmd = "nice " + optimized_program + " --seed %SRAND" \
    + " --config " + path + "/optimized_settings.gtp" \
    + " --logfile-name " + path + "/optimized.log"    
    
oppcmd = "nice " + opponent_program  + " --seed %SRAND" \
    + " --config " + path + "/opponent_settings.gtp" \
    + " --logfile-name " + path + "/opponent.log"

optLogName = path + "/optimized-stderr.log"
oppLogName = path + "/opponent-stderr.log"

verbose = False

bcmd = ''
wcmd = ''
bLogName = ''
wLogName = ''
if (seed % 2 == 0): # (black, white) = (optimized, opponent)
    bcmd = optcmd
    bLogName = optLogName
    wcmd = oppcmd
    wLogName = oppLogName
else:               # (black, white) = (opponent, optimized)
    bcmd = oppcmd
    bLogName = oppLogName
    wcmd = optcmd
    wLogName = optLogName

black = Program("B", bcmd, bLogName, verbose)
white = Program("W", wcmd, wLogName, verbose)
    
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
    gamePlayer.save(path + "/game.sgf", seed, resultBlack, resultWhite)

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

