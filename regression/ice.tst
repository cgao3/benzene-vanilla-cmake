#-----------------------------------------------------------------------------
# Tests inferior cell engine.
#
# $Id: ice.tst 1786 2008-12-14 01:55:45Z broderic $
#-----------------------------------------------------------------------------

# Turn these settings on for improved identification of dead regions
param_player_ice find_three_sided_dead_regions 1
param_player_ice iterative_dead_regions 1

# a common case near the edge
loadsgf sgf/inferior/basic.sgf
10 compute-inferior black
#? [.*e1 b.*g1 b.*]

# two variations of this common case that break if we don't
# consider neighbour cliques through groups
loadsgf sgf/inferior/basic-variation1.sgf
11 compute-inferior black
#? [.*e1 b.*g1 b.*]

loadsgf sgf/inferior/basic-variation2.sgf
12 compute-inferior black
#? [.*e1 b.*g1 b.*]

# another trivial case near the edge
loadsgf sgf/inferior/basic2.sgf
13 compute-inferior white
#? [.*a5 d.*]


# basic chain of capturing
loadsgf sgf/inferior/chain-of-captured.sgf
20 compute-inferior white
#? [.*a7 w.*b7 w.*]

# computing captured, the game is already decided
loadsgf sgf/inferior/game-decided-via-captured.sgf
21 compute-inferior black
#? [.*d1 b.*e1 b.*i1 w.*j1 w.*c2 b.*d2 b.*h2 w.*j2 w.*k2 d.*a3 w.*c3 w.*a4 w.*b4 w.*]

# tricky sequence of capturing
loadsgf sgf/inferior/massive-captured-missing.sgf
22 compute-inferior white
#? [.*f5 w.*g5 w.*d6 w.*e6 w.*f6 w.*e7 w.*]*


# if checks neighbours through groups, should find this dead cell
loadsgf sgf/inferior/dead-thru-group.sgf
30 compute-inferior black
#? [.*e6 d.*]

loadsgf sgf/inferior/use-captain-not-cell.sgf
31 compute-inferior black
#? [.*e6 b.*g6 b.*]


# c11 and d10 will only be found as a pre-simplicial pair
# if we iterate on the search for vulnerable cells
loadsgf sgf/inferior/iterate-vulnerable.sgf
40 compute-inferior black
#? [.*d10 b.*c11 b.*d11 d.*e11 b.*f11 b.*]


# check that the vulnerable calculations go through groups
# and edges properly
loadsgf sgf/inferior/clique-thru-groups.sgf
50 compute-inferior black
#? [.*c6 b.*e6 b.*]

loadsgf sgf/inferior/thorough-thru-edge.sgf
51 compute-inferior white
#? [.*e6 #\[d6\].*g6 b.*h7 b.*i7 b.*i8 b.*]

loadsgf sgf/inferior/adj-to-both-edges.sgf
52 compute-inferior white
#? [!.*f5.*]

loadsgf sgf/inferior/all-nbs-adj-to-one-edge.sgf
53 compute-inferior white
#? [.*e6 [b|w|d].*f6 [b|w|d].*e7 [b|w|d].*f7 [b|w|d].*]

loadsgf sgf/inferior/strange-path.sgf
54 compute-inferior black
#? [.*f3 #\[e6\].*e9 #\[e6\].*]

loadsgf sgf/inferior/dead-thru-edge.sgf
55 compute-inferior black
#? [.*e7 d.*]

loadsgf sgf/inferior/complex-presimplicial-pair.sgf
56 compute-inferior black
#? [.*e3 w.*d5 w.*]

# d4 is killed by d3, and (d5,f5) are either
# both dead or both black captured.
loadsgf sgf/inferior/complex-dead.sgf
57 compute-inferior black
#? [.*d4 #\[d3\].*[d5 db.*f5 db|d5 b.*f5 b].*]

loadsgf sgf/inferior/vulnerable-via-superset-group.sgf
58 compute-inferior black
#? [.*f8 #\[f9\].*]


# knowing about fill-in can give us more vulnerable cells
loadsgf sgf/inferior/probes-vulnerable.sgf
60 compute-inferior white
#? [.*f1 #.*g1 #.*f2 #.*]*


# checks for hand-coded patterns
boardsize 11 11
70 compute-inferior white
#? [.*j10 \!\[j9\].*i11 \!\[i10\].*]


# checks whether detects regions that are completely
# useless, both near edge and in centre
loadsgf sgf/inferior/large-dead-region.sgf
80 compute-inferior white
#? [.*c9 d.*d9 d.*c10 d.*]

loadsgf sgf/inferior/large-dead-region2.sgf
81 compute-inferior white
#? [.*e9 d.*f9 d.*e10 d.*]

loadsgf sgf/inferior/centre-dead-region1.sgf
82 compute-inferior white
#? [.*f6 d.*]

loadsgf sgf/inferior/centre-dead-region2.sgf
83 compute-inferior black
#? [.*f6 d.*]

loadsgf sgf/inferior/centre-dead-region3.sgf
84 compute-inferior black
#? [.*f8 d.*]

loadsgf sgf/inferior/edge-sealed-dead-region1.sgf
85 compute-inferior white
#? [.*b3 d.*a5 d.*]

loadsgf sgf/inferior/edge-sealed-dead-region2.sgf
86 compute-inferior white
#? [.*f8 b.*f9 d.*]

loadsgf sgf/inferior/edge-sealed-dead-region3.sgf
87 compute-inferior black
#? [.*g7 d.*g9 d.*]

loadsgf sgf/inferior/dead-region-via-one-cut.sgf
88 compute-inferior white
#? [.*e5 d.*]

loadsgf sgf/inferior/dead-region-via-two-clique.sgf
89 compute-inferior white
#? [.*e5 d.*]


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
#? [.*e3 #\[d4\].*d4 #\[e3\].*]


# Vulnerable patterns are troublesome, and cannot always be removed
# from the mustplay. Here are some weird/painful situations:

loadsgf sgf/inferior/vulnerable-dominated-cycle.sgf
100 compute-inferior black
#? [.*e5 #\[e6\].*c6 #\[c7\].*e6 \!\[d6\].*c7 \!\[d6\].*d7 #\[d6\].*]


# Combinatorial decomposition tests
# Use vc-build command to get results, as not computed by ice.

loadsgf sgf/inferior/dead-via-isolated-VC1.sgf
110 vc-build white
#? [.*b2 d.*]

loadsgf sgf/inferior/dead-via-isolated-VC2.sgf
111 vc-build white
#? [.*c2 d.*f3 b.*]

loadsgf sgf/inferior/dead-via-isolated-VC3.sgf
112 vc-build black
#? [.*a1 d.*c2 b.*]

loadsgf sgf/inferior/dead-via-isolated-VC4.sgf
113 vc-build black
#? [.*g4 d.*h4 w.*h7 w.*f9 d.*]

loadsgf sgf/inferior/double-decomposition1.sgf
114 vc-build white
#? [.*i3 b.*b4 w.*d4.*i4 d.*j5 w.*j9 w.*h11 d.*]

loadsgf sgf/inferior/decomp-with-6-groups.sgf
115 vc-build black
#? [.*i6 b.*j7 b.*k8 d.*i10 b.*]*


# A strange sort of `globally' captured/vulnerable/dead

loadsgf sgf/inferior/global-captured-and-dead.sgf
120 compute-inferior white
#? [.*j2 b.*i3 b.*g6 d.*c7 b.*d7 b.*i8 b.*i9 b.*]*


# Dead cells not identifiable via `ring' idea

loadsgf sgf/inferior/dead-not-ring01.sgf
130 compute-inferior white
#? [.*e6 d.*]

loadsgf sgf/inferior/dead-not-ring02.sgf
131 compute-inferior white
#? [.*f5 d.*]

loadsgf sgf/inferior/dead-not-ring03.sgf
132 compute-inferior black
#? [.*f5 d.*]

loadsgf sgf/inferior/dead-not-ring04.sgf
133 compute-inferior black
#? [.*e6 d.*]

loadsgf sgf/inferior/dead-not-ring05.sgf
134 compute-inferior white
#? [.*f6 d.*]

# Fails if ice-iterative-dead-regions set to false
loadsgf sgf/inferior/dead-not-ring06.sgf
135 compute-inferior black
#? [.*g4 w.*g5 d.*e7 w.*]
