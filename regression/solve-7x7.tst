#-----------------------------------------------------------------------------
# Solves each 7x7 position and checks winner.
#
# $Id: solve-7x7.tst 1722 2008-10-31 02:13:55Z broderic $
#-----------------------------------------------------------------------------

boardsize 7 7

play b a1
11 solve-state w
#? [white]

undo
play b a2
12 solve-state w
#? [white]

undo
play b a3
13 solve-state w
#? [white]

undo
play b a4
14 solve-state w
#? [black]

undo
play b a5
15 solve-state w
#? [white]

undo
play b a6
16 solve-state w
#? [black]

undo
play b a7
17 solve-state w
#? [black]

undo
play b b1
21 solve-state w
#? [white]

undo
play b b2
22 solve-state w
#? [white]

undo
play b b3
23 solve-state w
#? [black]

undo
play b b4
24 solve-state w
#? [black]

undo
play b b5
25 solve-state w
#? [black]

undo
play b b6
26 solve-state w
#? [black]

undo
play b b7
27 solve-state w
#? [white]

undo
play b c1
31 solve-state w
#? [white]

undo
play b c2
32 solve-state w
#? [black]

undo
play b c3
33 solve-state w
#? [black]

undo
play b c4
34 solve-state w
#? [black]

undo
play b c5
35 solve-state w
#? [black]

undo
play b c6
36 solve-state w
#? [black]

undo
play b c7
37 solve-state w
#? [white]

undo
play b d1
41 solve-state w
#? [white]

undo
play b d2
42 solve-state w
#? [white]

undo
play b d3
43 solve-state w
#? [black]

undo
play b d4
44 solve-state w
#? [black]

undo
play b d5
45 solve-state w
#? [black]

undo
play b d6
46 solve-state w
#? [white]

undo
play b d7
47 solve-state w
#? [white]

undo
play b e1
51 solve-state w
#? [white]

undo
play b e2
52 solve-state w
#? [black]

undo
play b e3
53 solve-state w
#? [black]

undo
play b e4
54 solve-state w
#? [black]

undo
play b e5
55 solve-state w
#? [black]

undo
play b e6
56 solve-state w
#? [black]

undo
play b e7
57 solve-state w
#? [white]

undo
play b f1
61 solve-state w
#? [white]

undo
play b f2
62 solve-state w
#? [black]

undo
play b f3
63 solve-state w
#? [black]

undo
play b f4
64 solve-state w
#? [black]

undo
play b f5
65 solve-state w
#? [black]

undo
play b f6
66 solve-state w
#? [white]

undo
play b f7
67 solve-state w
#? [white]

undo
play b g1
71 solve-state w
#? [black]

undo
play b g2
72 solve-state w
#? [black]

undo
play b g3
73 solve-state w
#? [white]

undo
play b g4
74 solve-state w
#? [black]

undo
play b g5
75 solve-state w
#? [white]

undo
play b g6
76 solve-state w
#? [white]

undo
play b g7
77 solve-state w
#? [white]
