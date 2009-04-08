//---------------------------------------------------------------------------
// $Id: ConnectionsTest.cpp 1794 2008-12-15 00:09:48Z broderic $
//---------------------------------------------------------------------------

#include <boost/test/auto_unit_test.hpp>

#include "BitsetIterator.hpp"
#include "ConnectionBuilder.hpp"
#include "Connections.hpp"
#include "ChangeLog.hpp"
#include "GroupBoard.hpp"

//---------------------------------------------------------------------------

namespace {

BOOST_AUTO_UNIT_TEST(Connections_CheckCopy)
{
    GroupBoard bd(11, 11);
    Connections con1(bd.Const(), BLACK);
    con1.Add(VC(NORTH, SOUTH), 0);
    Connections con2(con1);
    BOOST_CHECK(con1 == con2);

    con1.Add(VC(NORTH, HEX_CELL_A1), 0);
    BOOST_CHECK(con1 != con2);

    con2 = con1;
    BOOST_CHECK(con1 == con2);

    con1.Add(VC(NORTH, HEX_CELL_C1), 0);
    BOOST_CHECK(con1 != con2);
}

/** @todo Make this test quicker! */
BOOST_AUTO_UNIT_TEST(Connections_CheckRevert)
{
    GroupBoard bd(11, 11);

    bd.startNewGame();
    bd.playMove(BLACK, HEX_CELL_A9);
    bd.playMove(WHITE, HEX_CELL_F5);
    bd.playMove(BLACK, HEX_CELL_I4);
    bd.playMove(WHITE, HEX_CELL_H6);

    ChangeLog<VC> cl;
    Connections con1(bd.Const(), BLACK);
    con1.SetSoftLimit(VC::FULL, 10);
    con1.SetSoftLimit(VC::SEMI, 25);
    Connections con2(con1);

    ConnectionBuilderParam param;
    param.max_ors = 4;
    param.and_over_edge = true;
    param.use_greedy_union = true;

    ConnectionBuilder builder(param);
    builder.Build(con1, bd);
    builder.Build(con2, bd);
    BOOST_CHECK(con1 == con2);

    for (BitsetIterator p(bd.getEmpty()); p; ++p) {
        bitset_t added[BLACK_AND_WHITE];
        added[BLACK].set(*p);
        bd.absorb();
        bd.playMove(BLACK, *p);
        bd.absorb(*p);

        builder.Build(con2, bd, added, &cl);
        
        //std::cout << cl.dump() << std::endl;

        con2.Revert(cl);
        bd.undoMove(*p);

        BOOST_CHECK(cl.empty());
        BOOST_CHECK(ConUtil::EqualOnGroups(con1, con2, bd));
    }
}

}

//---------------------------------------------------------------------------
