#-----------------------------------------------------------------------------
# Tests middle game play.
#
# Note 1: Expected test outcomes are based on wolve, since it is
#         deterministic (wolve passes 0-1 more tests than mohex).
# Note 2: Since these positions cannot be solved, correct answers are
#         somewhat subjective. However, as much as possible we should
#         try to ensure they are accurate.
#
#-----------------------------------------------------------------------------

# Testing how we make connections when we have a choice.

loadsgf sgf/midgame/make-connection-with-max-influence01.sgf
10 genmove black
#? [d7]*

loadsgf sgf/midgame/make-connection-with-max-influence02.sgf
11 genmove white
#? [h3]*

loadsgf sgf/midgame/make-connection-with-max-influence03.sgf
12 genmove white
#? [h8]*


# Testing how we play ladders (especially those that don't work for us).

loadsgf sgf/midgame/do-not-pursue-broken-ladder01.sgf
20 genmove white
#? [h2|j4|j8|g9]
