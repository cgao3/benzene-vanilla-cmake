noinst_LIBRARIES = libhex.a

libhex_a_SOURCES = \
BenzenePlayer.cpp \
BoardUtil.cpp \
ConstBoard.cpp \
Decompositions.cpp \
EndgameUtil.cpp \
Game.cpp \
GraphUtil.cpp \
Groups.cpp \
HandCodedPattern.cpp\
HashedPatternSet.cpp\
HexAbSearch.cpp \
HexBoard.cpp \
HexEnvironment.cpp \
HexEval.cpp \
HexHtpEngine.cpp \
HexPoint.cpp \
HexProp.cpp \
HexSgUtil.cpp \
ICEngine.cpp \
IcePatternSet.cpp \
InferiorCells.cpp \
NeighborTracker.cpp \
Pattern.cpp\
PatternState.cpp\
Resistance.cpp \
RingGodel.cpp \
SequenceHash.cpp \
StoneBoard.cpp \
TwoDistance.cpp \
VC.cpp \
VCBuilder.cpp \
VCCommands.cpp \
VCList.cpp \
VCPattern.cpp \
VCSet.cpp \
VCUtil.cpp \
ZobristHash.cpp

noinst_HEADERS = \
BenzenePlayer.hpp \
BitsetIterator.hpp \
BoardIterator.hpp \
BoardUtil.hpp \
ConstBoard.hpp \
Decompositions.hpp \
EndgameUtil.hpp \
Game.hpp \
GraphUtil.hpp \
Groups.hpp \
HandCodedPattern.hpp \
HashedPatternSet.hpp \
Hex.hpp \
HexAbSearch.hpp \
HexBoard.hpp \
HexColor.hpp \
HexEnvironment.hpp \
HexEval.hpp \
HexHtpEngine.hpp \
HexModState.hpp \
HexPlayer.hpp \
HexPoint.hpp \
HexPoints11x11.hpp \
HexPoints13x13.hpp \
HexPoints14x14.hpp \
HexPoints19x19.hpp \
HexProp.hpp \
HexSgUtil.hpp \
HexState.hpp \
HexStateAssertRestored.hpp \
ICEngine.hpp \
IcePatternSet.hpp \
InferiorCells.hpp \
NeighborTracker.hpp \
Move.hpp \
PatternState.hpp \
Pattern.hpp \
Resistance.hpp \
RingGodel.hpp \
SearchedState.hpp \
SequenceHash.hpp \
StateDB.hpp \
StoneBoard.hpp \
TwoDistance.hpp \
VC.hpp \
VCBuilder.hpp \
VCCommands.hpp \
VCList.hpp \
VCPattern.hpp \
VCSet.hpp \
VCUtil.hpp \
ZobristHashes.hpp \
ZobristHash.hpp

libhex_a_CPPFLAGS = \
$(BOOST_CPPFLAGS) \
-DABS_TOP_SRCDIR='"@abs_top_srcdir@"' \
-DDATADIR='"$(pkgdatadir)"' \
-I$(FUEGO_ROOT)/gtpengine \
-I$(FUEGO_ROOT)/smartgame \
-I@top_srcdir@/src/ \
-I@top_srcdir@/src/util

DISTCLEANFILES = *~
