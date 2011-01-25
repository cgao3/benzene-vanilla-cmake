#-----------------------------------------------------------------------------
# Tests inferior cell engine.
#-----------------------------------------------------------------------------

# Turn these settings on for improved identification of dead regions
param_player_ice find_three_sided_dead_regions 1
param_player_ice iterative_dead_regions 1

# a common case near the edge
loadsgf sgf/inferior/basic.sgf
10 compute-inferior black
#? [.*e1 fcb.*g1 fcb.*]

# two variations of this common case that break if we don't
# consider neighbour cliques through groups
loadsgf sgf/inferior/basic-variation1.sgf
11 compute-inferior black
#? [.*e1 fcb.*g1 fcb.*]

loadsgf sgf/inferior/basic-variation2.sgf
12 compute-inferior black
#? [.*e1 fcb.*g1 fcb.*]

# another trivial case near the edge
loadsgf sgf/inferior/basic2.sgf
13 compute-inferior white
#? [.*a5 fd.*]


# basic chain of capturing
loadsgf sgf/inferior/chain-of-captured.sgf
20 compute-inferior white
#? [.*a7 fcw.*b7 fcw.*]

# computing captured, the game is already decided
loadsgf sgf/inferior/game-decided-via-captured.sgf
21 compute-inferior black
#? [.*d1 fcb.*e1 fcb.*i1 fcw.*j1 fcw.*c2 fcb.*d2 fcb.*h2 fcw.*j2 fcw.*k2 fd.*a3 fcw.*c3 fcw.*a4 fcw.*b4 fcw.*]

# tricky sequence of capturing
loadsgf sgf/inferior/massive-captured-missing.sgf
22 compute-inferior white
#? [.*f5 fcw.*g5 fcw.*d6 fcw.*e6 fcw.*f6 fcw.*e7 fcw.*]*


# if checks neighbours through groups, should find this dead cell
loadsgf sgf/inferior/dead-thru-group.sgf
30 compute-inferior black
#? [.*e6 fd.*]

loadsgf sgf/inferior/use-captain-not-cell.sgf
31 compute-inferior black
#? [.*e6 fcb.*g6 fcb.*]


# c11 and d10 will only be found as a pre-simplicial pair
# if we iterate on the search for vulnerable cells
loadsgf sgf/inferior/iterate-vulnerable.sgf
40 compute-inferior black
#? [.*d10 fcb.*c11 fcb.*d11 fd.*e11 fcb.*f11 fcb.*]

# check that the vulnerable calculations go through groups
# and edges properly
loadsgf sgf/inferior/clique-thru-groups.sgf
50 compute-inferior black
#? [.*c6 fcb.*e6 fcb.*]

loadsgf sgf/inferior/thorough-thru-edge.sgf
51 compute-inferior white
#? [.*e6 iv\[d6\].*g6 fcb.*h7 fcb.*i7 fcb.*i8 fcb.*]

loadsgf sgf/inferior/adj-to-both-edges.sgf
52 compute-inferior white
#? [!.*f5.*]

loadsgf sgf/inferior/all-nbs-adj-to-one-edge.sgf
53 compute-inferior white
#? [.*e6 f.*f6 f.*e7 f.*f7 f.*]

loadsgf sgf/inferior/strange-path.sgf
54 compute-inferior black
#? [.*f3 iv\[e6\].*e9 iv\[e6\].*]

loadsgf sgf/inferior/dead-thru-edge.sgf
55 compute-inferior black
#? [.*e7 fd.*]

loadsgf sgf/inferior/complex-presimplicial-pair.sgf
56 compute-inferior black
#? [.*e3 fcw.*d5 fcw.*]

# d4 is killed by d3, and (d5,f5) are either
# both dead or both black captured.
loadsgf sgf/inferior/complex-dead.sgf
57 compute-inferior black
#? [.*d4 iv\[d3\].*d5 f.b.*f5 f.b.*]

loadsgf sgf/inferior/vulnerable-via-superset-group.sgf
58 compute-inferior black
#? [.*f8 iv\[f9\].*]


# knowing about fill-in can give us more vulnerable cells
loadsgf sgf/inferior/probes-vulnerable.sgf
60 compute-inferior white
#? [.*f1 iv.*g1 iv.*f2 iv.*]*


# checks for hand-coded patterns
boardsize 11 11
70 compute-inferior white
#? [.*j10 id\[j9\].*i11 id\[i10\].*]


# Checks whether detects regions that are completely
# useless, both near edge and in centre
# Note that dead regions can be any colour of fill-in
loadsgf sgf/inferior/large-dead-region.sgf
80 compute-inferior white
#? [.*c9 f.*d9 f.*c10 f.*]

loadsgf sgf/inferior/large-dead-region2.sgf
81 compute-inferior white
#? [.*e9 f.*f9 f.*e10 f.*]

loadsgf sgf/inferior/centre-dead-region1.sgf
82 compute-inferior white
#? [.*f6 f.*]

loadsgf sgf/inferior/centre-dead-region2.sgf
83 compute-inferior black
#? [.*f6 f.*]

loadsgf sgf/inferior/centre-dead-region3.sgf
84 compute-inferior black
#? [.*f8 f.*]

loadsgf sgf/inferior/edge-sealed-dead-region1.sgf
85 compute-inferior white
#? [.*b2 f.*a5 f.*]

loadsgf sgf/inferior/edge-sealed-dead-region2.sgf
86 compute-inferior white
#? [.*f8 fcb.*f9 f.*]

loadsgf sgf/inferior/edge-sealed-dead-region3.sgf
87 compute-inferior black
#? [.*g7 f.*g9 f.*]

loadsgf sgf/inferior/dead-region-via-one-cut.sgf
88 compute-inferior white
#? [.*e5 f.*]

loadsgf sgf/inferior/dead-region-via-two-clique.sgf
89 compute-inferior white
#? [.*e5 f.*]


# There is a problem with using Jack's vulnerable patterns
# for presimplicial pairs. For patterns 537, 1737, and 2436
# the presimplicial pair code only works if we also fill-in
# the other two empty locations as captured (and NOT as dead).
# For patterns 707, 717, 1503, 1523, 1777, 1787, 2067, 2279,
# 2291, 3198, and 3378 the problem is more serious: we cannot
# use these vulnerable patterns for presimplicial pairs at all,
# since they rely on fill-in that is only obtained by only one
# of the two vulnerable cells. The tests below check for these
# issues.
#
# To make matters worse, some vulnerable patterns contain
# captured or dead cells, so they are repetitive/redundant.
# Here are some I've found: 441, 467, 517, 527, 627, 647,
# 747, 1283, 1293, 1323, 1363, 1403, 1443, 1647, 1657, 1687,
# 1697, 1727, 1767, 1837, 1857, 2007, 2047, 2255, 2412, 2424,
# 3270, 3330, and 3919.

loadsgf sgf/inferior/local-pattern-fillin1.sgf
90 compute-inferior white
#? [.*e3 iv\[d4\].*d4 iv\[e3\].*]


# Vulnerable patterns are troublesome, and cannot always be removed
# from the mustplay. Here are some weird/painful situations:

# Old results for the test below were more interesting:
# #? [.*e5 iv\[e6\].*c6 iv\[c7\].*e6 id\[d6\].*c7 id\[d6\].*d7 iv\[d6\].*]
loadsgf sgf/inferior/vulnerable-dominated-cycle.sgf
100 compute-dominated black
#? [.*e5 id\[d6\].*e6 id\[d6\].*c7 id\[d6\].*d7 id\[e6\].*]




# Combinatorial decomposition tests
# Use vc-build command to get results, as not computed by ice.

loadsgf sgf/inferior/dead-via-isolated-VC1.sgf
110 vc-build white
#? [.*c2 fd.*]

loadsgf sgf/inferior/dead-via-isolated-VC2.sgf
111 vc-build white
#? [.*c2 fd.*f3 fcb.*]

loadsgf sgf/inferior/dead-via-isolated-VC3.sgf
112 vc-build black
#? [.*a1 fcb.*c2 fcb.*]

loadsgf sgf/inferior/dead-via-isolated-VC4.sgf
113 vc-build black
#? [.*g4 fd.*h4 fcw.*h7 fcw.*f9 fd.*]

loadsgf sgf/inferior/double-decomposition1.sgf
114 vc-build white
#? [.*i3 fcb.*b4 fcw.*d4 f.*i4 fd.*j5 fcw.*j9 fcw.*h11 fd.*]

loadsgf sgf/inferior/decomp-with-6-groups.sgf
115 vc-build black
#? [.*i6 fcb.*j7 fcb.*k8 fd.*i10 fcb.*]*


# A strange sort of `globally' captured/vulnerable/dead

loadsgf sgf/inferior/global-captured-and-dead.sgf
120 compute-inferior white
#? [.*j2 fcb.*i3 fcb.*g6 fd.*c7 fcb.*d7 fcb.*i8 fcb.*i9 fcb.*]*


# Dead cells not identifiable via `ring' idea

loadsgf sgf/inferior/dead-not-ring01.sgf
130 compute-inferior white
#? [.*e6 fd.*]

loadsgf sgf/inferior/dead-not-ring02.sgf
131 compute-inferior white
#? [.*f5 fd.*]

loadsgf sgf/inferior/dead-not-ring03.sgf
132 compute-inferior black
#? [.*f5 fd.*]

loadsgf sgf/inferior/dead-not-ring04.sgf
133 compute-inferior black
#? [.*e6 fd.*]

loadsgf sgf/inferior/dead-not-ring05.sgf
134 compute-inferior white
#? [.*f6 fd.*]

# Fails if ice-iterative-dead-regions set to false
loadsgf sgf/inferior/dead-not-ring06.sgf
135 compute-inferior black
#? [.*g4 fcw.*g5 fd.*e7 fcw.*]
