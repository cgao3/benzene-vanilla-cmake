//----------------------------------------------------------------------------
/** @file
 */
//----------------------------------------------------------------------------

#include <boost/test/auto_unit_test.hpp>

#include "HexBoard.hpp"
#include "Connections.hpp"

using namespace benzene;

//---------------------------------------------------------------------------

namespace {

//---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(HexBoard_PlayAndUndo)
{
    ICEngine ice;
    ConnectionBuilderParam param;
    HexBoard brd(7, 7, ice, param);

    brd.startNewGame();
    brd.ComputeAll(BLACK, HexBoard::DO_NOT_REMOVE_WINNING_FILLIN);
    BOOST_CHECK(!brd.Cons(BLACK).Exists(NORTH, HEX_CELL_A4, VC::FULL));

    brd.PlayMove(BLACK, HEX_CELL_B2);
    BOOST_CHECK_EQUAL(brd.getColor(HEX_CELL_B2), BLACK);
    BOOST_CHECK(brd.Cons(BLACK).Exists(NORTH, HEX_CELL_A4, VC::FULL));

    brd.UndoMove();
    BOOST_CHECK(brd.isEmpty(HEX_CELL_B2));
    BOOST_CHECK(!brd.Cons(BLACK).Exists(NORTH, HEX_CELL_A4, VC::FULL));
}

BOOST_AUTO_TEST_CASE(HexBoard_CopyConstructor)
{
    ICEngine ice;
    ConnectionBuilderParam param;
    HexBoard brd(7, 7, ice, param);
    
    brd.startNewGame();
    brd.ComputeAll(BLACK, HexBoard::DO_NOT_REMOVE_WINNING_FILLIN);
    brd.PlayMove(BLACK, HEX_CELL_B2);
    BOOST_CHECK_EQUAL(brd.getColor(HEX_CELL_B2), BLACK);
    BOOST_CHECK(brd.Cons(BLACK).Exists(NORTH, HEX_CELL_A4, VC::FULL));

    HexBoard cpy(brd);
    BOOST_CHECK_EQUAL(cpy.getColor(HEX_CELL_B2), BLACK);
    BOOST_CHECK(cpy.Cons(BLACK).Exists(NORTH, HEX_CELL_A4, VC::FULL));

    brd.UndoMove();
    BOOST_CHECK_EQUAL(cpy.getColor(HEX_CELL_B2), BLACK);
    BOOST_CHECK(cpy.Cons(BLACK).Exists(NORTH, HEX_CELL_A4, VC::FULL));    
}

//---------------------------------------------------------------------------

} // namespace

//---------------------------------------------------------------------------
