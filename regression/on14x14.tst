#-----------------------------------------------------------------------------
# Tests Hex engine/solver on boards larger than 11x11.
#
# It is absolutely vital that benzene is compiled with 14x14 support
# in order to run these tests (see the README file for details).
#
#-----------------------------------------------------------------------------

# Edge templates

loadsgf sgf/largeBoards/14x14-row6Template.sgf
11 solve-state w
#? [black]


# Endgame positions

loadsgf sgf/largeBoards/13x13-game01.sgf
20 solve-state black
#? [black]

loadsgf sgf/largeBoards/13x13-game02.sgf
21 solve-state white
#? [white]


# Berge Puzzles

loadsgf sgf/largeBoards/14x14-berge-3.sgf
30 solve-state black
#? [black]

loadsgf sgf/largeBoards/14x14-berge-4.sgf
31 solve-state black
#? [black]

loadsgf sgf/largeBoards/14x14-berge-5.sgf
32 solve-state black
#? [black]

# Personal addition: solving the previous puzzle for the opposite player
loadsgf sgf/largeBoards/14x14-berge-5.sgf
33 solve-state white
#? [white]
