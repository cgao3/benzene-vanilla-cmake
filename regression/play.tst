#-----------------------------------------------------------------------------
# play.tst
#
# Play a couple of quick moves as wolve and as mohex. 
#
# This test is mainly for checking that no assertion triggers.
#-----------------------------------------------------------------------------

boardsize 6 6
vc-default-max-ors 3
vc-semi-soft-limit 10
vc-full-soft-limit 20
player-name wolve
wolve-max-depth 1

10 genmove b
#? [[a-k]([1-9]|1[0-1])]

20 genmove w
#? [[a-k]([1-9]|1[0-1])]

30 genmove b
#? [[a-k]([1-9]|1[0-1])]

40 genmove w
#? [[a-k]([1-9]|1[0-1])]

#-----------------------------------------------------------------------------

boardsize 6 6
player-name mohex
uct-scale-num-games-to-size false
uct-num-games 1000

110 genmove b
#? [[a-k]([1-9]|1[0-1])]

120 genmove w
#? [[a-k]([1-9]|1[0-1])]

130 genmove b
#? [[a-k]([1-9]|1[0-1])]

140 genmove w
#? [[a-k]([1-9]|1[0-1])]

