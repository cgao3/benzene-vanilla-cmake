#-----------------------------------------------------------------------------
# Solves each 6x6 position and checks winner. 
#-----------------------------------------------------------------------------

boardsize 6 6

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
#? [black]

undo
play b a4
14 dfs-solve-state w
#? [black]

undo
play b a5
15 dfs-solve-state w
#? [black]

undo
play b a6
16 dfs-solve-state w
#? [black]

undo
play b b1
21 dfs-solve-state w
#? [white]

undo
play b b2
22 dfs-solve-state w
#? [black]

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
#? [white]

undo
play b d1
41 dfs-solve-state w
#? [white]

undo
play b d2
42 dfs-solve-state w
#? [black]

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
#? [white]

undo
play b f1
61 dfs-solve-state w
#? [black]

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
#? [white]

undo
play b f6
66 dfs-solve-state w
#? [white]
