#-----------------------------------------------------------------------------
# Solves each 7x7 position with dfpn and checks winner.
#-----------------------------------------------------------------------------

boardsize 7 7

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
#? [white]

undo
dfpn-clear-tt
play b a4
14 dfpn-solve-state white
#? [black]

undo
dfpn-clear-tt
play b a5
15 dfpn-solve-state white
#? [white]

undo
dfpn-clear-tt
play b a6
16 dfpn-solve-state white
#? [black]

undo
dfpn-clear-tt
play b a7
17 dfpn-solve-state white
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
#? [white]

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
#? [black]

undo
dfpn-clear-tt
play b b7
27 dfpn-solve-state white
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
#? [black]

undo
dfpn-clear-tt
play b c7
37 dfpn-solve-state white
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
#? [white]

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
