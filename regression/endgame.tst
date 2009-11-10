#-----------------------------------------------------------------------------
# Tests endgame play.
#
# Note 1: Only cells that are not inferior are considered, so technically
#         not all winning responses are accepted as correct.
# Note 2: When some winning moves are much shorter/preferable/easier to
#         maintain than others, then the harder winning moves may be
#         considered failures. However, in such cases a comment must be
#         made below to indicate this is the case.
# Note 3: Expected test outcomes are based on wolve (22/39 pass).
#         Mohex with 2k rollouts per cell a bit faster, and passes
#         ~23 tests (subject to variance).
#
#-----------------------------------------------------------------------------

# Testing basic endgames where the winning move should be easily determined
# using VCs at shallow depth.

loadsgf sgf/endgame/easy-endgame01.sgf
10 genmove black
#? [d1|d2|f1|e2|b4]

# In this position, white can win quickly with c3. All other winning moves
# prolong the situation.
loadsgf sgf/endgame/easy-endgame02.sgf
11 genmove white
#? [c3]

loadsgf sgf/endgame/easy-endgame03.sgf
12 genmove white
#? [e2|e3|f3|b7|e7|f9|e10|d11]

loadsgf sgf/endgame/easy-endgame04.sgf
13 genmove white
#? [i3]


# Testing close games where the winning VC is not visible at shallow
# depth, so the game will be decided by endgame play.

loadsgf sgf/endgame/close-endgame01.sgf
20 genmove black
#? [b8|i9]*

loadsgf sgf/endgame/close-endgame02.sgf
21 genmove black
#? [h4]

loadsgf sgf/endgame/close-endgame03.sgf
22 genmove white
#? [g4|f6|h6|e7|f7|g7|f8|g8]

loadsgf sgf/endgame/close-endgame04.sgf
23 genmove white
#? [h4|g5]

loadsgf sgf/endgame/close-endgame05.sgf
24 genmove black
#? [i7]*

loadsgf sgf/endgame/close-endgame06.sgf
25 genmove black
#? [h7|e8|i8]

loadsgf sgf/endgame/close-endgame07.sgf
26 genmove black
#? [j3]

loadsgf sgf/endgame/close-endgame08.sgf
27 genmove white
#? [e3|c5|d6|b7]*

# There are some (winning) delaying moves that are omitted as valid answers
loadsgf sgf/endgame/close-endgame09.sgf
28 genmove white
#? [c9]*

# The endgame tests 29-31 are based on commented games from HexWiki.
# One can see that human play and analysis is still lacking even in this
# late stage of the game.
loadsgf sgf/endgame/close-endgame10.sgf
29 genmove white
#? [f6]*

loadsgf sgf/endgame/close-endgame11.sgf
30 genmove white
#? [d8|c9|b10]*

loadsgf sgf/endgame/close-endgame12.sgf
31 genmove black
#? [d5|f5]

# MoHex fails badly by playing the losing move c6, even though dfpn can
# easily finds a winning move for White (easiest is g3).
# Hard to solve losing moves: c6 definitely, e3 likely, etc.
loadsgf sgf/endgame/close-endgame13.sgf
32 genmove white
#? [f3|g3|c4|d4]*


# Testing randomly-generated endgame positions where our evaluation functions
# perform differently/are often incorrect.

# Game 0008-0009: all non-inferior cells are in the mustplay here
loadsgf sgf/endgame/random-endgame01.sgf
50 genmove black
#? [h4]*

# Game 0052-0053:
# Note: b c10 is losing in 0052, can make another test by doing solve_state
#       after w g9, but will be very slow
loadsgf sgf/endgame/random-endgame02.sgf
51 genmove white
#? [e5]*

# Game 0068-0069: first move (after w c2) differs, and determines outcome
loadsgf sgf/endgame/random-endgame03.sgf
52 genmove black
#? [d3|g3|d4|e4|c8]*

# Game 0090-0091: first move (after w g11) differs, and determines outcome
loadsgf sgf/endgame/random-endgame04.sgf
53 genmove black
#? [f5|f7]

# Game 0122-0123: first move (after w d5) differs, but black error is after
#                 exchange of h2 for h3
loadsgf sgf/endgame/random-endgame05a.sgf
54 genmove black
#? [h2|i2|c3|i3|j3|i4]
loadsgf sgf/endgame/random-endgame05b.sgf
55 genmove black
#? [c3|i4]*

# Game 0124-0125: black g5 is an error, gives two tests, 0125 too slow.
loadsgf sgf/endgame/random-endgame06a.sgf
56 genmove white
#? [h6|i6|h8|i8|h9|i9|e10|h10|i10]
loadsgf sgf/endgame/random-endgame06b.sgf
57 genmove black
#? [g6|e9]*

# Game 0130-0131: no errors in 0130, black c8 is really bad mistake in 0131
loadsgf sgf/endgame/random-endgame07.sgf
58 genmove black
#? [g6|e7|g7|d8]

# Game 0228-0229: first move determines winner (h7 only winning move!)
loadsgf sgf/endgame/random-endgame08.sgf
59 genmove black
#? [h7]

# Game 0374-0375:
loadsgf sgf/endgame/random-endgame09.sgf
60 genmove white
#? [e3|e4|g4|d5|e5|c6|e6|c8]

# Game 0464-0465
loadsgf sgf/endgame/random-endgame10.sgf
61 genmove white
#? [e2|f3|e5]

# Game 0594-0595
loadsgf sgf/endgame/random-endgame11.sgf
62 genmove black
#? [e5]*

# Game 0614-0615: first move after w c3 decides it
loadsgf sgf/endgame/random-endgame12.sgf
63 genmove black
#? [g4]*

# Game 0950-0951: first move after w h8 decides it
loadsgf sgf/endgame/random-endgame13.sgf
64 genmove black
#? [h5]

# Game 0954-0955: After w h8, b d9 loses; after b e8, w d9 loses
loadsgf sgf/endgame/random-endgame14a.sgf
65 genmove black
#? [k6]*
loadsgf sgf/endgame/random-endgame14b.sgf
66 genmove white
#? [e7]*

# Surprisingly, very few moves need to be considered here due to lots
# of fill-in (i.e. g3, h8, etc). I've listed all winning moves for now.
loadsgf sgf/endgame/random-endgame15.sgf
67 genmove black
#? [d6|c7|d7|h7|j7|b8|c8|d8|e8|f8|g8|i8|b9|f9|g9|h9|i9|f10|g10]

# Only two moves need to be considered when use backing-up of decomposition
# info: i4 and c3. Both are provably winning moves.
loadsgf sgf/endgame/random-endgame16.sgf
68 genmove white
#? [c3|i4]


# Testing real game positions where an AI (one or more) played a dumb
# endgame move when a simple winning sequence was available. Note that these
# are not necessarily the only winning moves, just the simplest (by far).

loadsgf sgf/endgame/simple-endgame01.sgf
80 genmove white
#? [g2]

# In this position, mohex sees an empty black mustplay after w c8, while
# wolve does not!
loadsgf sgf/endgame/simple-endgame02.sgf
81 genmove white
#? [c8]*

# Can easily win with this move
loadsgf sgf/endgame/simple-endgame03.sgf
82 genmove black
#? [e4]*

# Lots of winning moves here
loadsgf sgf/endgame/simple-endgame04.sgf
83 genmove white
#? [i5|h6|g7|f8|g8|h8|e9|g9|h9|i9|f10|i10]
