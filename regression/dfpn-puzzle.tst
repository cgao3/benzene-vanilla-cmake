#-----------------------------------------------------------------------------
# Tests dfpn solver with various Hex puzzles.
#
#
# Note 1: This test takes 12h4m to complete (commit 9b9d9f45cd9711) on
#         thorhild (Intel Core 2 Quad CPU Q9450 @ 2.66GHz).
#
# Note 2: Times listed are rounded to the nearest second, and are omitted
#         if the test is trivial (takes less than 0.5 seconds).
#
#-----------------------------------------------------------------------------

# Claude Berge puzzles

dfpn-clear-tt
loadsgf sgf/puzzles/5x5-berge-comp-1.sgf
10 dfpn-solve-state
#? [black]


# David Boll puzzles (kind of - based off positions on his site)

# Takes 921 seconds to solve.
# The losing moves in the mustplay are: i4, c7, h7
dfpn-clear-tt
loadsgf sgf/puzzles/9x9-boll-01.sgf
20 dfpn-solver-find-winning
#? [h5 i5 i6 e7 g7 c8]

# Takes 1 second to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/9x9-boll-02.sgf
21 dfpn-solve-state
#? [white]

# Takes 8844 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/11x11-boll-03.sgf
22 dfpn-solver-find-winning
#? [d5 d8 c9 b10]

# Takes 9444 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/11x11-boll-04.sgf
23 dfpn-solver-find-winning
#? [f7 c8]

# Takes 12566 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/11x11-boll-05.sgf
24 dfpn-solver-find-winning
#? [f7 d8 b9 b10 a11]


# Cameron Browne puzzles

# Takes 67 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/11x11-browne-01.sgf
30 dfpn-solver-find-winning
#? [c4 e4 f4 k5 f6 j7]

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

dfpn-clear-tt
loadsgf sgf/puzzles/7x7-enderton-02.sgf
41 dfpn-solver-find-winning
#? [d6]

dfpn-clear-tt
loadsgf sgf/puzzles/7x7-enderton-03.sgf
42 dfpn-solver-find-winning
#? [e3 e6]

# Takes 75 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-enderton-04.sgf
43 dfpn-solver-find-winning
#? [f2 e3 d4]


# Ryan Hayward puzzles

dfpn-clear-tt
loadsgf sgf/puzzles/6x6-hayward-comp-01.sgf
50 dfpn-solver-find-winning
#? [d3]


# Piet Hein puzzles

dfpn-clear-tt
loadsgf sgf/puzzles/11x11-hein-01.sgf
60 dfpn-solve-state
#? [white]

dfpn-clear-tt
loadsgf sgf/puzzles/3x3-hein-02.sgf
61 dfpn-solve-state
#? [white]

dfpn-clear-tt
loadsgf sgf/puzzles/6x6-hein-03.sgf
62 dfpn-solve-state
#? [white]

dfpn-clear-tt
loadsgf sgf/puzzles/4x4-hein-04.sgf
63 dfpn-solve-state
#? [white]

dfpn-clear-tt
loadsgf sgf/puzzles/4x4-hein-comp-05.sgf
64 dfpn-solve-state
#? [black]

# dfpn-clear-tt
loadsgf sgf/puzzles/5x5-hein-comp-06.sgf
65 dfpn-solver-find-winning
#? [d3]

dfpn-clear-tt
loadsgf sgf/puzzles/6x6-hein-07.sgf
66 dfpn-solve-state
#? [white]


# Philip Henderson puzzles

# Takes 6764 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/9x9-henderson-01.sgf
70 dfpn-solver-find-winning
#? [c4 b5 g5]

dfpn-clear-tt
loadsgf sgf/puzzles/6x6-henderson-02.sgf
71 dfpn-solver-find-winning
#? [b4]

# Takes 305 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/11x11-henderson-03.sgf
72 dfpn-solve-state
#? [black]

dfpn-clear-tt
loadsgf sgf/puzzles/6x6-henderson-04.sgf
73 dfpn-solver-find-winning
#? [c4]

# Takes 5 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-05.sgf
74 dfpn-solver-find-winning
#? [b6]

# Takes 76 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-06.sgf
75 dfpn-solver-find-winning
#? [f4]

# Takes 143 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-07.sgf
76 dfpn-solver-find-winning
#? [g2 c4]

# Takes 71 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-08.sgf
77 dfpn-solver-find-winning
#? [e3]

# Takes 1 second to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/6x6-henderson-09.sgf
78 dfpn-solver-find-winning
#? [d4]

dfpn-clear-tt
loadsgf sgf/puzzles/6x6-henderson-10.sgf
79 dfpn-solver-find-winning
#? [c4]

dfpn-clear-tt
loadsgf sgf/puzzles/5x5-henderson-11.sgf
80 dfpn-solver-find-winning
#? [c4]

dfpn-clear-tt
loadsgf sgf/puzzles/5x5-henderson-12.sgf
81 dfpn-solver-find-winning
#? [b3]

# Takes 1 second to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/6x6-henderson-13.sgf
82 dfpn-solver-find-winning
#? [b6]

dfpn-clear-tt
loadsgf sgf/puzzles/6x6-henderson-14.sgf
83 dfpn-solver-find-winning
#? [e2]

dfpn-clear-tt
loadsgf sgf/puzzles/6x6-henderson-15.sgf
84 dfpn-solver-find-winning
#? [b3 d5]

dfpn-clear-tt
loadsgf sgf/puzzles/6x6-henderson-16.sgf
85 dfpn-solver-find-winning
#? [e1 b5]

dfpn-clear-tt
loadsgf sgf/puzzles/6x6-henderson-17.sgf
86 dfpn-solver-find-winning
#? [c4]

dfpn-clear-tt
loadsgf sgf/puzzles/6x6-henderson-18.sgf
87 dfpn-solver-find-winning
#? [f4]

dfpn-clear-tt
loadsgf sgf/puzzles/6x6-henderson-19.sgf
88 dfpn-solver-find-winning
#? [c4]

# Takes 1 second to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/6x6-henderson-20.sgf
89 dfpn-solver-find-winning
#? [d3]

dfpn-clear-tt
loadsgf sgf/puzzles/6x6-henderson-21.sgf
90 dfpn-solver-find-winning
#? [b3 c3]

dfpn-clear-tt
loadsgf sgf/puzzles/6x6-henderson-22.sgf
91 dfpn-solver-find-winning
#? [c4]

# Takes 1 second to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-23.sgf
92 dfpn-solver-find-winning
#? []

# Takes 1 second to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-24.sgf
93 dfpn-solver-find-winning
#? [d5]

# Takes 4 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-25.sgf
94 dfpn-solver-find-winning
#? [f2 f5]

# Takes 12 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-26.sgf
95 dfpn-solver-find-winning
#? [d5]

# Takes 3 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-27.sgf
96 dfpn-solver-find-winning
#? []

# Takes 5 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-28.sgf
97 dfpn-solver-find-winning
#? [d5]

# Takes 2 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-29.sgf
98 dfpn-solver-find-winning
#? [c5]

# Takes 46 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-30.sgf
99 dfpn-solver-find-winning
#? [d3]

# Takes 11 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-31.sgf
100 dfpn-solver-find-winning
#? [d3]

# Takes 10 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-32.sgf
101 dfpn-solver-find-winning
#? [c5]

# Takes 89 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-33.sgf
102 dfpn-solver-find-winning
#? [e4]

dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-34.sgf
103 dfpn-solver-find-winning
#? [f2 f5]

# Takes 161 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-35.sgf
104 dfpn-solver-find-winning
#? [b2 b4]

# Takes 3 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-36.sgf
105 dfpn-solver-find-winning
#? [b5]

# Takes 3 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-37.sgf
106 dfpn-solver-find-winning
#? [b4]

# Takes 15 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-38.sgf
107 dfpn-solver-find-winning
#? [b3 b6]

# Takes 17 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-39.sgf
108 dfpn-solver-find-winning
#? [d4]

# Takes 1 second to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-40.sgf
109 dfpn-solver-find-winning
#? [e3]

# Takes 15 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-41.sgf
110 dfpn-solver-find-winning
#? [d5]

# Takes 33 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-42.sgf
111 dfpn-solver-find-winning
#? [d4]

# Takes 45 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-43.sgf
112 dfpn-solver-find-winning
#? [c6]

# Takes 2 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-44.sgf
113 dfpn-solver-find-winning
#? [b7]

# Takes 1 second to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-45.sgf
114 dfpn-solver-find-winning
#? [c2 f2]

# Takes 5 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-46.sgf
115 dfpn-solver-find-winning
#? [d4]

# Takes 16 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-47.sgf
116 dfpn-solver-find-winning
#? [f3 b4]

dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-48.sgf
117 dfpn-solver-find-winning
#? [f2 c5]

# Takes 1 second to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-49.sgf
118 dfpn-solver-find-winning
#? [e3]

dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-50.sgf
119 dfpn-solver-find-winning
#? [d5]

# Takes 28 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-51.sgf
120 dfpn-solver-find-winning
#? [c5]

# Takes 3 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-52.sgf
121 dfpn-solver-find-winning
#? [e4]

# Takes 1 second to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-53.sgf
122 dfpn-solver-find-winning
#? [c5]

# Takes 11 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-54.sgf
123 dfpn-solver-find-winning
#? [c5]

# Takes 5 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-55.sgf
124 dfpn-solver-find-winning
#? [e4]

# Takes 890 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/8x8-henderson-56.sgf
125 dfpn-solver-find-winning
#? [e6]

# Takes 44 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-57.sgf
126 dfpn-solver-find-winning
#? [c4]

# Takes 13 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-58.sgf
127 dfpn-solver-find-winning
#? [c2]

# Takes 5 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-59.sgf
128 dfpn-solver-find-winning
#? [b4]

dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-60.sgf
129 dfpn-solver-find-winning
#? [c7]

# Takes 2 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-61.sgf
130 dfpn-solver-find-winning
#? [e4]

dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-62.sgf
131 dfpn-solver-find-winning
#? [f3]


# Little Golem puzzles

# Takes 3 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-LG-01.sgf
150 dfpn-solver-find-winning
#? [b6]

# Takes 1579 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/10x10-LG-01.sgf
151 dfpn-solve-state
#? [white]
# Takes ??? to solve.
# dfpn-clear-tt
# 151 dfpn-solver-find-winning
# #? [c4 c7]

# Takes 1141 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/10x10-LG-01sol.sgf
152 dfpn-solve-state
#? [white]
