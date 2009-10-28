#-----------------------------------------------------------------------------
# Tests dfpn solver with various Hex puzzles.
#
#
# Note 1: As of Oct 28, 2009 this test takes 18h43m to complete.
#
# Note 2: Times listed are rounded to the nearest given unit, and are omitted
#         if the test is trivial (takes less than 1 second).
#
#-----------------------------------------------------------------------------

# Claude Berge puzzles

dfpn-clear-tt
loadsgf sgf/puzzles/5x5-berge-comp-1.sgf
10 dfpn-solve-state
#? [black]


# David Boll puzzles (kind of - based off positions on his site)

# Takes 24 minutes to solve.
# The losing moves in the mustplay are: i4, c7, h7
dfpn-clear-tt
loadsgf sgf/puzzles/9x9-boll-01.sgf
20 dfpn-solver-find-winning
#? [h5 i5 i6 e7 g7 c8]

# Takes 2 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/9x9-boll-02.sgf
21 dfpn-solve-state
#? [white]

# Takes 3h56m to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/11x11-boll-03.sgf
22 dfpn-solver-find-winning
#? [d5 d8 c9 b10]

# Takes 3h19m to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/11x11-boll-04.sgf
23 dfpn-solver-find-winning
#? [f7 c8]

# Takes 5h50m to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/11x11-boll-05.sgf
24 dfpn-solver-find-winning
#? [f7 d8 b9 b10 a11]


# Cameron Browne puzzles

# Takes 5 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/11x11-browne-01.sgf
30 dfpn-solve-state
#? [black]

dfpn-clear-tt
loadsgf sgf/puzzles/11x11-browne-comp-02.sgf
31 dfpn-solver-find-winning
#? [e3]

dfpn-clear-tt
loadsgf sgf/puzzles/11x11-browne-03.sgf
32 dfpn-solve-state
#? [white]


# Bert Enderton puzzles

dfpn-clear-tt
loadsgf sgf/puzzles/7x7-enderton-01.sgf
40 dfpn-solver-find-winning
#? [d5]

# Takes 1 second to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-enderton-02.sgf
41 dfpn-solver-find-winning
#? [d6]

dfpn-clear-tt
loadsgf sgf/puzzles/7x7-enderton-03.sgf
42 dfpn-solver-find-winning
#? [e3 e6]

# Takes 4m40s to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-enderton-04.sgf
43 dfpn-solver-find-winning
#? [f2 e3 d4]


# Ryan Hayward puzzles

# Takes 1 second to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/6x6-hayward-comp-01.sgf
50 dfpn-solver-find-winning
#? [d3]


# Piet Hein puzzles

dfpn-clear-tt
loadsgf sgf/puzzles/11x11-hein-01.sgf
70 dfpn-solve-state
#? [white]

dfpn-clear-tt
loadsgf sgf/puzzles/3x3-hein-02.sgf
71 dfpn-solve-state
#? [white]

dfpn-clear-tt
loadsgf sgf/puzzles/6x6-hein-03.sgf
72 dfpn-solve-state
#? [white]

dfpn-clear-tt
loadsgf sgf/puzzles/4x4-hein-04.sgf
73 dfpn-solve-state
#? [white]

dfpn-clear-tt
loadsgf sgf/puzzles/4x4-hein-comp-05.sgf
74 dfpn-solve-state
#? [black]

# dfpn-clear-tt
loadsgf sgf/puzzles/5x5-hein-comp-06.sgf
75 dfpn-solver-find-winning
#? [d3]

dfpn-clear-tt
loadsgf sgf/puzzles/6x6-hein-07.sgf
76 dfpn-solve-state
#? [white]


# Philip Henderson puzzles

# Takes 3h34m to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/9x9-henderson-01.sgf
80 dfpn-solver-find-winning
#? [c4 b5 g5]

# Takes 1 second to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/6x6-henderson-02.sgf
81 dfpn-solver-find-winning
#? [b4]


# Little Golem puzzles

# Takes 9 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-LG-01.sgf
90 dfpn-solver-find-winning
#? [b6]

# Takes 56 minutes to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/10x10-LG-01.sgf
91 dfpn-solve-state
#? [white]
# Takes ??? to solve.
# dfpn-clear-tt
# 91 dfpn-solver-find-winning
# #? [c4 c7]

# Takes 41 minutes to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/10x10-LG-01sol.sgf
92 dfpn-solve-state
#? [white]
