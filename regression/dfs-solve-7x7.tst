#-----------------------------------------------------------------------------
# Solves each 7x7 position and checks winner.
#-----------------------------------------------------------------------------

boardsize 7 7

# Must turn of perm.inf. for solver!
param_solver_ice find_permanently_inferior 0

play b a1
11 dfs-solve-state w
#? [white]

undo
play b a2
12 dfs-solve-state w
#? [white]

undo
play b a3
13 dfs-solve-state w
#? [white]

undo
play b a4
14 dfs-solve-state w
#? [black]

undo
play b a5
15 dfs-solve-state w
#? [white]

undo
play b a6
16 dfs-solve-state w
#? [black]

undo
play b a7
17 dfs-solve-state w
#? [black]

undo
play b b1
21 dfs-solve-state w
#? [white]

undo
play b b2
22 dfs-solve-state w
#? [white]

undo
play b b3
23 dfs-solve-state w
#? [black]

undo
play b b4
24 dfs-solve-state w
#? [black]

undo
play b b5
25 dfs-solve-state w
#? [black]

undo
play b b6
26 dfs-solve-state w
#? [black]

undo
play b b7
27 dfs-solve-state w
#? [white]

undo
play b c1
31 dfs-solve-state w
#? [white]

undo
play b c2
32 dfs-solve-state w
#? [black]

undo
play b c3
33 dfs-solve-state w
#? [black]

undo
play b c4
34 dfs-solve-state w
#? [black]

undo
play b c5
35 dfs-solve-state w
#? [black]

undo
play b c6
36 dfs-solve-state w
#? [black]

undo
play b c7
37 dfs-solve-state w
#? [white]

undo
play b d1
41 dfs-solve-state w
#? [white]

undo
play b d2
42 dfs-solve-state w
#? [white]

undo
play b d3
43 dfs-solve-state w
#? [black]

undo
play b d4
44 dfs-solve-state w
#? [black]

undo
play b d5
45 dfs-solve-state w
#? [black]

undo
play b d6
46 dfs-solve-state w
#? [white]

undo
play b d7
47 dfs-solve-state w
#? [white]

undo
play b e1
51 dfs-solve-state w
#? [white]

undo
play b e2
52 dfs-solve-state w
#? [black]

undo
play b e3
53 dfs-solve-state w
#? [black]

undo
play b e4
54 dfs-solve-state w
#? [black]

undo
play b e5
55 dfs-solve-state w
#? [black]

undo
play b e6
56 dfs-solve-state w
#? [black]

undo
play b e7
57 dfs-solve-state w
#? [white]

undo
play b f1
61 dfs-solve-state w
#? [white]

undo
play b f2
62 dfs-solve-state w
#? [black]

undo
play b f3
63 dfs-solve-state w
#? [black]

undo
play b f4
64 dfs-solve-state w
#? [black]

undo
play b f5
65 dfs-solve-state w
#? [black]

undo
play b f6
66 dfs-solve-state w
#? [white]

undo
play b f7
67 dfs-solve-state w
#? [white]

undo
play b g1
71 dfs-solve-state w
#? [black]

undo
play b g2
72 dfs-solve-state w
#? [black]

undo
play b g3
73 dfs-solve-state w
#? [white]

undo
play b g4
74 dfs-solve-state w
#? [black]

undo
play b g5
75 dfs-solve-state w
#? [white]

undo
play b g6
76 dfs-solve-state w
#? [white]

undo
play b g7
77 dfs-solve-state w
#? [white]
