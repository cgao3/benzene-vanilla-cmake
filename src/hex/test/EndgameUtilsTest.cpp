//---------------------------------------------------------------------------
/** @file EndgameUtilsTest.cpp
 */
//---------------------------------------------------------------------------

#include <boost/test/auto_unit_test.hpp>

#include "EndgameUtils.hpp"
#include "BitsetIterator.hpp"
#include "BoardUtils.hpp"

using namespace benzene;

//---------------------------------------------------------------------------

namespace {

/** Ensure MovesToConsider() removes all rotations from consider set. */
BOOST_AUTO_TEST_CASE(EndgameUtils_ConsiderRotations)
{
    ICEngine ice;
    VCBuilderParam param;
    HexBoard brd(9, 9, ice, param);
    brd.GetPosition().PlayMove(BLACK, HEX_CELL_E5);
    brd.ComputeAll(WHITE);
    const bitset_t consider = EndgameUtils::MovesToConsider(brd, WHITE);
    for (BitsetIterator it(consider); it; ++it)
    {
        HexPoint rotated = BoardUtils::Rotate(brd.GetPosition().Const(), *it);
        BOOST_CHECK(!consider.test(rotated));
    }
}

}

//---------------------------------------------------------------------------
