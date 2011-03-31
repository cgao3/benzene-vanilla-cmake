//---------------------------------------------------------------------------
/** @file ConstBoardTest.cpp */
//---------------------------------------------------------------------------

#include <boost/test/auto_unit_test.hpp>

#include "BoardUtil.hpp"
#include "ConstBoard.hpp"

using namespace benzene;

//---------------------------------------------------------------------------

namespace {

BOOST_AUTO_TEST_CASE(ConstBoard_Dimensions)
{
    BOOST_REQUIRE(MAX_WIDTH >= 5 && MAX_HEIGHT >= 7);
    ConstBoard* cb = &ConstBoard::Get(1, 1);
    BOOST_CHECK_EQUAL(cb->Width(), 1);
    BOOST_CHECK_EQUAL(cb->Height(), 1);
    cb = &ConstBoard::Get(5);
    BOOST_CHECK_EQUAL(cb->Width(), 5);
    BOOST_CHECK_EQUAL(cb->Height(), 5);
    cb = &ConstBoard::Get(4, 7);
    BOOST_CHECK_EQUAL(cb->Width(), 4);
    BOOST_CHECK_EQUAL(cb->Height(), 7);
    cb = &ConstBoard::Get(MAX_WIDTH, MAX_HEIGHT);
    BOOST_CHECK_EQUAL(cb->Width(), MAX_WIDTH);
    BOOST_CHECK_EQUAL(cb->Height(), MAX_HEIGHT);
}

BOOST_AUTO_TEST_CASE(ConstBoard_CellsLocationsValid)
{
    BOOST_REQUIRE(MAX_WIDTH >= 5 && MAX_HEIGHT >= 3);
    ConstBoard* cb = &ConstBoard::Get(5, 3);
    bitset_t b1 = cb->GetCells();
    BOOST_CHECK_EQUAL(b1.count(), 15u);
    BOOST_CHECK(b1.test(FIRST_CELL));
    BOOST_CHECK(!b1.test(FIRST_CELL-1));
    BOOST_CHECK(!b1.test(NORTH));
    BOOST_CHECK(!b1.test(SOUTH));
    BOOST_CHECK(!b1.test(WEST));
    BOOST_CHECK(!b1.test(EAST));
    bitset_t b2 = cb->GetLocations(); // adds 4 edges
    BOOST_CHECK_EQUAL(b1.count() + 4, b2.count());
    BOOST_CHECK(BitsetUtil::IsSubsetOf(b1, b2));
    BOOST_CHECK(b2.test(FIRST_EDGE));
    BOOST_CHECK(!b2.test(FIRST_EDGE-1));
    BOOST_CHECK(!b2.test(SWAP_PIECES));
    bitset_t b3 = cb->GetValid(); // adds swap and resign
    BOOST_CHECK_EQUAL(b2.count() + 2, b3.count());
    BOOST_CHECK(BitsetUtil::IsSubsetOf(b2, b3));
    BOOST_CHECK(b3.test(FIRST_SPECIAL));
    BOOST_CHECK(!b3.test(FIRST_SPECIAL-1));
    
    // checking individual HexPoints
    BOOST_CHECK(cb->IsValid(SWAP_PIECES));
    BOOST_CHECK(!cb->IsLocation(SWAP_PIECES));
    BOOST_CHECK(cb->IsLocation(NORTH));
    BOOST_CHECK(cb->IsLocation(SOUTH));
    BOOST_CHECK(cb->IsValid(EAST));
    BOOST_CHECK(!cb->IsCell(WEST));
    BOOST_CHECK(cb->IsValid(HEX_CELL_A1));
    BOOST_CHECK(cb->IsCell(HEX_CELL_A3));
    BOOST_CHECK(cb->IsLocation(HEX_CELL_E3));
    BOOST_CHECK(!cb->IsValid(INVALID_POINT));
    BOOST_CHECK(cb->IsValid(RESIGN));
    BOOST_CHECK(!cb->IsLocation(RESIGN));
    BOOST_CHECK(FIRST_INVALID==BITSETSIZE || !cb->IsValid(FIRST_INVALID));
    BOOST_CHECK(!cb->IsValid(HEX_CELL_F1));
    BOOST_CHECK(!cb->IsValid(HEX_CELL_A4));
    BOOST_CHECK(!cb->IsValid(HEX_CELL_E4));
    
    // checking validity of bitsets
    BOOST_CHECK(cb->IsValid(b1));
    BOOST_CHECK(cb->IsValid(b2));
    BOOST_CHECK(cb->IsValid(b3));
    BOOST_CHECK(!cb->IsValid(b3.flip()));
    b3.flip(0);
    BOOST_CHECK(!cb->IsValid(b3.flip()));
    b1.reset();
    b1.set(0);
    BOOST_CHECK(!cb->IsValid(b1));
    b1.flip(0);
    b1.set(6);
    b1.set(7);
    BOOST_CHECK(cb->IsValid(b1));
}

BOOST_AUTO_TEST_CASE(ConstBoard_CellLocationValidIterators)
{
    BOOST_REQUIRE(MAX_WIDTH >= 9 && MAX_HEIGHT >= 6);
    ConstBoard* cb = &ConstBoard::Get(9, 6);
    bitset_t originalBitset, remainingBitset;
    bool allInSet, noRepeats;
    
    // testing cells iterator
    allInSet = true;
    noRepeats = true;
    originalBitset = cb->GetCells();
    remainingBitset = originalBitset;
    for (BoardIterator it(cb->Interior()); it; ++it) {
	allInSet = allInSet && originalBitset.test(*it);
	noRepeats = noRepeats && remainingBitset.test(*it);
	remainingBitset.reset(*it);
    }
    BOOST_CHECK(allInSet);
    BOOST_CHECK(noRepeats);
    BOOST_CHECK(remainingBitset.none());
    
    // testing locations iterator
    allInSet = true;
    noRepeats = true;
    originalBitset = cb->GetLocations();
    remainingBitset = originalBitset;
    for (BoardIterator it(cb->EdgesAndInterior()); it; ++it) {
	allInSet = allInSet && originalBitset.test(*it);
	noRepeats = noRepeats && remainingBitset.test(*it);
	remainingBitset.reset(*it);
    }
    BOOST_CHECK(allInSet);
    BOOST_CHECK(noRepeats);
    BOOST_CHECK(remainingBitset.none());
    
    // testing all iterator
    allInSet = true;
    noRepeats = true;
    originalBitset = cb->GetValid();
    remainingBitset = originalBitset;
    for (BoardIterator it(cb->AllValid()); it; ++it) {
	allInSet = allInSet && originalBitset.test(*it);
	noRepeats = noRepeats && remainingBitset.test(*it);
	remainingBitset.reset(*it);
    }
    BOOST_CHECK(allInSet);
    BOOST_CHECK(noRepeats);
    BOOST_CHECK(remainingBitset.none());
}

BOOST_AUTO_TEST_CASE(ConstBoard_NeighbourIterators)
{
    BOOST_REQUIRE(MAX_WIDTH >= 11 && MAX_HEIGHT >= 11);
    BOOST_REQUIRE(Pattern::MAX_EXTENSION >= 3);
    ConstBoard* cb = &ConstBoard::Get(8, 8);
    bitset_t b;
    bool allAdjacent, allUnique;
    
    // testing immediate neighbours iterator
    allAdjacent = true;
    allUnique = true;
    for (BoardIterator it(cb->Nbs(FIRST_CELL)); it; ++it) {
	allAdjacent = allAdjacent && cb->Adjacent(FIRST_CELL, *it);
	allUnique = allUnique && (!b.test(*it));
	b.set(*it);
    }
    BOOST_CHECK(allAdjacent);
    BOOST_CHECK(allUnique);
    BOOST_CHECK_EQUAL(b.count(), 4u);
    
    b.reset();
    allAdjacent = true;
    allUnique = true;
    for (BoardIterator it(cb->Nbs(WEST)); it; ++it) {
	allAdjacent = allAdjacent && cb->Adjacent(WEST, *it);
	allUnique = allUnique && (!b.test(*it));
	b.set(*it);
    }
    BOOST_CHECK(allAdjacent);
    BOOST_CHECK(allUnique);
    // interior cells + nbr edges
    BOOST_CHECK_EQUAL(b.count(), (std::size_t)(cb->Height()+2));
    
    b.reset();
    allAdjacent = true;
    allUnique = true;
    for (BoardIterator it(cb->Nbs(HEX_CELL_B6)); it; ++it) {
	allAdjacent = allAdjacent && cb->Adjacent(HEX_CELL_B6, *it);
	allUnique = allUnique && (!b.test(*it));
	b.set(*it);
    }
    BOOST_CHECK(allAdjacent);
    BOOST_CHECK(allUnique);
    BOOST_CHECK_EQUAL(b.count(), 6u);
    
    // testing radius neighbours iterator
    cb = &ConstBoard::Get(11, 11);
    int radius;
    bool allWithinRadius;
    radius = 2;
    b.reset();
    allUnique = true;
    allWithinRadius = true;
    for (BoardIterator it(cb->Nbs(HEX_CELL_F6, radius)); it; ++it) {
	int curDistance = cb->Distance(HEX_CELL_F6, *it);
	allWithinRadius = allWithinRadius && (0 < curDistance) && (curDistance <= radius);
	allUnique = allUnique && (!b.test(*it));
	b.set(*it);
    }
    BOOST_CHECK(allUnique);
    BOOST_CHECK(allWithinRadius);
    BOOST_CHECK_EQUAL(b.count(), 18u);
    
    radius = 3;
    b.reset();
    allUnique = true;
    allWithinRadius = true;
    for (BoardIterator it(cb->Nbs(HEX_CELL_F6, radius)); it; ++it) {
	int curDistance = cb->Distance(HEX_CELL_F6, *it);
	allWithinRadius = allWithinRadius && (0 < curDistance) && (curDistance <= radius);
	allUnique = allUnique && (!b.test(*it));
	b.set(*it);
    }
    BOOST_CHECK(allUnique);
    BOOST_CHECK(allWithinRadius);
    BOOST_CHECK_EQUAL(b.count(), 36u);
    
    b.reset();
    allUnique = true;
    allWithinRadius = true;
    for (BoardIterator it(cb->Nbs(HEX_CELL_D3, radius)); it; ++it) {
	int curDistance = cb->Distance(HEX_CELL_D3, *it);
	allWithinRadius = allWithinRadius && (0 < curDistance) && (curDistance <= radius);
	allUnique = allUnique && (!b.test(*it));
	b.set(*it);
    }
    BOOST_CHECK(allUnique);
    BOOST_CHECK(allWithinRadius);
    BOOST_CHECK_EQUAL(b.count(), 33u);
    
    b.reset();
    allUnique = true;
    allWithinRadius = true;
    for (BoardIterator it(cb->Nbs(SOUTH, radius)); it; ++it) {
	int curDistance = cb->Distance(SOUTH, *it);
	allWithinRadius = allWithinRadius && (0 < curDistance) && (curDistance <= radius);
	allUnique = allUnique && (!b.test(*it));
	b.set(*it);
    }
    BOOST_CHECK(allUnique);
    BOOST_CHECK(allWithinRadius);
    BOOST_CHECK_EQUAL(b.count(), (std::size_t)(radius*cb->Width() + 2));
    
    cb = &ConstBoard::Get(1, 1);
    b.reset();
    allUnique = true;
    allWithinRadius = true;
    for (BoardIterator it(cb->Nbs(EAST, radius)); it; ++it) {
	int curDistance = cb->Distance(EAST, *it);
	allWithinRadius = allWithinRadius && (0 < curDistance) && (curDistance <= radius);
	allUnique = allUnique && (!b.test(*it));
	b.set(*it);
    }
    BOOST_CHECK(allUnique);
    BOOST_CHECK(allWithinRadius);
    BOOST_CHECK_EQUAL(b.count(), 3u); // interior cell + 2 nbg edges
    
    cb = &ConstBoard::Get(3, 8);
    b.reset();
    allUnique = true;
    allWithinRadius = true;
    for (BoardIterator it(cb->Nbs(WEST, radius)); it; ++it) {
	int curDistance = cb->Distance(WEST, *it);
	allWithinRadius = allWithinRadius && (0 < curDistance) && (curDistance <= radius);
	allUnique = allUnique && (!b.test(*it));
	b.set(*it);
    }
    BOOST_CHECK(allUnique);
    BOOST_CHECK(allWithinRadius);
    BOOST_CHECK_EQUAL(b.count(), cb->GetLocations().count() - 2);
    
    radius = 2;
    b.reset();
    allUnique = true;
    allWithinRadius = true;
    for (BoardIterator it(cb->Nbs(WEST, radius)); it; ++it) {
	int curDistance = cb->Distance(WEST, *it);
	allWithinRadius = allWithinRadius 
            && (0 < curDistance) && (curDistance <= radius);
	allUnique = allUnique && (!b.test(*it));
	b.set(*it);
    }
    BOOST_CHECK(allUnique);
    BOOST_CHECK(allWithinRadius);
    BOOST_CHECK_EQUAL((int)b.count(), radius*cb->Height() + 2);
}

BOOST_AUTO_TEST_CASE(ConstBoard_DistanceAndAdjacency)
{
    BOOST_REQUIRE(MAX_WIDTH >= 11 && MAX_HEIGHT >= 11);
    
    // distance/adjacency from point on board to edges
    ConstBoard* cb = &ConstBoard::Get(1, 11);
    BOOST_CHECK_EQUAL(cb->Distance(HEX_CELL_A1, NORTH), 1);
    BOOST_CHECK(cb->Adjacent(HEX_CELL_A1, NORTH));
    BOOST_CHECK_EQUAL(cb->Distance(HEX_CELL_A1, SOUTH), 11);
    BOOST_CHECK(!cb->Adjacent(HEX_CELL_A1, SOUTH));
    BOOST_CHECK_EQUAL(cb->Distance(HEX_CELL_A1, EAST), 1);
    BOOST_CHECK(cb->Adjacent(HEX_CELL_A1, EAST));
    BOOST_CHECK_EQUAL(cb->Distance(HEX_CELL_A1, WEST), 1);
    BOOST_CHECK(cb->Adjacent(HEX_CELL_A1, WEST));
    cb = &ConstBoard::Get(8, 1);
    BOOST_CHECK_EQUAL(cb->Distance(HEX_CELL_B1, NORTH), 1);
    BOOST_CHECK(cb->Adjacent(HEX_CELL_A1, NORTH));
    BOOST_CHECK_EQUAL(cb->Distance(HEX_CELL_B1, SOUTH), 1);
    BOOST_CHECK(cb->Adjacent(HEX_CELL_A1, SOUTH));
    BOOST_CHECK_EQUAL(cb->Distance(HEX_CELL_B1, EAST), 7);
    BOOST_CHECK_EQUAL(cb->Distance(HEX_CELL_B1, WEST), 2);
    
    // distance and adjacency between two edges
    cb = &ConstBoard::Get(6, 7);
    BOOST_CHECK_EQUAL(cb->Distance(NORTH, NORTH), 0);
    BOOST_CHECK(!cb->Adjacent(NORTH, NORTH));
    BOOST_CHECK_EQUAL(cb->Distance(EAST, NORTH), 1);
    BOOST_CHECK_EQUAL(cb->Distance(SOUTH, NORTH), 7);
    BOOST_CHECK_EQUAL(cb->Distance(WEST, EAST), 6);
    BOOST_CHECK(!cb->Adjacent(EAST, WEST));
    BOOST_CHECK(!cb->Adjacent(NORTH, SOUTH));
    BOOST_CHECK(cb->Adjacent(NORTH, EAST));
    BOOST_CHECK(cb->Adjacent(NORTH, WEST));
    BOOST_CHECK(cb->Adjacent(SOUTH, EAST));
    BOOST_CHECK(cb->Adjacent(SOUTH, WEST));
    
    // adjacency of two points on board
    BOOST_CHECK(!cb->Adjacent(HEX_CELL_C6, HEX_CELL_B5));
    BOOST_CHECK(cb->Adjacent(HEX_CELL_C6, HEX_CELL_B6));
    BOOST_CHECK(cb->Adjacent(HEX_CELL_C6, HEX_CELL_B7));
    BOOST_CHECK(cb->Adjacent(HEX_CELL_C6, HEX_CELL_C5));
    BOOST_CHECK(!cb->Adjacent(HEX_CELL_C6, HEX_CELL_C6));
    BOOST_CHECK(cb->Adjacent(HEX_CELL_C6, HEX_CELL_C7));
    BOOST_CHECK(cb->Adjacent(HEX_CELL_C6, HEX_CELL_D5));
    BOOST_CHECK(cb->Adjacent(HEX_CELL_C6, HEX_CELL_D6));
    BOOST_CHECK(!cb->Adjacent(HEX_CELL_C6, HEX_CELL_D7));
    BOOST_CHECK(cb->Adjacent(HEX_CELL_A7, WEST));
    BOOST_CHECK(cb->Adjacent(HEX_CELL_A7, SOUTH));
    
    // distance between two points on board
    cb = &ConstBoard::Get(11, 11);
    BOOST_CHECK_EQUAL(cb->Distance(HEX_CELL_F4, HEX_CELL_F4), 0);
    BOOST_CHECK_EQUAL(cb->Distance(HEX_CELL_F4, HEX_CELL_A1), 8);
    BOOST_CHECK_EQUAL(cb->Distance(HEX_CELL_F4, HEX_CELL_B7), 4);
    BOOST_CHECK_EQUAL(cb->Distance(HEX_CELL_F4, HEX_CELL_C4), 3);
    BOOST_CHECK_EQUAL(cb->Distance(HEX_CELL_F4, HEX_CELL_F1), 3);
    BOOST_CHECK_EQUAL(cb->Distance(HEX_CELL_F4, HEX_CELL_F10), 6);
    BOOST_CHECK_EQUAL(cb->Distance(HEX_CELL_F4, HEX_CELL_H4), 2);
    BOOST_CHECK_EQUAL(cb->Distance(HEX_CELL_F4, HEX_CELL_K11), 12);
}

}

//---------------------------------------------------------------------------
