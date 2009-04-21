noinst_LIBRARIES = libhex.a

libhex_a_SOURCES = \
BoardUtils.cpp \
ConstBoard.cpp \
Game.cpp \
GraphUtils.cpp \
GroupBoard.cpp \
HandCodedPattern.cpp\
HashedPatternSet.cpp\
HexAbSearch.cpp \
HexBoard.cpp \
HexEval.cpp \
HexHtpEngine.cpp \
HexPoint.cpp \
HexProgram.cpp \
HexProp.cpp \
HexSgUtil.cpp \
ICEngine.cpp \
IcePatternSet.cpp \
InferiorCells.cpp \
OpeningBook.cpp \
Pattern.cpp\
PatternBoard.cpp\
PlayerUtils.cpp \
Resistance.cpp \
RingGodel.cpp \
SequenceHash.cpp \
StoneBoard.cpp \
TwoDistance.cpp \
VC.cpp \
VCBuilder.cpp \
VCList.cpp \
VCPattern.cpp \
VCSet.cpp \
VCUtils.cpp \
ZobristHash.cpp

noinst_HEADERS = \
BitsetIterator.hpp \
BoardIterator.hpp \
BoardUtils.hpp \
ConstBoard.hpp \
Game.hpp \
GraphUtils.hpp \
GroupBoard.hpp \
HandCodedPattern.hpp \
HashedPatternSet.hpp \
Hex.hpp \
HexAbSearch.hpp \
HexAssert.hpp \
HexBoard.hpp \
HexColor.hpp \
HexEval.hpp \
HexHtpEngine.hpp \
HexPlayer.hpp \
HexPoint.hpp \
HexPoints11x11.hpp \
HexPoints13x13.hpp \
HexPoints14x14.hpp \
HexPoints19x19.hpp \
HexProgram.hpp \
HexProp.hpp \
HexSgUtil.hpp \
ICEngine.hpp \
IcePatternSet.hpp \
InferiorCells.hpp \
Move.hpp \
OpeningBook.hpp \
PatternBoard.hpp \
Pattern.hpp \
PlayerUtils.hpp \
Resistance.hpp \
RingGodel.hpp \
SearchedState.hpp \
SequenceHash.hpp \
StoneBoard.hpp \
TransTable.hpp \
TwoDistance.hpp \
VC.hpp \
VCBuilder.hpp \
VCList.hpp \
VCPattern.hpp \
VCSet.hpp \
VCUtils.hpp \
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
