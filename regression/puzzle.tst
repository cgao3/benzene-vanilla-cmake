#-----------------------------------------------------------------------------
# Tests solver engine with various Hex puzzles.
#
#
# Note 1: solver-find-winning will crash on many of the tests below
#         because of MovesToConsider asserts. This needs to be fixed, and
#         the solutions below should be changed to winning moves rather
#         than simply winning colour.
#
# Note 2: Later on we should have tests for speed as well, not just
#         correctness.
#
# Note 3: As of Mar 31, 2009 this test takes ~10.75 hours to complete.
# 
#-----------------------------------------------------------------------------

# Claude Berge puzzles

loadsgf sgf/puzzles/5x5-berge-1.sgf
10 solve-state white
#? [white]

loadsgf sgf/puzzles/5x5-berge-2.sgf
11 solver-find-winning white
#? [c2]


# David Boll puzzles (kind of - based off positions on his site)

# This puzzle takes ~30 minutes to solve!!
# The losing moves in the mustplay are: i4, c7, h7
loadsgf sgf/puzzles/9x9-boll-01.sgf
20 solver-find-winning black
#? [h5 i5 i6 e7 g7 c8]

# Takes ~20 minutes despite being relatively straight-forward.
loadsgf sgf/puzzles/9x9-boll-02.sgf
21 solve-state white
#? [white]

# Takes somewhere in the range of 1-3 hours to solve
loadsgf sgf/puzzles/11x11-boll-03.sgf
22 solver-find-winning white
#? [d5 d8 c9 b10]

# Takes ~1-2 hours to solve.
loadsgf sgf/puzzles/11x11-boll-04.sgf
23 solver-find-winning black
#? [f7 c8]

# Takes ~2 hours to solve.
loadsgf sgf/puzzles/11x11-boll-05.sgf
24 solver-find-winning black
#? [f7 d8 b9 b10 a11]


# Cameron Browne puzzles

loadsgf sgf/puzzles/11x11-browne-01.sgf
30 solve-state black
#? [black]

loadsgf sgf/puzzles/11x11-browne-02.sgf
31 solve-state white
#? [white]

loadsgf sgf/puzzles/11x11-browne-03.sgf
32 solve-state white
#? [white]


# Bert Enderton puzzles

loadsgf sgf/puzzles/7x7-enderton-01.sgf
40 solver-find-winning white
#? [d5]

loadsgf sgf/puzzles/7x7-enderton-02.sgf
41 solver-find-winning white
#? [d6]

loadsgf sgf/puzzles/7x7-enderton-03.sgf
42 solver-find-winning white
#? [e3 e6]

loadsgf sgf/puzzles/7x7-enderton-04.sgf
43 solver-find-winning black
#? [f2 e3 d4]


# Ryan Hayward puzzles

loadsgf sgf/puzzles/6x6-hayward-01.sgf
50 solver-find-winning white
#? [d3]


# Piet Hein puzzles

loadsgf sgf/puzzles/11x11-hein-01.sgf
70 solve-state white
#? [white]

loadsgf sgf/puzzles/3x3-hein-02.sgf
71 solve-state white
#? [white]

loadsgf sgf/puzzles/6x6-hein-03.sgf
72 solve-state white
#? [white]

loadsgf sgf/puzzles/4x4-hein-04.sgf
73 solve-state white
#? [white]

loadsgf sgf/puzzles/4x4-hein-05.sgf
74 solve-state white
#? [white]

loadsgf sgf/puzzles/5x5-hein-06.sgf
75 solver-find-winning white
#? [c2]

loadsgf sgf/puzzles/6x6-hein-07.sgf
76 solve-state white
#? [white]


# Philip Henderson puzzles

# Takes ~2 hours to solve this
loadsgf sgf/puzzles/9x9-henderson-01.sgf
80 solver-find-winning black
#? [c4 b5 g5]

loadsgf sgf/puzzles/6x6-henderson-02.sgf
81 solver-find-winning black
#? [b4]


# Little Golem puzzles

loadsgf sgf/puzzles/7x7-LG-01.sgf
90 solver-find-winning white
#? [b6]

# Takes ~2-3 hours to solve
loadsgf sgf/puzzles/10x10-LG-01.sgf
91 solve-state white
#? [white]
# Takes a long time to find all solutions (somewhere in 4-20 hour range)
# 91 solver-find-winning white
# #? [c4 c7]

loadsgf sgf/puzzles/10x10-LG-01sol.sgf
91 solve-state black
#? [white]
