#-----------------------------------------------------------------------------
# Solves each 8x8 position with dfpn and checks winner.
# Uses a 10 stone database: make sure to delete '8x8.db' between runs. 
# Uses a 1 million state hash table.
#-----------------------------------------------------------------------------

boardsize 8 8
param_dfpn tt_bits 20
param_dfpn_db max_stones 10
dfpn-open-db 8x8.db

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
#? [white]

undo
dfpn-clear-tt
play b a5
15 dfpn-solve-state white
#? [black]

undo
dfpn-clear-tt
play b a6
16 dfpn-solve-state white
#? [white]

undo
dfpn-clear-tt
play b a7
17 dfpn-solve-state white
#? [black]

undo
dfpn-clear-tt
play b a8
18 dfpn-solve-state white
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
#? [black]

undo
dfpn-clear-tt
play b b8
28 dfpn-solve-state white
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
#? [white]

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
play b c8
38 dfpn-solve-state white
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

undo
dfpn-clear-tt
play b d5
45 dfpn-solve-state white
#? [black]

undo
dfpn-clear-tt
play b d6
46 dfpn-solve-state white
#? [black]

undo
dfpn-clear-tt
play b d7
47 dfpn-solve-state white
#? [white]

undo
dfpn-clear-tt
play b d8
48 dfpn-solve-state white
#? [white]
