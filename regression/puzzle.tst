#-----------------------------------------------------------------------------
# Tests dfs solver engine with various Hex puzzles.
#
#
# Note 1: This test takes 14h48m to complete (commit 97876949abf) on
#         thorhild (Intel Core 2 Quad CPU Q9450 @ 2.66GHz).
#
# Note 2: Times listed are rounded to the nearest given unit, and are omitted
#         if the test is trivial (takes less than 1 second).
#
#-----------------------------------------------------------------------------

# Claude Berge puzzles

solver-clear-tt
loadsgf sgf/puzzles/5x5-berge-1.sgf
10 solve-state white
#? [white]


# David Boll puzzles (kind of - based off positions on his site)

# Takes 46 minutes to solve.
# The losing moves in the mustplay are: i4, c7, h7
solver-clear-tt
loadsgf sgf/puzzles/9x9-boll-01.sgf
20 solver-find-winning black
#? [h5 i5 i6 e7 g7 c8]

# Takes 41 minutes to solve. (shows DFS weaknesses)
solver-clear-tt
loadsgf sgf/puzzles/9x9-boll-02.sgf
21 solve-state white
#? [white]

# Takes 2h25 minutes to solve.
solver-clear-tt
loadsgf sgf/puzzles/11x11-boll-03.sgf
22 solver-find-winning white
#? [d5 d8 c9 b10]

# Takes 57 minutes to solve.
solver-clear-tt
loadsgf sgf/puzzles/11x11-boll-04.sgf
23 solver-find-winning black
#? [f7 c8]

# Takes 2h17m to solve.
solver-clear-tt
loadsgf sgf/puzzles/11x11-boll-05.sgf
24 solver-find-winning black
#? [f7 d8 b9 b10 a11]


# Cameron Browne puzzles

solver-clear-tt
loadsgf sgf/puzzles/11x11-browne-01.sgf
30 solver-find-winning black
#? [c4 e4 f4 k5 f6 j7]

solver-clear-tt
loadsgf sgf/puzzles/11x11-browne-02.sgf
31 solver-find-winning white
#? [c5]

solver-clear-tt
loadsgf sgf/puzzles/11x11-browne-03.sgf
32 solve-state white
#? [white]


# Bert Enderton puzzles

solver-clear-tt
loadsgf sgf/puzzles/7x7-enderton-01.sgf
40 solver-find-winning white
#? [d5]

solver-clear-tt
loadsgf sgf/puzzles/7x7-enderton-02.sgf
41 solver-find-winning white
#? [d6]

solver-clear-tt
loadsgf sgf/puzzles/7x7-enderton-03.sgf
42 solver-find-winning white
#? [e3 e6]

# Takes 53 seconds to solve.
solver-clear-tt
loadsgf sgf/puzzles/7x7-enderton-04.sgf
43 solver-find-winning black
#? [f2 e3 d4]


# Ryan Hayward puzzles

solver-clear-tt
loadsgf sgf/puzzles/6x6-hayward-01.sgf
50 solver-find-winning white
#? [d3]


# Piet Hein puzzles

solver-clear-tt
loadsgf sgf/puzzles/11x11-hein-01.sgf
60 solve-state white
#? [white]

solver-clear-tt
loadsgf sgf/puzzles/3x3-hein-02.sgf
61 solve-state white
#? [white]

solver-clear-tt
loadsgf sgf/puzzles/6x6-hein-03.sgf
62 solve-state white
#? [white]

solver-clear-tt
loadsgf sgf/puzzles/4x4-hein-04.sgf
63 solve-state white
#? [white]

solver-clear-tt
loadsgf sgf/puzzles/4x4-hein-05.sgf
64 solve-state white
#? [white]

solver-clear-tt
loadsgf sgf/puzzles/5x5-hein-06.sgf
65 solver-find-winning white
#? [c2]

solver-clear-tt
loadsgf sgf/puzzles/6x6-hein-07.sgf
66 solve-state white
#? [white]


# Philip Henderson puzzles

# Takes 5h26m to solve.
solver-clear-tt
loadsgf sgf/puzzles/9x9-henderson-01.sgf
70 solver-find-winning black
#? [c4 b5 g5]

solver-clear-tt
loadsgf sgf/puzzles/6x6-henderson-02.sgf
71 solver-find-winning black
#? [b4]

# Takes 18 minutes to solve.
solver-clear-tt
loadsgf sgf/puzzles/11x11-henderson-03.sgf
72 solve-state black
#? [black]

solver-clear-tt
loadsgf sgf/puzzles/6x6-henderson-04.sgf
73 solver-find-winning black
#? [c4]

solver-clear-tt
loadsgf sgf/puzzles/7x7-henderson-05.sgf
74 solver-find-winning white
#? [b6]

solver-clear-tt
loadsgf sgf/puzzles/7x7-henderson-06.sgf
75 solver-find-winning black
#? [f4]

solver-clear-tt
loadsgf sgf/puzzles/7x7-henderson-07.sgf
76 solver-find-winning black
#? [g2 c4]

solver-clear-tt
loadsgf sgf/puzzles/7x7-henderson-08.sgf
77 solver-find-winning white
#? [e3]

solver-clear-tt
loadsgf sgf/puzzles/6x6-henderson-09.sgf
78 solver-find-winning black
#? [d4]

solver-clear-tt
loadsgf sgf/puzzles/6x6-henderson-10.sgf
79 solver-find-winning black
#? [c4]

solver-clear-tt
loadsgf sgf/puzzles/5x5-henderson-11.sgf
80 solver-find-winning white
#? [c4]

solver-clear-tt
loadsgf sgf/puzzles/5x5-henderson-12.sgf
81 solver-find-winning black
#? [b3]


# Little Golem puzzles

# Takes 2 seconds to solve.
solver-clear-tt
loadsgf sgf/puzzles/7x7-LG-01.sgf
90 solver-find-winning white
#? [b6]

# Takes 2h12m to solve.
solver-clear-tt
loadsgf sgf/puzzles/10x10-LG-01.sgf
91 solve-state white
#? [white]
# Takes ??? to solve.
# solver-clear-tt
# 91 solver-find-winning white
# #? [c4 c7]

# Takes 2m18s to solve.
solver-clear-tt
loadsgf sgf/puzzles/10x10-LG-01sol.sgf
92 solve-state black
#? [white]
