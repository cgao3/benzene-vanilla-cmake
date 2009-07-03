#-----------------------------------------------------------------------------
# Solves each 7x7 position with dfpn and checks winner.
#-----------------------------------------------------------------------------

boardsize 7 7

play b a1
11 solve-state-dfpn w
#? [white]

undo
play b a2
12 solve-state-dfpn w
#? [white]

undo
play b a3
13 solve-state-dfpn w
#? [white]

undo
play b a4
14 solve-state-dfpn w
#? [black]

undo
play b a5
15 solve-state-dfpn w
#? [white]

undo
play b a6
16 solve-state-dfpn w
#? [black]

undo
play b a7
17 solve-state-dfpn w
#? [black]

undo
play b b1
21 solve-state-dfpn w
#? [white]

undo
play b b2
22 solve-state-dfpn w
#? [white]

undo
play b b3
23 solve-state-dfpn w
#? [black]

undo
play b b4
24 solve-state-dfpn w
#? [black]

undo
play b b5
25 solve-state-dfpn w
#? [black]

undo
play b b6
26 solve-state-dfpn w
#? [black]

undo
play b b7
27 solve-state-dfpn w
#? [white]

undo
play b c1
31 solve-state-dfpn w
#? [white]

undo
play b c2
32 solve-state-dfpn w
#? [black]

undo
play b c3
33 solve-state-dfpn w
#? [black]

undo
play b c4
34 solve-state-dfpn w
#? [black]

undo
play b c5
35 solve-state-dfpn w
#? [black]

undo
play b c6
36 solve-state-dfpn w
#? [black]

undo
play b c7
37 solve-state-dfpn w
#? [white]

undo
play b d1
41 solve-state-dfpn w
#? [white]

undo
play b d2
42 solve-state-dfpn w
#? [white]

undo
play b d3
43 solve-state-dfpn w
#? [black]

undo
play b d4
44 solve-state-dfpn w
#? [black]

