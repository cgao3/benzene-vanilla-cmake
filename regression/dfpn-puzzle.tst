#-----------------------------------------------------------------------------
# Tests dfpn solver with various Hex puzzles.
#
#
# Note 1: This test takes 7h32m to complete (commit 738eedf707706ffb78) on
#         thorhild (Intel Core 2 Quad CPU Q9450 @ 2.66GHz).
#
# Note 2: Times listed are rounded to the nearest second, and are omitted
#         if the test is trivial (takes less than 0.5 seconds).
#
#-----------------------------------------------------------------------------

# Claude Berge puzzles

dfpn-clear-tt
loadsgf sgf/puzzles/5x5-berge-1.sgf
10 dfpn-solve-state white
#? [white]


# David Boll puzzles (kind of - based off positions on his site)

# Takes 838 seconds to solve.
# The losing moves in the mustplay are: i4, c7, h7
dfpn-clear-tt
loadsgf sgf/puzzles/9x9-boll-01.sgf
20 dfpn-solver-find-winning black
#? [h5 i5 i6 e7 g7 c8]

# Takes 3 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/9x9-boll-02.sgf
21 dfpn-solve-state white
#? [white]

# Takes 3534 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/11x11-boll-03.sgf
22 dfpn-solver-find-winning white
#? [d5 d8 c9 b10]

# Takes 3245 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/11x11-boll-04.sgf
23 dfpn-solver-find-winning black
#? [f7 c8]

# Takes 3626 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/11x11-boll-05.sgf
24 dfpn-solver-find-winning black
#? [f7 d8 b9 b10 a11]


# Cameron Browne puzzles

# Takes 66 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/11x11-browne-01.sgf
30 dfpn-solver-find-winning black
#? [c4 e4 f4 k5 f6 j7]

dfpn-clear-tt
loadsgf sgf/puzzles/11x11-browne-02.sgf
31 dfpn-solver-find-winning white
#? [c5]

dfpn-clear-tt
loadsgf sgf/puzzles/11x11-browne-03.sgf
32 dfpn-solve-state white
#? [white]


# Bert Enderton puzzles

dfpn-clear-tt
loadsgf sgf/puzzles/7x7-enderton-01.sgf
40 dfpn-solver-find-winning white
#? [d5]

dfpn-clear-tt
loadsgf sgf/puzzles/7x7-enderton-02.sgf
41 dfpn-solver-find-winning white
#? [d6]

dfpn-clear-tt
loadsgf sgf/puzzles/7x7-enderton-03.sgf
42 dfpn-solver-find-winning white
#? [e3 e6]

# Takes 34 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-enderton-04.sgf
43 dfpn-solver-find-winning black
#? [f2 e3 d4]


# Ryan Hayward puzzles

dfpn-clear-tt
loadsgf sgf/puzzles/6x6-hayward-01.sgf
50 dfpn-solver-find-winning white
#? [d3]


# Piet Hein puzzles

# 7f0ff9e17720 severe: ../../src/hex/StoneBoard.hpp:437: benzene::HexColor
# benzene::StoneBoard::WhoseTurn() const: Assertion `IsStandardPosition()' failed
# dfpn-clear-tt
# loadsgf sgf/puzzles/11x11-hein-01.sgf
# 60 dfpn-solve-state white
# #? [white]

dfpn-clear-tt
loadsgf sgf/puzzles/3x3-hein-02.sgf
61 dfpn-solve-state white
#? [white]

dfpn-clear-tt
loadsgf sgf/puzzles/6x6-hein-03.sgf
62 dfpn-solve-state white
#? [white]

dfpn-clear-tt
loadsgf sgf/puzzles/4x4-hein-04.sgf
63 dfpn-solve-state white
#? [white]

dfpn-clear-tt
loadsgf sgf/puzzles/4x4-hein-05.sgf
64 dfpn-solve-state white
#? [white]

# dfpn-clear-tt
loadsgf sgf/puzzles/5x5-hein-06.sgf
65 dfpn-solver-find-winning white
#? [c2]

dfpn-clear-tt
loadsgf sgf/puzzles/6x6-hein-07.sgf
66 dfpn-solve-state white
#? [white]


# Philip Henderson puzzles

# Takes 2447 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/9x9-henderson-01.sgf
70 dfpn-solver-find-winning black
#? [c4 b5 g5]

dfpn-clear-tt
loadsgf sgf/puzzles/6x6-henderson-02.sgf
71 dfpn-solver-find-winning black
#? [b4]

# Takes 205 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/11x11-henderson-03.sgf
72 dfpn-solve-state black
#? [black]

dfpn-clear-tt
loadsgf sgf/puzzles/6x6-henderson-04.sgf
73 dfpn-solver-find-winning black
#? [c4]

# Takes 5 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-05.sgf
74 dfpn-solver-find-winning white
#? [b6]

# Takes 57 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-06.sgf
75 dfpn-solver-find-winning black
#? [f4]

# Takes 64 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-07.sgf
76 dfpn-solver-find-winning black
#? [g2 c4]

# Takes 41 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-08.sgf
77 dfpn-solver-find-winning white
#? [e3]

# Takes 1 second to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/6x6-henderson-09.sgf
78 dfpn-solver-find-winning black
#? [d4]

dfpn-clear-tt
loadsgf sgf/puzzles/6x6-henderson-10.sgf
79 dfpn-solver-find-winning black
#? [c4]

dfpn-clear-tt
loadsgf sgf/puzzles/5x5-henderson-11.sgf
80 dfpn-solver-find-winning white
#? [c4]

dfpn-clear-tt
loadsgf sgf/puzzles/5x5-henderson-12.sgf
81 dfpn-solver-find-winning black
#? [b3]

dfpn-clear-tt
loadsgf sgf/puzzles/6x6-henderson-13.sgf
82 dfpn-solver-find-winning white
#? [b6]

dfpn-clear-tt
loadsgf sgf/puzzles/6x6-henderson-14.sgf
83 dfpn-solver-find-winning white
#? [e2]

dfpn-clear-tt
loadsgf sgf/puzzles/6x6-henderson-15.sgf
84 dfpn-solver-find-winning black
#? [b3 d5]

dfpn-clear-tt
loadsgf sgf/puzzles/6x6-henderson-16.sgf
85 dfpn-solver-find-winning white
#? [e1 b5]

dfpn-clear-tt
loadsgf sgf/puzzles/6x6-henderson-17.sgf
86 dfpn-solver-find-winning white
#? [c4]

dfpn-clear-tt
loadsgf sgf/puzzles/6x6-henderson-18.sgf
87 dfpn-solver-find-winning black
#? [f4]

dfpn-clear-tt
loadsgf sgf/puzzles/6x6-henderson-19.sgf
88 dfpn-solver-find-winning black
#? [c4]

dfpn-clear-tt
loadsgf sgf/puzzles/6x6-henderson-20.sgf
89 dfpn-solver-find-winning white
#? [d3]

dfpn-clear-tt
loadsgf sgf/puzzles/6x6-henderson-21.sgf
90 dfpn-solver-find-winning black
#? [b3 c3]

dfpn-clear-tt
loadsgf sgf/puzzles/6x6-henderson-22.sgf
91 dfpn-solver-find-winning black
#? [c4]

# Takes 1 second to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-23.sgf
92 dfpn-solver-find-winning white
#? []

dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-24.sgf
93 dfpn-solver-find-winning white
#? [d5]

# Takes 4 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-25.sgf
94 dfpn-solver-find-winning black
#? [f2 f5]

# Takes 10 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-26.sgf
95 dfpn-solver-find-winning black
#? [d5]

# Takes 3 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-27.sgf
96 dfpn-solver-find-winning black
#? []

# Takes 4 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-28.sgf
97 dfpn-solver-find-winning black
#? [d5]

# Takes 1 second to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-29.sgf
98 dfpn-solver-find-winning white
#? [c5]

# Takes 39 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-30.sgf
99 dfpn-solver-find-winning white
#? [d3]

# Takes 8 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-31.sgf
100 dfpn-solver-find-winning white
#? [d3]

# Takes 11 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-32.sgf
101 dfpn-solver-find-winning black
#? [c5]

# Takes 47 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-33.sgf
102 dfpn-solver-find-winning black
#? [e4]

dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-34.sgf
103 dfpn-solver-find-winning black
#? [f2 f5]

# Takes 70 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-35.sgf
104 dfpn-solver-find-winning white
#? [b2 b4]

# Takes 2 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-36.sgf
105 dfpn-solver-find-winning white
#? [b5]

# Takes 3 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-37.sgf
106 dfpn-solver-find-winning black
#? [b4]

# Takes 11 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-38.sgf
107 dfpn-solver-find-winning black
#? [b3 b6]

# Takes 6 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-39.sgf
108 dfpn-solver-find-winning black
#? [d4]

# Takes 1 second to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-40.sgf
109 dfpn-solver-find-winning white
#? [e3]

# Takes 13 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-41.sgf
110 dfpn-solver-find-winning white
#? [d5]

# Takes 24 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-42.sgf
111 dfpn-solver-find-winning white
#? [d4]

# Takes 37 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-43.sgf
112 dfpn-solver-find-winning white
#? [c6]

# Takes 1 second to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-44.sgf
113 dfpn-solver-find-winning black
#? [b7]

# Takes 1 second to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-45.sgf
114 dfpn-solver-find-winning white
#? [c2 f2]

# Takes 3 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-46.sgf
115 dfpn-solver-find-winning black
#? [d4]

# Takes 8 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-47.sgf
116 dfpn-solver-find-winning black
#? [f3 b4]

dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-48.sgf
117 dfpn-solver-find-winning black
#? [f2 c5]

# Takes 1 second to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-49.sgf
118 dfpn-solver-find-winning white
#? [e3]

dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-50.sgf
119 dfpn-solver-find-winning white
#? [d5]

# Takes 12 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-51.sgf
120 dfpn-solver-find-winning black
#? [c5]

# Takes 1 second to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-52.sgf
121 dfpn-solver-find-winning black
#? [e4]

dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-53.sgf
122 dfpn-solver-find-winning white
#? [c5]

# Takes 5 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-54.sgf
123 dfpn-solver-find-winning white
#? [c5]

# Takes 2 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-55.sgf
124 dfpn-solver-find-winning black
#? [e4]

# Takes 712 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/8x8-henderson-56.sgf
125 dfpn-solver-find-winning white
#? [e6]

# Takes 27 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-57.sgf
126 dfpn-solver-find-winning black
#? [c4]

# Takes 12 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-58.sgf
127 dfpn-solver-find-winning white
#? [c2]

# Takes 3 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-59.sgf
128 dfpn-solver-find-winning white
#? [b4]

dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-60.sgf
129 dfpn-solver-find-winning white
#? [c7]

# Takes 1 second to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-61.sgf
130 dfpn-solver-find-winning white
#? [e4]

dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-62.sgf
131 dfpn-solver-find-winning black
#? [f3]

# Takes 264 seconds to solve
dfpn-clear-tt
loadsgf sgf/puzzles/11x11-henderson-63.sgf
132 dfpn-solver-find-winning black
#? [i7]

# Takes 224 seconds to solve
dfpn-clear-tt
loadsgf sgf/puzzles/9x9-henderson-64.sgf
133 dfpn-solver-find-winning black
#? [b6]

# Takes 338 seconds to solve
dfpn-clear-tt
loadsgf sgf/puzzles/9x9-henderson-65.sgf
134 dfpn-solver-find-winning white
#? [f7]

# Takes 2617 seconds to solve
dfpn-clear-tt
loadsgf sgf/puzzles/8x8-henderson-66.sgf
135 dfpn-solver-find-winning white
#? [d5]

# Takes 2674 seconds to solve
dfpn-clear-tt
loadsgf sgf/puzzles/9x9-henderson-67.sgf
136 dfpn-solver-find-winning white
#? [d8]

dfpn-clear-tt
loadsgf sgf/puzzles/9x9-henderson-68.sgf
137 dfpn-solver-find-winning black
#? [g7]

# Takes 143 seconds to solve
dfpn-clear-tt
loadsgf sgf/puzzles/9x9-henderson-69.sgf
138 dfpn-solver-find-winning black
#? [h5]

# Takes 75 seconds to solve
dfpn-clear-tt
loadsgf sgf/puzzles/9x9-henderson-70.sgf
139 dfpn-solver-find-winning white
#? [f4]

# Takes 37 seconds to solve
dfpn-clear-tt
loadsgf sgf/puzzles/11x11-henderson-71.sgf
140 dfpn-solver-find-winning black
#? [e8]

# Takes 729 seconds to solve
dfpn-clear-tt
loadsgf sgf/puzzles/9x9-henderson-72.sgf
141 dfpn-solver-find-winning white
#? [e3]

# Takes 1839 seconds to solve
dfpn-clear-tt
loadsgf sgf/puzzles/9x9-henderson-73.sgf
142 dfpn-solver-find-winning white
#? [f3]

dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-74.sgf
143 dfpn-solver-find-winning black
#? [c4]

# Takes 5 seconds to solve
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-75.sgf
144 dfpn-solver-find-winning black
#? [d3]

# Takes 8 seconds to solve
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-76.sgf
145 dfpn-solver-find-winning black
#? [g2]

# Takes 9 seconds to solve
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-77.sgf
146 dfpn-solver-find-winning black
#? [c5]

# Takes 7 seconds to solve
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-78.sgf
147 dfpn-solver-find-winning black
#? [c3]

# Takes 12 seconds to solve
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-79.sgf
148 dfpn-solver-find-winning white
#? [e5]

# Takes 6 seconds to solve
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-80.sgf
149 dfpn-solver-find-winning white
#? [e6]

# Takes 4 seconds to solve
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-henderson-81.sgf
150 dfpn-solver-find-winning black
#? [e5]

dfpn-clear-tt
loadsgf sgf/puzzles/6x6-henderson-82.sgf
151 dfpn-solver-find-winning black
#? [c4]

# Takes 443 seconds to solve
dfpn-clear-tt
loadsgf sgf/puzzles/11x11-henderson-83.sgf
152 dfpn-solver-find-winning white
#? [i10]

# Takes 8 seconds to solve
dfpn-clear-tt
loadsgf sgf/puzzles/11x11-henderson-84.sgf
153 dfpn-solver-find-winning black
#? [b8]

# Takes 1 second to solve
dfpn-clear-tt
loadsgf sgf/puzzles/11x11-henderson-85.sgf
154 dfpn-solver-find-winning white
#? [h2]

# Takes 8 seconds to solve
dfpn-clear-tt
loadsgf sgf/puzzles/11x11-henderson-86.sgf
155 dfpn-solver-find-winning white
#? [d4]

# Takes 1415 seconds to solve
dfpn-clear-tt
loadsgf sgf/puzzles/11x11-henderson-87.sgf
156 dfpn-solver-find-winning white
#? [h6 f8]

# Takes 194 seconds to solve
dfpn-clear-tt
loadsgf sgf/puzzles/8x8-henderson-88.sgf
157 dfpn-solver-find-winning black
#? [g6 a7]

# Takes 229 seconds to solve
dfpn-clear-tt
loadsgf sgf/puzzles/8x8-henderson-89.sgf
158 dfpn-solver-find-winning white
#? [c3]

# Takes 800 seconds to solve
dfpn-clear-tt
loadsgf sgf/puzzles/8x8-henderson-90.sgf
159 dfpn-solver-find-winning black
#? [a4 b4]

# Takes 2126 seconds to solve
dfpn-clear-tt
loadsgf sgf/puzzles/8x8-henderson-91.sgf
160 dfpn-solver-find-winning white
#? [d2 b3]

# Takes 8 seconds to solve
dfpn-clear-tt
loadsgf sgf/puzzles/8x8-henderson-92.sgf
161 dfpn-solver-find-winning black
#? [c4]

# Takes 2 seconds to solve
dfpn-clear-tt
loadsgf sgf/puzzles/8x8-henderson-93.sgf
162 dfpn-solver-find-winning white
#? [e6]

# Takes 9 seconds to solve
dfpn-clear-tt
loadsgf sgf/puzzles/8x8-henderson-94.sgf
163 dfpn-solver-find-winning white
#? [f3]

# Takes 12 seconds to solve
dfpn-clear-tt
loadsgf sgf/puzzles/8x8-henderson-95.sgf
164 dfpn-solver-find-winning white
#? [d7]

# Takes 104 seconds to solve
dfpn-clear-tt
loadsgf sgf/puzzles/8x8-henderson-96.sgf
165 dfpn-solver-find-winning white
#? [e1]

# Takes 13 seconds to solve
dfpn-clear-tt
loadsgf sgf/puzzles/8x8-henderson-97.sgf
166 dfpn-solver-find-winning white
#? []

# Takes 1 second to solve
dfpn-clear-tt
loadsgf sgf/puzzles/8x8-henderson-98.sgf
167 dfpn-solver-find-winning white
#? [e3]

# Takes 11 seconds to solve
dfpn-clear-tt
loadsgf sgf/puzzles/8x8-henderson-99.sgf
168 dfpn-solver-find-winning black
#? [a7]

# Takes 5 seconds to solve
dfpn-clear-tt
loadsgf sgf/puzzles/9x9-henderson-100.sgf
169 dfpn-solver-find-winning white
#? [e2]

# Takes 103 seconds to solve
dfpn-clear-tt
loadsgf sgf/puzzles/9x9-henderson-101.sgf
170 dfpn-solver-find-winning white
#? [g7]

# Takes 113 seconds to solve
dfpn-clear-tt
loadsgf sgf/puzzles/9x9-henderson-102.sgf
171 dfpn-solver-find-winning black
#? [h5]

# Takes 82 seconds to solve
dfpn-clear-tt
loadsgf sgf/puzzles/11x11-henderson-103.sgf
172 dfpn-solver-find-winning black
#? [e7]

# Takes 107 seconds to solve
dfpn-clear-tt
loadsgf sgf/puzzles/9x9-henderson-104.sgf
173 dfpn-solver-find-winning white
#? [f7]

# Takes 16 seconds to solve
dfpn-clear-tt
loadsgf sgf/puzzles/9x9-henderson-105.sgf
174 dfpn-solver-find-winning black
#? [h4]

# Takes 682 seconds to solve
dfpn-clear-tt
loadsgf sgf/puzzles/11x11-henderson-106.sgf
175 dfpn-solver-find-winning white
#? [f5]


# Little Golem puzzles

# Takes 2 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/7x7-LG-01.sgf
180 dfpn-solver-find-winning white
#? [b6]

# Takes 1049 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/10x10-LG-01.sgf
181 dfpn-solve-state white
#? [white]
# Takes ??? to solve.
# dfpn-clear-tt
# 181 dfpn-solver-find-winning white
# #? [c4 c7]

# Takes 890 seconds to solve.
dfpn-clear-tt
loadsgf sgf/puzzles/10x10-LG-01sol.sgf
182 dfpn-solve-state black
#? [white]
