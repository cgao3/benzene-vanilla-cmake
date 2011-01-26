//---------------------------------------------------------------------------
/** @file DecompositionsTest.cpp */
//---------------------------------------------------------------------------
#include <boost/test/auto_unit_test.hpp>

#include "Decompositions.hpp"

using namespace benzene;

//---------------------------------------------------------------------------

namespace {

BOOST_AUTO_TEST_CASE(Decompositions_VCDecomp)
{
    ICEngine ice;
    VCBuilderParam param;
    HexBoard brd(7, 7, ice, param);
    std::string str(". . . . W B ."
                     ". . . . . . ."
                      ". B B B W . ."
                       ". B B W . . ."
                        ". . W . . . ."
                         ". . W . . . ."
                          ". . . . . . .");
    brd.GetPosition().SetPosition(str);

    // Find decomp between E1, B3, WEST, and NORTH. 
    brd.SetUseDecompositions(false);
    brd.ComputeAll(BLACK);
    brd.SetUseDecompositions(true);
    bitset_t capturedVC;
    BOOST_CHECK(Decompositions::Find(brd, BLACK, capturedVC));
    BOOST_CHECK(capturedVC.any());
}

BOOST_AUTO_TEST_CASE(BoardUtil_SplitDecompositions)
{
    ICEngine ice;
    VCBuilderParam param;
    HexBoard brd(7, 7, ice, param);
    std::string s(". . . . W B ."
                   ". . . . . . ."
                    ". B B B W . ."
                     ". B B W . . ."
                      ". . W . . . ."
                       ". . W . . . ."
                        ". . . . . . .");
    brd.GetPosition().SetPosition(s);

    // Find splitting decomp between NORTH, E3, SOUTH.
    brd.ComputeAll(WHITE);
    HexPoint group;
    BOOST_CHECK(Decompositions::FindSplitting(brd, WHITE, group));
    BOOST_CHECK_EQUAL(group, HEX_CELL_E3);
}

} // namespace

//---------------------------------------------------------------------------
