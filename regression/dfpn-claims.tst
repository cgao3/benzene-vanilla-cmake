#-----------------------------------------------------------------------------
# Solves the b7 opening on 8x8 without and with claims
#-----------------------------------------------------------------------------

boardsize 8 8
play b b7

dfpn-solve-state w
#? [black]

dfpn-clear-tt

dfpn-claims sgf/claims/8-b7.sgf b
dfpn-solve-state w
#? [black]
