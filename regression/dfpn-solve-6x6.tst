#-----------------------------------------------------------------------------
# Solves each 6x6 position with dfpn and checks winner.
#-----------------------------------------------------------------------------

boardsize 6 6

play b a1
11 dfpn-solve-state white
#? [white]

undo
dfpn-clear-tt
play b a2
12 dfpn-solve-state white
#? [white]

undo
dfpn-clear-tt
play b a3
13 dfpn-solve-state white
#? [black]

undo
dfpn-clear-tt
play b a4
14 dfpn-solve-state white
#? [black]

undo
dfpn-clear-tt
play b a5
15 dfpn-solve-state white
#? [black]

undo
dfpn-clear-tt
play b a6
16 dfpn-solve-state white
#? [black]

undo
dfpn-clear-tt
play b b1
21 dfpn-solve-state white
#? [white]

undo
dfpn-clear-tt
play b b2
22 dfpn-solve-state white
#? [black]

undo
dfpn-clear-tt
play b b3
23 dfpn-solve-state white
#? [black]

undo
dfpn-clear-tt
play b b4
24 dfpn-solve-state white
#? [black]

undo
dfpn-clear-tt
play b b5
25 dfpn-solve-state white
#? [black]

undo
dfpn-clear-tt
play b b6
26 dfpn-solve-state white
#? [white]

undo
dfpn-clear-tt
play b c1
31 dfpn-solve-state white
#? [white]

undo
dfpn-clear-tt
play b c2
32 dfpn-solve-state white
#? [black]

undo
dfpn-clear-tt
play b c3
33 dfpn-solve-state white
#? [black]

undo
dfpn-clear-tt
play b c4
34 dfpn-solve-state white
#? [black]

undo
dfpn-clear-tt
play b c5
35 dfpn-solve-state white
#? [black]

undo
dfpn-clear-tt
play b c6
36 dfpn-solve-state white
#? [white]

undo
dfpn-clear-tt
play b d1
41 dfpn-solve-state white
#? [white]

undo
dfpn-clear-tt
play b d2
42 dfpn-solve-state white
#? [black]

undo
dfpn-clear-tt
play b d3
43 dfpn-solve-state white
#? [black]

undo
dfpn-clear-tt
play b d4
44 dfpn-solve-state white
#? [black]

undo
dfpn-clear-tt
play b d5
45 dfpn-solve-state white
#? [black]

undo
dfpn-clear-tt
play b d6
46 dfpn-solve-state white
#? [white]

undo
dfpn-clear-tt
play b e1
51 dfpn-solve-state white
#? [white]

undo
dfpn-clear-tt
play b e2
52 dfpn-solve-state white
#? [black]

undo
dfpn-clear-tt
play b e3
53 dfpn-solve-state white
#? [black]

undo
dfpn-clear-tt
play b e4
54 dfpn-solve-state white
#? [black]

undo
dfpn-clear-tt
play b e5
55 dfpn-solve-state white
#? [black]

undo
dfpn-clear-tt
play b e6
56 dfpn-solve-state white
#? [white]

undo
dfpn-clear-tt
play b f1
61 dfpn-solve-state white
#? [black]

undo
dfpn-clear-tt
play b f2
62 dfpn-solve-state white
#? [black]

undo
dfpn-clear-tt
play b f3
63 dfpn-solve-state white
#? [black]

undo
dfpn-clear-tt
play b f4
64 dfpn-solve-state white
#? [black]

undo
dfpn-clear-tt
play b f5
65 dfpn-solve-state white
#? [white]

undo
dfpn-clear-tt
play b f6
66 dfpn-solve-state white
#? [white]
