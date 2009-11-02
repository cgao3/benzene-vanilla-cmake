#-----------------------------------------------------------------------------
# Tests middle game play.
#
# Note 1: Expected test outcomes are based on wolve, since it is
#         deterministic (wolve passes 0-1 more tests than mohex).
# Note 2: Since most of these positions cannot be solved, correct
#         answers are subjective. However, as much as possible we
#         should try to ensure they are accurate.
#
#-----------------------------------------------------------------------------

# Testing how we make connections when we have a choice.

# d7 is the first winning move found by dfpn. h7 was also confirmed...
# there may be others.
loadsgf sgf/midgame/make-connection-with-max-influence01.sgf
10 genmove black
#? [d7|h7]*

# Solvers still cannot figure out this position. However, g5 is known
# to be a losing move, and h3 is known to be winning.
loadsgf sgf/midgame/make-connection-with-max-influence02.sgf
11 genmove white
#? [h3]*

# Not completely solved, but only the following moves are NOT proven losses,
# nor inferior cells: i2, j6, h-j8, h-j9, c-i10, e-h11.
loadsgf sgf/midgame/make-connection-with-max-influence03.sgf
12 genmove white
#? [i2|j6|h8|h9]
