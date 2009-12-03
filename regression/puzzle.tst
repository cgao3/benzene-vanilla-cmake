#-----------------------------------------------------------------------------
# Tests dfs solver engine with various Hex puzzles.
#
#
# Note 1: This test takes 9h7m to complete (commit 7bdf42667840c6f) on
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

# Takes 19 minutes to solve.
# The losing moves in the mustplay are: i4, c7, h7
solver-clear-tt
loadsgf sgf/puzzles/9x9-boll-01.sgf
20 solver-find-winning black
#? [h5 i5 i6 e7 g7 c8]

# Takes 39 minutes to solve (shows DFS weaknesses).
solver-clear-tt
loadsgf sgf/puzzles/9x9-boll-02.sgf
21 solve-state white
#? [white]

# Takes 1h48 minutes to solve.
solver-clear-tt
loadsgf sgf/puzzles/11x11-boll-03.sgf
22 solver-find-winning white
#? [d5 d8 c9 b10]

# Takes 49 minutes to solve.
solver-clear-tt
loadsgf sgf/puzzles/11x11-boll-04.sgf
23 solver-find-winning black
#? [f7 c8]

# Takes 1h41m to solve.
solver-clear-tt
loadsgf sgf/puzzles/11x11-boll-05.sgf
24 solver-find-winning black
#? [f7 d8 b9 b10 a11]


# Cameron Browne puzzles

# Takes 23 seconds to solve.
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

# Takes 50 seconds to solve.
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

# Takes 2h49m to solve.
solver-clear-tt
loadsgf sgf/puzzles/9x9-henderson-01.sgf
70 solver-find-winning black
#? [c4 b5 g5]

solver-clear-tt
loadsgf sgf/puzzles/6x6-henderson-02.sgf
71 solver-find-winning black
#? [b4]

# Takes 6 minutes to solve.
solver-clear-tt
loadsgf sgf/puzzles/11x11-henderson-03.sgf
72 solve-state black
#? [black]

solver-clear-tt
loadsgf sgf/puzzles/6x6-henderson-04.sgf
73 solver-find-winning black
#? [c4]

# Takes 4 seconds to solve.
solver-clear-tt
loadsgf sgf/puzzles/7x7-henderson-05.sgf
74 solver-find-winning white
#? [b6]

# Takes 44 seconds to solve.
solver-clear-tt
loadsgf sgf/puzzles/7x7-henderson-06.sgf
75 solver-find-winning black
#? [f4]

# Takes 38 seconds to solve.
solver-clear-tt
loadsgf sgf/puzzles/7x7-henderson-07.sgf
76 solver-find-winning black
#? [g2 c4]

# Takes 48 seconds to solve.
solver-clear-tt
loadsgf sgf/puzzles/7x7-henderson-08.sgf
77 solver-find-winning white
#? [e3]

# Takes 1 second to solve.
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

# Takes 1 second to solve.
solver-clear-tt
loadsgf sgf/puzzles/6x6-henderson-13.sgf
82 solver-find-winning white
#? [b6]

solver-clear-tt
loadsgf sgf/puzzles/6x6-henderson-14.sgf
83 solver-find-winning white
#? [e2]

solver-clear-tt
loadsgf sgf/puzzles/6x6-henderson-15.sgf
84 solver-find-winning black
#? [b3 d5]

solver-clear-tt
loadsgf sgf/puzzles/6x6-henderson-16.sgf
85 solver-find-winning white
#? [e1 b5]

solver-clear-tt
loadsgf sgf/puzzles/6x6-henderson-17.sgf
86 solver-find-winning white
#? [c4]

solver-clear-tt
loadsgf sgf/puzzles/6x6-henderson-18.sgf
87 solver-find-winning black
#? [f4]

solver-clear-tt
loadsgf sgf/puzzles/6x6-henderson-19.sgf
88 solver-find-winning black
#? [c4]

solver-clear-tt
loadsgf sgf/puzzles/6x6-henderson-20.sgf
89 solver-find-winning white
#? [d3]

solver-clear-tt
loadsgf sgf/puzzles/6x6-henderson-21.sgf
90 solver-find-winning black
#? [b3 c3]

solver-clear-tt
loadsgf sgf/puzzles/6x6-henderson-22.sgf
91 solver-find-winning black
#? [c4]

solver-clear-tt
loadsgf sgf/puzzles/7x7-henderson-23.sgf
92 solver-find-winning white
#? []

solver-clear-tt
loadsgf sgf/puzzles/7x7-henderson-24.sgf
93 solver-find-winning white
#? [d5]

# Takes 4 seconds to solve.
solver-clear-tt
loadsgf sgf/puzzles/7x7-henderson-25.sgf
94 solver-find-winning black
#? [f2 f5]

# Takes 8 seconds to solve.
solver-clear-tt
loadsgf sgf/puzzles/7x7-henderson-26.sgf
95 solver-find-winning black
#? [d5]

# Takes 2 seconds to solve.
solver-clear-tt
loadsgf sgf/puzzles/7x7-henderson-27.sgf
96 solver-find-winning black
#? []

# Takes 4 seconds to solve.
solver-clear-tt
loadsgf sgf/puzzles/7x7-henderson-28.sgf
97 solver-find-winning black
#? [d5]

# Takes 2 seconds to solve.
solver-clear-tt
loadsgf sgf/puzzles/7x7-henderson-29.sgf
98 solver-find-winning white
#? [c5]

# Takes 33 seconds to solve.
solver-clear-tt
loadsgf sgf/puzzles/7x7-henderson-30.sgf
99 solver-find-winning white
#? [d3]

# Takes 8 seconds to solve.
solver-clear-tt
loadsgf sgf/puzzles/7x7-henderson-31.sgf
100 solver-find-winning white
#? [d3]

# Takes 10 seconds to solve.
solver-clear-tt
loadsgf sgf/puzzles/7x7-henderson-32.sgf
101 solver-find-winning black
#? [c5]

# Takes 61 seconds to solve.
solver-clear-tt
loadsgf sgf/puzzles/7x7-henderson-33.sgf
102 solver-find-winning black
#? [e4]

solver-clear-tt
loadsgf sgf/puzzles/7x7-henderson-34.sgf
103 solver-find-winning black
#? [f2 f5]

# Takes 43 seconds to solve.
solver-clear-tt
loadsgf sgf/puzzles/7x7-henderson-35.sgf
104 solver-find-winning white
#? [b2 b4]

# Takes 2 seconds to solve.
solver-clear-tt
loadsgf sgf/puzzles/7x7-henderson-36.sgf
105 solver-find-winning white
#? [b5]

# Takes 2 seconds to solve.
solver-clear-tt
loadsgf sgf/puzzles/7x7-henderson-37.sgf
106 solver-find-winning black
#? [b4]

# Takes 14 seconds to solve.
solver-clear-tt
loadsgf sgf/puzzles/7x7-henderson-38.sgf
107 solver-find-winning black
#? [b3 b6]

# Takes 13 seconds to solve.
solver-clear-tt
loadsgf sgf/puzzles/7x7-henderson-39.sgf
108 solver-find-winning black
#? [d4]

# Takes 1 second to solve.
solver-clear-tt
loadsgf sgf/puzzles/7x7-henderson-40.sgf
109 solver-find-winning white
#? [e3]

# Takes 13 seconds to solve.
solver-clear-tt
loadsgf sgf/puzzles/7x7-henderson-41.sgf
110 solver-find-winning white
#? [d5]

# Takes 25 seconds to solve.
solver-clear-tt
loadsgf sgf/puzzles/7x7-henderson-42.sgf
111 solver-find-winning white
#? [d4]

# Takes 36 seconds to solve.
solver-clear-tt
loadsgf sgf/puzzles/7x7-henderson-43.sgf
112 solver-find-winning white
#? [c6]

# Takes 2 seconds to solve.
solver-clear-tt
loadsgf sgf/puzzles/7x7-henderson-44.sgf
113 solver-find-winning black
#? [b7]

# Takes 1 second to solve.
solver-clear-tt
loadsgf sgf/puzzles/7x7-henderson-45.sgf
114 solver-find-winning white
#? [c2 f2]

# Takes 4 seconds to solve.
solver-clear-tt
loadsgf sgf/puzzles/7x7-henderson-46.sgf
115 solver-find-winning black
#? [d4]

# Takes 8 seconds to solve.
solver-clear-tt
loadsgf sgf/puzzles/7x7-henderson-47.sgf
116 solver-find-winning black
#? [f3 b4]

solver-clear-tt
loadsgf sgf/puzzles/7x7-henderson-48.sgf
117 solver-find-winning black
#? [f2 c5]

# Takes 1 second to solve.
solver-clear-tt
loadsgf sgf/puzzles/7x7-henderson-49.sgf
118 solver-find-winning white
#? [e3]

solver-clear-tt
loadsgf sgf/puzzles/7x7-henderson-50.sgf
119 solver-find-winning white
#? [d5]

# Takes 16 seconds to solve.
solver-clear-tt
loadsgf sgf/puzzles/7x7-henderson-51.sgf
120 solver-find-winning black
#? [c5]

# Takes 2 seconds to solve.
solver-clear-tt
loadsgf sgf/puzzles/7x7-henderson-52.sgf
121 solver-find-winning black
#? [e4]

solver-clear-tt
loadsgf sgf/puzzles/7x7-henderson-53.sgf
122 solver-find-winning white
#? [c5]

# Takes 8 seconds to solve.
solver-clear-tt
loadsgf sgf/puzzles/7x7-henderson-54.sgf
123 solver-find-winning white
#? [c5]

# Takes 5 seconds to solve.
solver-clear-tt
loadsgf sgf/puzzles/7x7-henderson-55.sgf
124 solver-find-winning black
#? [e4]

# Takes 951 seconds to solve.
solver-clear-tt
loadsgf sgf/puzzles/8x8-henderson-56.sgf
125 solver-find-winning white
#? [e6]

solver-clear-tt
loadsgf sgf/puzzles/7x7-henderson-57.sgf
126 solver-find-winning black
#? [c4]

solver-clear-tt
loadsgf sgf/puzzles/7x7-henderson-58.sgf
127 solver-find-winning white
#? [c2]

solver-clear-tt
loadsgf sgf/puzzles/7x7-henderson-59.sgf
128 solver-find-winning white
#? [b4]

solver-clear-tt
loadsgf sgf/puzzles/7x7-henderson-60.sgf
129 solver-find-winning white
#? [c7]

solver-clear-tt
loadsgf sgf/puzzles/7x7-henderson-61.sgf
130 solver-find-winning white
#? [e4]


# Little Golem puzzles

# Takes 2 seconds to solve.
solver-clear-tt
loadsgf sgf/puzzles/7x7-LG-01.sgf
150 solver-find-winning white
#? [b6]

# Takes 29m5s to solve.
solver-clear-tt
loadsgf sgf/puzzles/10x10-LG-01.sgf
151 solve-state white
#? [white]
# Takes ??? to solve.
# solver-clear-tt
# 151 solver-find-winning white
# #? [c4 c7]

# Takes 2m5s to solve.
solver-clear-tt
loadsgf sgf/puzzles/10x10-LG-01sol.sgf
152 solve-state black
#? [white]
