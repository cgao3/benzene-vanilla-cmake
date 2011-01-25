#-----------------------------------------------------------------------------
# Test some functions of the HTP interface. 
#
# This is basically ensuring the code in the HTP wrapper 
# doesn't crash for each basic command. 
#-----------------------------------------------------------------------------

10 name
#? [.*]

20 version
#? [.*]

30 boardsize 1 1
#? []

40 boardsize 2 2
#? []

50 boardsize 11 11
#? []

60 showboard
#? [.*]

70 play b a1
#? []

80 play w a2
#? []

90 undo
#? []

100 undo
#? []

110 compute-inferior b
#? [.*]

120 compute-inferior w
#? [.*]

130 eval-resist b
#? [.*]

140 eval-twod b
#? [.*]

