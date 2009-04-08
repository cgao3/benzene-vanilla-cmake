#-----------------------------------------------------------------------------
# Solves each 6x6 position and checks winner. 
#
# $Id: solve-6x6.tst 1722 2008-10-31 02:13:55Z broderic $
#-----------------------------------------------------------------------------

boardsize 6 6

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
#? [black]

undo
play b a4
14 solve-state w
#? [black]

undo
play b a5
15 solve-state w
#? [black]

undo
play b a6
16 solve-state w
#? [black]

undo
play b b1
21 solve-state w
#? [white]

undo
play b b2
22 solve-state w
#? [black]

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
#? [white]

undo
play b d1
41 solve-state w
#? [white]

undo
play b d2
42 solve-state w
#? [black]

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
#? [white]

undo
play b f1
61 solve-state w
#? [black]

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
#? [white]

undo
play b f6
66 solve-state w
#? [white]
