//---------------------------------------------------------------------------
/** @file
 */
//---------------------------------------------------------------------------

#include <boost/test/auto_unit_test.hpp>

#include "StoneBoard.hpp"

using namespace benzene;

//---------------------------------------------------------------------------

namespace {

BOOST_AUTO_TEST_CASE(StoneBoard_numStones)
{
    BOOST_REQUIRE(MAX_WIDTH >= 5 && MAX_HEIGHT >= 5);

    StoneBoard b(5, 5);
    BOOST_CHECK_EQUAL(b.numStones(), 0);

    b.playMove(BLACK, FIRST_CELL);
    BOOST_CHECK_EQUAL(b.numStones(), 1);

    b.playMove(WHITE, HEX_CELL_A2);
    BOOST_CHECK_EQUAL(b.numStones(), 2);

    b.setColor(BLACK, HEX_CELL_A3);
    BOOST_CHECK_EQUAL(b.numStones(), 2);
}

BOOST_AUTO_TEST_CASE(StoneBoard_AddRemoveSetColor)
{
    BOOST_REQUIRE(MAX_WIDTH >= 8 && MAX_HEIGHT >= 8);
    StoneBoard sb = StoneBoard(8, 8);
    bitset_t b;
    
    // test addColor
    sb.addColor(BLACK, b);
    BOOST_CHECK_EQUAL(sb.getBlack().count(), 2u);
    BOOST_CHECK_EQUAL(sb.getWhite().count(), 2u);
    BOOST_CHECK(sb.getWhite().test(WEST));
    BOOST_CHECK(sb.getWhite().test(EAST));
    b.set(FIRST_CELL);
    b.set(HEX_CELL_A3);
    sb.addColor(BLACK, b);
    BOOST_CHECK_EQUAL(sb.getBlack().count(), 4u);
    BOOST_CHECK_EQUAL(sb.getWhite().count(), 2u);
    b.reset();
    b.set(HEX_CELL_A2);
    sb.addColor(WHITE, b);
    BOOST_CHECK_EQUAL(sb.getBlack().count(), 4u);
    BOOST_CHECK(sb.getBlack().test(FIRST_CELL));
    BOOST_CHECK(sb.getBlack().test(HEX_CELL_A3));
    BOOST_CHECK_EQUAL(sb.getWhite().count(), 3u);
    BOOST_CHECK(sb.getWhite().test(HEX_CELL_A2));
    
    // test removeColor when nothing removed
    b.flip();
    b &= sb.Const().getCells();
    sb.removeColor(WHITE, b);
    BOOST_CHECK_EQUAL(sb.getBlack().count(), 4u);
    BOOST_CHECK_EQUAL(sb.getWhite().count(), 3u);
    
    // test setColor with EMPTY
    sb.setColor(EMPTY, FIRST_CELL);
    BOOST_CHECK_EQUAL(sb.getBlack().count(), 3u);
    BOOST_CHECK(sb.getBlack().test(HEX_CELL_A3));
    BOOST_CHECK_EQUAL(sb.getWhite().count(), 3u);
    sb.setColor(EMPTY, HEX_CELL_A2);
    BOOST_CHECK_EQUAL(sb.getBlack().count(), 3u);
    BOOST_CHECK_EQUAL(sb.getWhite().count(), 2u);
    
    // test setColor with BLACK/WHITE
    b.reset();
    b.set(FIRST_CELL);
    b.set(HEX_CELL_A4);
    sb.setColor(WHITE, b);
    BOOST_CHECK_EQUAL(sb.getBlack().count(), 3u);
    BOOST_CHECK(sb.getBlack().test(HEX_CELL_A3));
    BOOST_CHECK_EQUAL(sb.getWhite().count(), 2u);
    BOOST_CHECK(sb.getWhite().test(FIRST_CELL));
    BOOST_CHECK(sb.getWhite().test(HEX_CELL_A4));
    
    // test removeColor under normal conditions
    b.reset();
    b.set(FIRST_CELL);
    sb.removeColor(WHITE, b);
    BOOST_CHECK_EQUAL(sb.getBlack().count(), 3u);
    BOOST_CHECK(sb.getBlack().test(HEX_CELL_A3));
    BOOST_CHECK_EQUAL(sb.getWhite().count(), 1u);
    BOOST_CHECK(sb.getWhite().test(HEX_CELL_A4));
    b.set(HEX_CELL_A3);
    b.set(HEX_CELL_A4);
    sb.removeColor(BLACK, b);
    BOOST_CHECK_EQUAL(sb.getBlack().count(), 2u);
    BOOST_CHECK_EQUAL(sb.getWhite().count(), 1u);
    BOOST_CHECK(sb.getWhite().test(HEX_CELL_A4));
}

BOOST_AUTO_TEST_CASE(StoneBoard_PlayAndUndoMoves)
{
    BOOST_REQUIRE(MAX_WIDTH >= 9 && MAX_HEIGHT >= 9);
    
    StoneBoard sb = StoneBoard(9, 9);
    BOOST_CHECK_EQUAL(sb.getBlack().count(), 2u);
    BOOST_CHECK(sb.isWhite(EAST));
    BOOST_CHECK(sb.isWhite(WEST));
    BOOST_CHECK_EQUAL(sb.getPlayed().count(), 4u);
    BOOST_CHECK(sb.isPlayed(NORTH));
    BOOST_CHECK(sb.isPlayed(EAST));
    
    // test playMove
    sb.playMove(BLACK, FIRST_CELL);
    BOOST_CHECK_EQUAL(sb.getBlack().count(), 3u);
    BOOST_CHECK_EQUAL(sb.getWhite().count(), 2u);
    BOOST_CHECK_EQUAL(sb.getPlayed().count(), 5u);
    BOOST_CHECK(sb.isBlack(FIRST_CELL));
    BOOST_CHECK(sb.isPlayed(FIRST_CELL));
    sb.playMove(WHITE, HEX_CELL_A9);
    BOOST_CHECK_EQUAL(sb.getBlack().count(), 3u);
    BOOST_CHECK_EQUAL(sb.getWhite().count(), 3u);
    BOOST_CHECK_EQUAL(sb.getPlayed().count(), 6u);
    BOOST_CHECK(sb.isWhite(HEX_CELL_A9));
    BOOST_CHECK(sb.isPlayed(HEX_CELL_A9));
    
    // test undoMove
    sb.undoMove(FIRST_CELL);
    BOOST_CHECK_EQUAL(sb.getBlack().count(), 2u);
    BOOST_CHECK_EQUAL(sb.getWhite().count(), 3u);
    BOOST_CHECK_EQUAL(sb.getPlayed().count(), 5u);
    BOOST_CHECK(!sb.isBlack(FIRST_CELL));
    BOOST_CHECK(!sb.isPlayed(FIRST_CELL));
    sb.playMove(WHITE, HEX_CELL_A5);
    BOOST_CHECK_EQUAL(sb.getBlack().count(), 2u);
    BOOST_CHECK_EQUAL(sb.getWhite().count(), 4u);
    BOOST_CHECK_EQUAL(sb.getPlayed().count(), 6u);
    BOOST_CHECK(sb.isWhite(HEX_CELL_A5));
    BOOST_CHECK(sb.isPlayed(HEX_CELL_A5));
    sb.undoMove(HEX_CELL_A9);
    BOOST_CHECK_EQUAL(sb.getBlack().count(), 2u);
    BOOST_CHECK_EQUAL(sb.getWhite().count(), 3u);
    BOOST_CHECK_EQUAL(sb.getPlayed().count(), 5u);
    BOOST_CHECK(!sb.isWhite(HEX_CELL_A9));
    BOOST_CHECK(!sb.isPlayed(HEX_CELL_A9));
    
    // check that RESIGN and SWAP_PIECES have no effect on board
    // status, or ability to keep playing moves, although they
    // can affect which moves are legal
    sb.startNewGame();
    BOOST_CHECK(!sb.isLegal(SWAP_PIECES));
    sb.playMove(BLACK, HEX_CELL_A5);
    BOOST_CHECK(sb.isLegal(SWAP_PIECES));
    sb.playMove(WHITE, SWAP_PIECES);
    BOOST_CHECK(!sb.isLegal(SWAP_PIECES));
    BOOST_CHECK(sb.isBlack(HEX_CELL_A5));
    BOOST_CHECK(sb.isLegal(RESIGN));
    BOOST_CHECK(sb.isLegal(HEX_CELL_F6));
    sb.playMove(BLACK, RESIGN);
    BOOST_CHECK(!sb.isLegal(RESIGN));
    BOOST_CHECK(!sb.isLegal(HEX_CELL_F6));
    sb.playMove(WHITE, HEX_CELL_F6);
    BOOST_CHECK(sb.isBlack(HEX_CELL_A5));
    BOOST_CHECK(sb.isWhite(HEX_CELL_F6));
}

BOOST_AUTO_TEST_CASE(StoneBoard_RotateAndMirrorBoard)
{
    BOOST_REQUIRE(MAX_WIDTH >= 8 && MAX_HEIGHT >= 8);
    StoneBoard sb = StoneBoard(5, 6);
    
    // test rotateBoard on non-square board
    sb.playMove(BLACK, HEX_CELL_A5);
    sb.playMove(WHITE, HEX_CELL_B3);
    BOOST_CHECK_EQUAL(sb.getBlack().count(), 3u);
    BOOST_CHECK_EQUAL(sb.getWhite().count(), 3u);
    BOOST_CHECK_EQUAL(sb.getPlayed().count(), 6u);
    BOOST_CHECK(sb.isBlack(HEX_CELL_A5));
    BOOST_CHECK(sb.isPlayed(HEX_CELL_A5));
    BOOST_CHECK(sb.isWhite(HEX_CELL_B3));
    BOOST_CHECK(sb.isPlayed(HEX_CELL_B3));
    sb.rotateBoard();
    BOOST_CHECK_EQUAL(sb.getBlack().count(), 3u);
    BOOST_CHECK_EQUAL(sb.getWhite().count(), 3u);
    BOOST_CHECK_EQUAL(sb.getPlayed().count(), 6u);
    BOOST_CHECK(sb.isBlack(HEX_CELL_E2));
    BOOST_CHECK(sb.isPlayed(HEX_CELL_E2));
    BOOST_CHECK(sb.isWhite(HEX_CELL_D4));
    BOOST_CHECK(sb.isPlayed(HEX_CELL_D4));
    BOOST_CHECK(sb.isBlack(NORTH));
    BOOST_CHECK(sb.isWhite(WEST));
    BOOST_CHECK(sb.isWhite(EAST));
    BOOST_CHECK(sb.isPlayed(SOUTH));
    
    // test rotateBoard on square board
    sb = StoneBoard(8, 8);
    sb.playMove(BLACK, HEX_CELL_B2);
    sb.playMove(WHITE, HEX_CELL_D4);
    sb.playMove(BLACK, HEX_CELL_D5);
    BOOST_CHECK_EQUAL(sb.getBlack().count(), 4u);
    BOOST_CHECK_EQUAL(sb.getWhite().count(), 3u);
    BOOST_CHECK_EQUAL(sb.getPlayed().count(), 7u);
    BOOST_CHECK(sb.isBlack(HEX_CELL_D5));
    BOOST_CHECK(sb.isPlayed(HEX_CELL_D4));
    sb.rotateBoard();
    BOOST_CHECK_EQUAL(sb.getBlack().count(), 4u);
    BOOST_CHECK_EQUAL(sb.getWhite().count(), 3u);
    BOOST_CHECK_EQUAL(sb.getPlayed().count(), 7u);
    BOOST_CHECK(sb.isBlack(HEX_CELL_E4));
    BOOST_CHECK(sb.isBlack(HEX_CELL_G7));
    BOOST_CHECK(sb.isWhite(HEX_CELL_E5));
    BOOST_CHECK(sb.isPlayed(HEX_CELL_E4));
    BOOST_CHECK(sb.isPlayed(HEX_CELL_G7));
    BOOST_CHECK(sb.isPlayed(HEX_CELL_E5));
    BOOST_CHECK(sb.isBlack(NORTH));
    BOOST_CHECK(sb.isBlack(SOUTH));
    BOOST_CHECK(sb.isWhite(EAST));
    BOOST_CHECK(sb.isWhite(EAST));
    BOOST_CHECK(sb.isPlayed(NORTH));
    BOOST_CHECK(sb.isPlayed(SOUTH));
    BOOST_CHECK(sb.isPlayed(EAST));
    BOOST_CHECK(sb.isPlayed(WEST));
    BOOST_CHECK(!sb.isPlayed(RESIGN));
    sb.playMove(WHITE, RESIGN);
    sb.rotateBoard();
    BOOST_CHECK_EQUAL(sb.getBlack().count(), 4u);
    BOOST_CHECK_EQUAL(sb.getWhite().count(), 3u);
    BOOST_CHECK_EQUAL(sb.getPlayed().count(), 8u);
    BOOST_CHECK(sb.isBlack(HEX_CELL_D5));
    BOOST_CHECK(sb.isBlack(HEX_CELL_B2));
    BOOST_CHECK(sb.isWhite(HEX_CELL_D4));
    BOOST_CHECK(sb.isPlayed(HEX_CELL_D5));
    BOOST_CHECK(sb.isPlayed(HEX_CELL_B2));
    BOOST_CHECK(sb.isPlayed(HEX_CELL_D4));
    BOOST_CHECK(sb.isPlayed(RESIGN));
    BOOST_CHECK(sb.isBlack(NORTH));
    BOOST_CHECK(sb.isBlack(SOUTH));
    BOOST_CHECK(sb.isWhite(EAST));
    BOOST_CHECK(sb.isWhite(WEST));
    BOOST_CHECK(sb.isPlayed(NORTH));
    BOOST_CHECK(sb.isPlayed(SOUTH));
    BOOST_CHECK(sb.isPlayed(EAST));
    BOOST_CHECK(sb.isPlayed(WEST));
    
    // test mirror on square board
    sb.mirrorBoard();
    BOOST_CHECK_EQUAL(sb.getBlack().count(), 4u);
    BOOST_CHECK_EQUAL(sb.getWhite().count(), 3u);
    BOOST_CHECK_EQUAL(sb.getPlayed().count(), 8u);
    BOOST_CHECK(sb.isBlack(HEX_CELL_E4));
    BOOST_CHECK(sb.isBlack(HEX_CELL_B2));
    BOOST_CHECK(sb.isWhite(HEX_CELL_D4));
    BOOST_CHECK(sb.isPlayed(HEX_CELL_E4));
    BOOST_CHECK(sb.isPlayed(HEX_CELL_B2));
    BOOST_CHECK(sb.isPlayed(HEX_CELL_D4));
    BOOST_CHECK(sb.isPlayed(RESIGN));
    BOOST_CHECK(sb.isWhite(NORTH));
    BOOST_CHECK(sb.isWhite(SOUTH));
    BOOST_CHECK(sb.isBlack(EAST));
    BOOST_CHECK(sb.isBlack(WEST));
    BOOST_CHECK(sb.isPlayed(NORTH));
    BOOST_CHECK(sb.isPlayed(SOUTH));
    BOOST_CHECK(sb.isPlayed(EAST));
    BOOST_CHECK(sb.isPlayed(WEST));
    sb.playMove(WHITE, HEX_CELL_F2);
    sb.mirrorBoard();
    BOOST_CHECK_EQUAL(sb.getBlack().count(), 4u);
    BOOST_CHECK_EQUAL(sb.getWhite().count(), 4u);
    BOOST_CHECK_EQUAL(sb.getPlayed().count(), 9u);
    BOOST_CHECK(sb.isBlack(HEX_CELL_D5));
    BOOST_CHECK(sb.isBlack(HEX_CELL_B2));
    BOOST_CHECK(sb.isWhite(HEX_CELL_D4));
    BOOST_CHECK(sb.isWhite(HEX_CELL_B6));
    BOOST_CHECK(sb.isPlayed(HEX_CELL_D5));
    BOOST_CHECK(sb.isPlayed(HEX_CELL_B2));
    BOOST_CHECK(sb.isPlayed(HEX_CELL_D4));
    BOOST_CHECK(sb.isPlayed(HEX_CELL_B6));
    BOOST_CHECK(sb.isPlayed(RESIGN));
    BOOST_CHECK(sb.isBlack(NORTH));
    BOOST_CHECK(sb.isBlack(SOUTH));
    BOOST_CHECK(sb.isWhite(EAST));
    BOOST_CHECK(sb.isWhite(WEST));
    BOOST_CHECK(sb.isPlayed(NORTH));
    BOOST_CHECK(sb.isPlayed(SOUTH));
    BOOST_CHECK(sb.isPlayed(EAST));
    BOOST_CHECK(sb.isPlayed(WEST));
}

BOOST_AUTO_TEST_CASE(StoneBoard_Hash)
{
    BOOST_REQUIRE(MAX_WIDTH >= 5 && MAX_HEIGHT >= 5);
    StoneBoard sb = StoneBoard(5, 5);

    hash_t h1,h2;
    h1 = sb.Hash();

    // check that playmove modifies the hash
    sb.playMove(BLACK, HEX_CELL_A1);
    sb.playMove(WHITE, HEX_CELL_A2);
    sb.playMove(BLACK, HEX_CELL_A3);
    sb.playMove(WHITE, HEX_CELL_A4);
    h2 = sb.Hash();
    BOOST_CHECK(h1 != h2);
    
    // addColor does not modify hash
    bitset_t bs;
    bs.set(HEX_CELL_A5);
    sb.addColor(BLACK, bs);
    BOOST_CHECK(h2 == sb.Hash());

    // removeColor (even of a played move)
    // does not modify hash. 
    bs.reset();
    bs.set(HEX_CELL_A4);
    sb.removeColor(WHITE, bs);
    BOOST_CHECK(h2 == sb.Hash());

    // set color does not modify hash.
    bs.reset();
    bs.set(HEX_CELL_A3);
    sb.setColor(BLACK, bs);
    BOOST_CHECK(h2 == sb.Hash());
}

BOOST_AUTO_TEST_CASE(StoneBoard_WhoseTurn)
{
    BOOST_REQUIRE(MAX_WIDTH >= 5 && MAX_HEIGHT >= 5);
    StoneBoard sb = StoneBoard(5, 5);

    BOOST_CHECK_EQUAL(sb.WhoseTurn(), FIRST_TO_PLAY);

    sb.playMove(FIRST_TO_PLAY, FIRST_CELL);
    BOOST_CHECK_EQUAL(sb.WhoseTurn(), !FIRST_TO_PLAY);
    
    sb.playMove(sb.WhoseTurn(), HEX_CELL_A2);
    BOOST_CHECK_EQUAL(sb.WhoseTurn(), FIRST_TO_PLAY);

    sb.playMove(sb.WhoseTurn(), HEX_CELL_A3);
    BOOST_CHECK_EQUAL(sb.WhoseTurn(), !FIRST_TO_PLAY);

    sb.playMove(sb.WhoseTurn(), HEX_CELL_A4);
    BOOST_CHECK_EQUAL(sb.WhoseTurn(), FIRST_TO_PLAY);

    // check that swap is handled properly
    sb.startNewGame();
    BOOST_CHECK_EQUAL(sb.WhoseTurn(), FIRST_TO_PLAY);

    sb.playMove(FIRST_TO_PLAY, FIRST_CELL);
    BOOST_CHECK_EQUAL(sb.WhoseTurn(), !FIRST_TO_PLAY);

    sb.playMove(!FIRST_TO_PLAY, SWAP_PIECES);
    BOOST_CHECK_EQUAL(sb.WhoseTurn(), !FIRST_TO_PLAY);
    
    sb.playMove(sb.WhoseTurn(), HEX_CELL_A2);
    BOOST_CHECK_EQUAL(sb.WhoseTurn(), FIRST_TO_PLAY);
    
    sb.playMove(sb.WhoseTurn(), HEX_CELL_A3);
    BOOST_CHECK_EQUAL(sb.WhoseTurn(), !FIRST_TO_PLAY);

    sb.playMove(sb.WhoseTurn(), HEX_CELL_A4);
    BOOST_CHECK_EQUAL(sb.WhoseTurn(), FIRST_TO_PLAY);
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
            b1.playMove(*color, HEX_CELL_A1);

        BoardID id = b1.GetBoardID();
        BOOST_CHECK_EQUAL(id.size(), 1u);
    
        b2.SetState(id);
        BOOST_CHECK(b1 == b2);
    }

    // check a 5x3 state
    {
        StoneBoard b1(5, 3);
        StoneBoard b2(5, 3);

        //
        //    B..W.
        //     .WB..
        //      BW..W
        //
        b1.playMove(BLACK, HEX_CELL_A1);
        b1.playMove(WHITE, HEX_CELL_D1);
        b1.playMove(WHITE, HEX_CELL_B2);
        b1.playMove(BLACK, HEX_CELL_C2);
        b1.playMove(BLACK, HEX_CELL_A3);
        b1.playMove(WHITE, HEX_CELL_B3);
        b1.playMove(WHITE, HEX_CELL_E3);

        BoardID id = b1.GetBoardID();
        BOOST_CHECK_EQUAL(id.size(), 4u);
    
        b2.SetState(id);
        BOOST_CHECK(b1 == b2);
    }
}

}

//---------------------------------------------------------------------------
