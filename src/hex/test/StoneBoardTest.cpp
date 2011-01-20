//---------------------------------------------------------------------------
/** @file StoneBoardTest.cpp */
//---------------------------------------------------------------------------

#include <boost/test/auto_unit_test.hpp>

#include "SgSystem.h"
#include "StoneBoard.hpp"

using namespace benzene;

//---------------------------------------------------------------------------

namespace {

BOOST_AUTO_TEST_CASE(StoneBoard_numStones)
{
    BOOST_REQUIRE(MAX_WIDTH >= 5 && MAX_HEIGHT >= 5);

    StoneBoard b(5, 5);
    BOOST_CHECK_EQUAL(b.NumStones(), 0);

    b.PlayMove(BLACK, FIRST_CELL);
    BOOST_CHECK_EQUAL(b.NumStones(), 1);

    b.PlayMove(WHITE, HEX_CELL_A2);
    BOOST_CHECK_EQUAL(b.NumStones(), 2);

    b.SetColor(BLACK, HEX_CELL_A3);
    BOOST_CHECK_EQUAL(b.NumStones(), 2);
}

BOOST_AUTO_TEST_CASE(StoneBoard_AddRemoveSetColor)
{
    BOOST_REQUIRE(MAX_WIDTH >= 8 && MAX_HEIGHT >= 8);
    StoneBoard sb = StoneBoard(8, 8);
    bitset_t b;
    
    // test addColor
    sb.AddColor(BLACK, b);
    BOOST_CHECK_EQUAL(sb.GetBlack().count(), 2u);
    BOOST_CHECK_EQUAL(sb.GetWhite().count(), 2u);
    BOOST_CHECK(sb.GetWhite().test(WEST));
    BOOST_CHECK(sb.GetWhite().test(EAST));
    b.set(FIRST_CELL);
    b.set(HEX_CELL_A3);
    sb.AddColor(BLACK, b);
    BOOST_CHECK_EQUAL(sb.GetBlack().count(), 4u);
    BOOST_CHECK_EQUAL(sb.GetWhite().count(), 2u);
    b.reset();
    b.set(HEX_CELL_A2);
    sb.AddColor(WHITE, b);
    BOOST_CHECK_EQUAL(sb.GetBlack().count(), 4u);
    BOOST_CHECK(sb.GetBlack().test(FIRST_CELL));
    BOOST_CHECK(sb.GetBlack().test(HEX_CELL_A3));
    BOOST_CHECK_EQUAL(sb.GetWhite().count(), 3u);
    BOOST_CHECK(sb.GetWhite().test(HEX_CELL_A2));
    
    // test removeColor when nothing removed
    b.flip();
    b &= sb.Const().GetCells();
    sb.RemoveColor(WHITE, b);
    BOOST_CHECK_EQUAL(sb.GetBlack().count(), 4u);
    BOOST_CHECK_EQUAL(sb.GetWhite().count(), 3u);
    
    // test setColor with EMPTY
    sb.SetColor(EMPTY, FIRST_CELL);
    BOOST_CHECK_EQUAL(sb.GetBlack().count(), 3u);
    BOOST_CHECK(sb.GetBlack().test(HEX_CELL_A3));
    BOOST_CHECK_EQUAL(sb.GetWhite().count(), 3u);
    sb.SetColor(EMPTY, HEX_CELL_A2);
    BOOST_CHECK_EQUAL(sb.GetBlack().count(), 3u);
    BOOST_CHECK_EQUAL(sb.GetWhite().count(), 2u);
    
    // test setColor with BLACK/WHITE
    b.reset();
    b.set(FIRST_CELL);
    b.set(HEX_CELL_A4);
    sb.SetColor(WHITE, b);
    BOOST_CHECK_EQUAL(sb.GetBlack().count(), 3u);
    BOOST_CHECK(sb.GetBlack().test(HEX_CELL_A3));
    BOOST_CHECK_EQUAL(sb.GetWhite().count(), 2u);
    BOOST_CHECK(sb.GetWhite().test(FIRST_CELL));
    BOOST_CHECK(sb.GetWhite().test(HEX_CELL_A4));
    
    // test removeColor under normal conditions
    b.reset();
    b.set(FIRST_CELL);
    sb.RemoveColor(WHITE, b);
    BOOST_CHECK_EQUAL(sb.GetBlack().count(), 3u);
    BOOST_CHECK(sb.GetBlack().test(HEX_CELL_A3));
    BOOST_CHECK_EQUAL(sb.GetWhite().count(), 1u);
    BOOST_CHECK(sb.GetWhite().test(HEX_CELL_A4));
    b.set(HEX_CELL_A3);
    b.set(HEX_CELL_A4);
    sb.RemoveColor(BLACK, b);
    BOOST_CHECK_EQUAL(sb.GetBlack().count(), 2u);
    BOOST_CHECK_EQUAL(sb.GetWhite().count(), 1u);
    BOOST_CHECK(sb.GetWhite().test(HEX_CELL_A4));
}

BOOST_AUTO_TEST_CASE(StoneBoard_PlayAndUndoMoves)
{
    BOOST_REQUIRE(MAX_WIDTH >= 9 && MAX_HEIGHT >= 9);
    
    StoneBoard sb = StoneBoard(9, 9);
    BOOST_CHECK_EQUAL(sb.GetBlack().count(), 2u);
    BOOST_CHECK(sb.IsWhite(EAST));
    BOOST_CHECK(sb.IsWhite(WEST));
    BOOST_CHECK_EQUAL(sb.GetPlayed().count(), 4u);
    BOOST_CHECK(sb.IsPlayed(NORTH));
    BOOST_CHECK(sb.IsPlayed(EAST));
    
    // test PlayMove
    sb.PlayMove(BLACK, FIRST_CELL);
    BOOST_CHECK_EQUAL(sb.GetBlack().count(), 3u);
    BOOST_CHECK_EQUAL(sb.GetWhite().count(), 2u);
    BOOST_CHECK_EQUAL(sb.GetPlayed().count(), 5u);
    BOOST_CHECK(sb.IsBlack(FIRST_CELL));
    BOOST_CHECK(sb.IsPlayed(FIRST_CELL));
    sb.PlayMove(WHITE, HEX_CELL_A9);
    BOOST_CHECK_EQUAL(sb.GetBlack().count(), 3u);
    BOOST_CHECK_EQUAL(sb.GetWhite().count(), 3u);
    BOOST_CHECK_EQUAL(sb.GetPlayed().count(), 6u);
    BOOST_CHECK(sb.IsWhite(HEX_CELL_A9));
    BOOST_CHECK(sb.IsPlayed(HEX_CELL_A9));
    
    // test UndoMove
    sb.UndoMove(FIRST_CELL);
    BOOST_CHECK_EQUAL(sb.GetBlack().count(), 2u);
    BOOST_CHECK_EQUAL(sb.GetWhite().count(), 3u);
    BOOST_CHECK_EQUAL(sb.GetPlayed().count(), 5u);
    BOOST_CHECK(!sb.IsBlack(FIRST_CELL));
    BOOST_CHECK(!sb.IsPlayed(FIRST_CELL));
    sb.PlayMove(WHITE, HEX_CELL_A5);
    BOOST_CHECK_EQUAL(sb.GetBlack().count(), 2u);
    BOOST_CHECK_EQUAL(sb.GetWhite().count(), 4u);
    BOOST_CHECK_EQUAL(sb.GetPlayed().count(), 6u);
    BOOST_CHECK(sb.IsWhite(HEX_CELL_A5));
    BOOST_CHECK(sb.IsPlayed(HEX_CELL_A5));
    sb.UndoMove(HEX_CELL_A9);
    BOOST_CHECK_EQUAL(sb.GetBlack().count(), 2u);
    BOOST_CHECK_EQUAL(sb.GetWhite().count(), 3u);
    BOOST_CHECK_EQUAL(sb.GetPlayed().count(), 5u);
    BOOST_CHECK(!sb.IsWhite(HEX_CELL_A9));
    BOOST_CHECK(!sb.IsPlayed(HEX_CELL_A9));
    
    // check that RESIGN and SWAP_PIECES have no effect on board
    // status, or ability to keep playing moves, although they
    // can affect which moves are legal
    sb.StartNewGame();
    BOOST_CHECK(!sb.IsLegal(SWAP_PIECES));
    sb.PlayMove(BLACK, HEX_CELL_A5);
    BOOST_CHECK(sb.IsLegal(SWAP_PIECES));
    sb.PlayMove(WHITE, SWAP_PIECES);
    BOOST_CHECK(!sb.IsLegal(SWAP_PIECES));
    BOOST_CHECK(sb.IsBlack(HEX_CELL_A5));
    BOOST_CHECK(sb.IsLegal(RESIGN));
    BOOST_CHECK(sb.IsLegal(HEX_CELL_F6));
    sb.PlayMove(BLACK, RESIGN);
    BOOST_CHECK(!sb.IsLegal(RESIGN));
    BOOST_CHECK(!sb.IsLegal(HEX_CELL_F6));
    sb.PlayMove(WHITE, HEX_CELL_F6);
    BOOST_CHECK(sb.IsBlack(HEX_CELL_A5));
    BOOST_CHECK(sb.IsWhite(HEX_CELL_F6));
}

BOOST_AUTO_TEST_CASE(StoneBoard_RotateAndMirrorBoard)
{
    BOOST_REQUIRE(MAX_WIDTH >= 8 && MAX_HEIGHT >= 8);
    StoneBoard sb = StoneBoard(5, 6);
    
    // test rotateBoard on non-square board
    sb.PlayMove(BLACK, HEX_CELL_A5);
    sb.PlayMove(WHITE, HEX_CELL_B3);
    BOOST_CHECK_EQUAL(sb.GetBlack().count(), 3u);
    BOOST_CHECK_EQUAL(sb.GetWhite().count(), 3u);
    BOOST_CHECK_EQUAL(sb.GetPlayed().count(), 6u);
    BOOST_CHECK(sb.IsBlack(HEX_CELL_A5));
    BOOST_CHECK(sb.IsPlayed(HEX_CELL_A5));
    BOOST_CHECK(sb.IsWhite(HEX_CELL_B3));
    BOOST_CHECK(sb.IsPlayed(HEX_CELL_B3));
    sb.RotateBoard();
    BOOST_CHECK_EQUAL(sb.GetBlack().count(), 3u);
    BOOST_CHECK_EQUAL(sb.GetWhite().count(), 3u);
    BOOST_CHECK_EQUAL(sb.GetPlayed().count(), 6u);
    BOOST_CHECK(sb.IsBlack(HEX_CELL_E2));
    BOOST_CHECK(sb.IsPlayed(HEX_CELL_E2));
    BOOST_CHECK(sb.IsWhite(HEX_CELL_D4));
    BOOST_CHECK(sb.IsPlayed(HEX_CELL_D4));
    BOOST_CHECK(sb.IsBlack(NORTH));
    BOOST_CHECK(sb.IsWhite(WEST));
    BOOST_CHECK(sb.IsWhite(EAST));
    BOOST_CHECK(sb.IsPlayed(SOUTH));
    
    // test rotateBoard on square board
    sb = StoneBoard(8, 8);
    sb.PlayMove(BLACK, HEX_CELL_B2);
    sb.PlayMove(WHITE, HEX_CELL_D4);
    sb.PlayMove(BLACK, HEX_CELL_D5);
    BOOST_CHECK_EQUAL(sb.GetBlack().count(), 4u);
    BOOST_CHECK_EQUAL(sb.GetWhite().count(), 3u);
    BOOST_CHECK_EQUAL(sb.GetPlayed().count(), 7u);
    BOOST_CHECK(sb.IsBlack(HEX_CELL_D5));
    BOOST_CHECK(sb.IsPlayed(HEX_CELL_D4));
    sb.RotateBoard();
    BOOST_CHECK_EQUAL(sb.GetBlack().count(), 4u);
    BOOST_CHECK_EQUAL(sb.GetWhite().count(), 3u);
    BOOST_CHECK_EQUAL(sb.GetPlayed().count(), 7u);
    BOOST_CHECK(sb.IsBlack(HEX_CELL_E4));
    BOOST_CHECK(sb.IsBlack(HEX_CELL_G7));
    BOOST_CHECK(sb.IsWhite(HEX_CELL_E5));
    BOOST_CHECK(sb.IsPlayed(HEX_CELL_E4));
    BOOST_CHECK(sb.IsPlayed(HEX_CELL_G7));
    BOOST_CHECK(sb.IsPlayed(HEX_CELL_E5));
    BOOST_CHECK(sb.IsBlack(NORTH));
    BOOST_CHECK(sb.IsBlack(SOUTH));
    BOOST_CHECK(sb.IsWhite(EAST));
    BOOST_CHECK(sb.IsWhite(EAST));
    BOOST_CHECK(sb.IsPlayed(NORTH));
    BOOST_CHECK(sb.IsPlayed(SOUTH));
    BOOST_CHECK(sb.IsPlayed(EAST));
    BOOST_CHECK(sb.IsPlayed(WEST));
    BOOST_CHECK(!sb.IsPlayed(RESIGN));
    sb.PlayMove(WHITE, RESIGN);
    sb.RotateBoard();
    BOOST_CHECK_EQUAL(sb.GetBlack().count(), 4u);
    BOOST_CHECK_EQUAL(sb.GetWhite().count(), 3u);
    BOOST_CHECK_EQUAL(sb.GetPlayed().count(), 8u);
    BOOST_CHECK(sb.IsBlack(HEX_CELL_D5));
    BOOST_CHECK(sb.IsBlack(HEX_CELL_B2));
    BOOST_CHECK(sb.IsWhite(HEX_CELL_D4));
    BOOST_CHECK(sb.IsPlayed(HEX_CELL_D5));
    BOOST_CHECK(sb.IsPlayed(HEX_CELL_B2));
    BOOST_CHECK(sb.IsPlayed(HEX_CELL_D4));
    BOOST_CHECK(sb.IsPlayed(RESIGN));
    BOOST_CHECK(sb.IsBlack(NORTH));
    BOOST_CHECK(sb.IsBlack(SOUTH));
    BOOST_CHECK(sb.IsWhite(EAST));
    BOOST_CHECK(sb.IsWhite(WEST));
    BOOST_CHECK(sb.IsPlayed(NORTH));
    BOOST_CHECK(sb.IsPlayed(SOUTH));
    BOOST_CHECK(sb.IsPlayed(EAST));
    BOOST_CHECK(sb.IsPlayed(WEST));
    
    // test mirror on square board
    sb.MirrorBoard();
    BOOST_CHECK_EQUAL(sb.GetBlack().count(), 4u);
    BOOST_CHECK_EQUAL(sb.GetWhite().count(), 3u);
    BOOST_CHECK_EQUAL(sb.GetPlayed().count(), 8u);
    BOOST_CHECK(sb.IsBlack(HEX_CELL_E4));
    BOOST_CHECK(sb.IsBlack(HEX_CELL_B2));
    BOOST_CHECK(sb.IsWhite(HEX_CELL_D4));
    BOOST_CHECK(sb.IsPlayed(HEX_CELL_E4));
    BOOST_CHECK(sb.IsPlayed(HEX_CELL_B2));
    BOOST_CHECK(sb.IsPlayed(HEX_CELL_D4));
    BOOST_CHECK(sb.IsPlayed(RESIGN));
    BOOST_CHECK(sb.IsWhite(NORTH));
    BOOST_CHECK(sb.IsWhite(SOUTH));
    BOOST_CHECK(sb.IsBlack(EAST));
    BOOST_CHECK(sb.IsBlack(WEST));
    BOOST_CHECK(sb.IsPlayed(NORTH));
    BOOST_CHECK(sb.IsPlayed(SOUTH));
    BOOST_CHECK(sb.IsPlayed(EAST));
    BOOST_CHECK(sb.IsPlayed(WEST));
    sb.PlayMove(WHITE, HEX_CELL_F2);
    sb.MirrorBoard();
    BOOST_CHECK_EQUAL(sb.GetBlack().count(), 4u);
    BOOST_CHECK_EQUAL(sb.GetWhite().count(), 4u);
    BOOST_CHECK_EQUAL(sb.GetPlayed().count(), 9u);
    BOOST_CHECK(sb.IsBlack(HEX_CELL_D5));
    BOOST_CHECK(sb.IsBlack(HEX_CELL_B2));
    BOOST_CHECK(sb.IsWhite(HEX_CELL_D4));
    BOOST_CHECK(sb.IsWhite(HEX_CELL_B6));
    BOOST_CHECK(sb.IsPlayed(HEX_CELL_D5));
    BOOST_CHECK(sb.IsPlayed(HEX_CELL_B2));
    BOOST_CHECK(sb.IsPlayed(HEX_CELL_D4));
    BOOST_CHECK(sb.IsPlayed(HEX_CELL_B6));
    BOOST_CHECK(sb.IsPlayed(RESIGN));
    BOOST_CHECK(sb.IsBlack(NORTH));
    BOOST_CHECK(sb.IsBlack(SOUTH));
    BOOST_CHECK(sb.IsWhite(EAST));
    BOOST_CHECK(sb.IsWhite(WEST));
    BOOST_CHECK(sb.IsPlayed(NORTH));
    BOOST_CHECK(sb.IsPlayed(SOUTH));
    BOOST_CHECK(sb.IsPlayed(EAST));
    BOOST_CHECK(sb.IsPlayed(WEST));
}

BOOST_AUTO_TEST_CASE(StoneBoard_SelfRotation)
{
    StoneBoard brd(3, 3);
    BOOST_CHECK(brd.IsSelfRotation());
    brd.PlayMove(BLACK, HEX_CELL_A1);
    BOOST_CHECK(!brd.IsSelfRotation());
    brd.PlayMove(BLACK, HEX_CELL_C3);
    BOOST_CHECK(brd.IsSelfRotation());
    brd.PlayMove(WHITE, HEX_CELL_B2);
    BOOST_CHECK(brd.IsSelfRotation());
    brd.PlayMove(WHITE, HEX_CELL_A2);
    BOOST_CHECK(!brd.IsSelfRotation());

    brd = StoneBoard(9, 9);
    brd.PlayMove(BLACK, HEX_CELL_E5);
    BOOST_CHECK(brd.IsSelfRotation());
}

BOOST_AUTO_TEST_CASE(StoneBoard_Hash)
{
    BOOST_REQUIRE(MAX_WIDTH >= 5 && MAX_HEIGHT >= 5);
    StoneBoard sb = StoneBoard(5, 5);

    SgHashCode h1,h2;
    h1 = sb.Hash();

    // check that playmove modifies the hash
    sb.PlayMove(BLACK, HEX_CELL_A1);
    sb.PlayMove(WHITE, HEX_CELL_A2);
    sb.PlayMove(BLACK, HEX_CELL_A3);
    sb.PlayMove(WHITE, HEX_CELL_A4);
    h2 = sb.Hash();
    BOOST_CHECK(h1 != h2);
    
    // addColor does not modify hash
    bitset_t bs;
    bs.set(HEX_CELL_A5);
    sb.AddColor(BLACK, bs);
    BOOST_CHECK(h2 == sb.Hash());

    // removeColor (even of a played move)
    // does not modify hash. 
    bs.reset();
    bs.set(HEX_CELL_A4);
    sb.RemoveColor(WHITE, bs);
    BOOST_CHECK(h2 == sb.Hash());

    // set color does not modify hash.
    bs.reset();
    bs.set(HEX_CELL_A3);
    sb.SetColor(BLACK, bs);
    BOOST_CHECK(h2 == sb.Hash());
}

BOOST_AUTO_TEST_CASE(StoneBoard_WhoseTurn)
{
    BOOST_REQUIRE(MAX_WIDTH >= 5 && MAX_HEIGHT >= 5);
    StoneBoard sb = StoneBoard(5, 5);

    BOOST_CHECK_EQUAL(sb.WhoseTurn(), FIRST_TO_PLAY);

    sb.PlayMove(FIRST_TO_PLAY, FIRST_CELL);
    BOOST_CHECK_EQUAL(sb.WhoseTurn(), !FIRST_TO_PLAY);
    
    sb.PlayMove(sb.WhoseTurn(), HEX_CELL_A2);
    BOOST_CHECK_EQUAL(sb.WhoseTurn(), FIRST_TO_PLAY);

    sb.PlayMove(sb.WhoseTurn(), HEX_CELL_A3);
    BOOST_CHECK_EQUAL(sb.WhoseTurn(), !FIRST_TO_PLAY);

    sb.PlayMove(sb.WhoseTurn(), HEX_CELL_A4);
    BOOST_CHECK_EQUAL(sb.WhoseTurn(), FIRST_TO_PLAY);

    // check that swap is handled properly
    sb.StartNewGame();
    BOOST_CHECK_EQUAL(sb.WhoseTurn(), FIRST_TO_PLAY);

    sb.PlayMove(FIRST_TO_PLAY, FIRST_CELL);
    BOOST_CHECK_EQUAL(sb.WhoseTurn(), !FIRST_TO_PLAY);

    sb.PlayMove(!FIRST_TO_PLAY, SWAP_PIECES);
    BOOST_CHECK_EQUAL(sb.WhoseTurn(), !FIRST_TO_PLAY);
    
    sb.PlayMove(sb.WhoseTurn(), HEX_CELL_A2);
    BOOST_CHECK_EQUAL(sb.WhoseTurn(), FIRST_TO_PLAY);
    
    sb.PlayMove(sb.WhoseTurn(), HEX_CELL_A3);
    BOOST_CHECK_EQUAL(sb.WhoseTurn(), !FIRST_TO_PLAY);

    sb.PlayMove(sb.WhoseTurn(), HEX_CELL_A4);
    BOOST_CHECK_EQUAL(sb.WhoseTurn(), FIRST_TO_PLAY);
}

BOOST_AUTO_TEST_CASE(StoneBoard_IsStandardPosition)
{
    StoneBoard brd(5, 5);
    BOOST_CHECK(brd.IsStandardPosition());
    brd.PlayMove(BLACK, HEX_CELL_A1);
    BOOST_CHECK(brd.IsStandardPosition());
    brd.PlayMove(WHITE, HEX_CELL_A2);
    BOOST_CHECK(brd.IsStandardPosition());
    brd.PlayMove(WHITE, HEX_CELL_A3);
    BOOST_CHECK(!brd.IsStandardPosition());
    brd.PlayMove(BLACK, HEX_CELL_A4);
    BOOST_CHECK(brd.IsStandardPosition());
    brd.PlayMove(BLACK, HEX_CELL_A5);
    BOOST_CHECK(brd.IsStandardPosition());
}

//---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(StoneBoard_SetStateString)
{
    std::string str(". . w"
                     " B b\n ."
                      ". W\tB   ");
    StoneBoard brd(3, 3, str);
    BOOST_CHECK(brd.IsEmpty(HEX_CELL_A1));
    BOOST_CHECK(brd.IsEmpty(HEX_CELL_B1));
    BOOST_CHECK(brd.IsWhite(HEX_CELL_C1));
    BOOST_CHECK(!brd.IsPlayed(HEX_CELL_C1));
    BOOST_CHECK(brd.IsBlack(HEX_CELL_A2));
    BOOST_CHECK(brd.IsPlayed(HEX_CELL_A2));
    BOOST_CHECK(brd.IsBlack(HEX_CELL_B2));
    BOOST_CHECK(!brd.IsPlayed(HEX_CELL_B2));
    BOOST_CHECK(brd.IsEmpty(HEX_CELL_C2));
    BOOST_CHECK(brd.IsEmpty(HEX_CELL_A3));
    BOOST_CHECK(brd.IsWhite(HEX_CELL_B3));
    BOOST_CHECK(brd.IsPlayed(HEX_CELL_B3));
    BOOST_CHECK(brd.IsBlack(HEX_CELL_C3));
    BOOST_CHECK(brd.IsPlayed(HEX_CELL_C3));
}

//---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(StoneBoard_BoardID)
{
    BOOST_REQUIRE(MAX_WIDTH >= 2 && MAX_HEIGHT >= 2);

    // check each color is encoded/decoded correctly
    // on a 1x1 board. 
    for (ColorIterator color; color; ++color) 
    {
        StoneBoard b1(1, 1);
        StoneBoard b2(1, 1);

        if (*color != EMPTY)
            b1.PlayMove(*color, HEX_CELL_A1);

        BoardID id = b1.GetBoardID();
        BOOST_CHECK_EQUAL(id.size(), 1u);
    
        b2.SetPosition(id);
        BOOST_CHECK(b1 == b2);
    }

    // check a 5x3 state
    {
        std::string str("B..W."
                         ".WB.."
                          "BW..W");
        StoneBoard b1(5, 3, str);
        StoneBoard b2(5, 3);
        BoardID id = b1.GetBoardID();
        BOOST_CHECK_EQUAL(id.size(), 4u);
        b2.SetPosition(id);
        BOOST_CHECK(b1 == b2);
    }
}

}

//---------------------------------------------------------------------------
