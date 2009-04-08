#-----------------------------------------------------------------------------
# Tests opening (and early midgame) play.
#
# Note: Expected results based on wolve. Mohex has variance, but faster
# and passes a bit more on average (~10-13)
#
#-----------------------------------------------------------------------------

# 5x5 positions

loadsgf sgf/opening/5x5a3.sgf
10 reg_genmove white
#? [b4]


# 6x6 positions

loadsgf sgf/opening/6x6a1.sgf
20 reg_genmove white
#? [b2|d2|e2|c3|d3|c4|d5]

loadsgf sgf/opening/6x6a2.sgf
21 reg_genmove white
#? [d3]*

loadsgf sgf/opening/6x6e1.sgf
22 reg_genmove white
#? [c2|e2|c3|c4|d4|d5]


# 7x7 positions

loadsgf sgf/opening/7x7.sgf
30 reg_genmove black
#? [g2|d3|e3|d4|c5|d5|b6]*

loadsgf sgf/opening/7x7a1.sgf
31 reg_genmove white
#? [d2|f2|c3|d3|e3|c4|d4|c5|d5|e5|b6|c6|e6|b7]

loadsgf sgf/opening/7x7a2.sgf
32 reg_genmove white
#? [e3|d4]*

loadsgf sgf/opening/7x7a3.sgf
33 reg_genmove white
#? [d4]*

loadsgf sgf/opening/7x7a5.sgf
34 reg_genmove white
#? [b6]

loadsgf sgf/opening/7x7b1.sgf
35 reg_genmove white
#? [d2|f2|c3|d3|e3|c4|d4|c5|d5|e5|b6|c6|e6|b7]

loadsgf sgf/opening/7x7b2.sgf
36 reg_genmove white
#? [e3]*

loadsgf sgf/opening/7x7c1.sgf
37 reg_genmove white
#? [d2|f2|c3|d3|e3|c4|d4|c5|d5|e5|b6|c6|e6|b7]

loadsgf sgf/opening/7x7d1.sgf
38 reg_genmove white
#? [c2|d2|e2|f2|c3|d3|e3|c4|d4|c5|d5|e5|b6|c6|e6|b7|d7]

loadsgf sgf/opening/7x7d2.sgf
39 reg_genmove white
#? [c5]*

loadsgf sgf/opening/7x7e1.sgf
40 reg_genmove white
#? [c2|e2|f2|c3|d3|e3|c4|d4|c5|d5|e5|b6|c6|e6]

loadsgf sgf/opening/7x7f1.sgf
41 reg_genmove white
#? [f2|d3|e3|d4|c5|d5|e5|b6|c6|e6|d7]
