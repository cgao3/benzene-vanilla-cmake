//---------------------------------------------------------------------------
/** @file
 */
//---------------------------------------------------------------------------

#include <boost/test/auto_unit_test.hpp>

#include "Bitset.hpp"

using namespace benzene;

//---------------------------------------------------------------------------

namespace {

BOOST_AUTO_TEST_CASE(Bitset_Basics)
{
    // sort of ridiculous, but in case someone switches
    // from stl to some other basis, we need these
    // operations to still work as expected
    bitset_t b1, b2;
    BOOST_REQUIRE(BITSETSIZE > 1);
    BOOST_CHECK_EQUAL(b1.count(), 0u);
    b1.flip();
    BOOST_CHECK_EQUAL(b1.count(), (std::size_t)BITSETSIZE);
    BOOST_CHECK(b1.test(0));
    BOOST_CHECK(b1.test(BITSETSIZE-1));
    BOOST_CHECK(!b2.test(0));
    BOOST_CHECK(!b2.test(BITSETSIZE-1));
    BOOST_CHECK(!b2.any());
    BOOST_CHECK(b2.none());
    BOOST_CHECK_EQUAL(b1.size(), (std::size_t)BITSETSIZE);
    BOOST_CHECK(!b1.none());
    b1.reset();
    BOOST_CHECK(b1.none());
    b1.set(0);
    BOOST_CHECK_EQUAL(b1.count(), 1u);
    b2.set(1);
    BOOST_CHECK_EQUAL(b2.count(), 1u);
    b2 |= b1;
    BOOST_CHECK_EQUAL(b2.count(), 2u);
    b1 ^= b2;
    BOOST_CHECK_EQUAL(b1.count(), 1u);
    BOOST_CHECK(!b1.test(0));
    BOOST_CHECK(b1.test(1));
    BOOST_CHECK(b2.test(0));
    BOOST_CHECK(b2.test(1));
    b2 &= b1;
    BOOST_CHECK_EQUAL(b1, b2);
    b1 ^= b2;
    BOOST_CHECK(b1.none());
}

BOOST_AUTO_TEST_CASE(Bitset_ConversionToBytes)
{
    bitset_t b1, b2;
    byte byteArray[8];
    BOOST_REQUIRE(BITSETSIZE >= 64);
    int numBits = 64;
    BitsetUtil::BitsetToBytes(b1, byteArray, numBits);
    b2 = BitsetUtil::BytesToBitset(byteArray, numBits);
    BOOST_CHECK_EQUAL(b1, b2);
    BOOST_CHECK_EQUAL(byteArray[0], 0);
    BOOST_CHECK_EQUAL(byteArray[1], 0);
    b1.set(0);
    b1.set(3);
    BitsetUtil::BitsetToBytes(b1, byteArray, numBits);
    b2 = BitsetUtil::BytesToBitset(byteArray, numBits);
    BOOST_CHECK_EQUAL(b1, b2);
    BOOST_CHECK_EQUAL(byteArray[0], 9);
    BOOST_CHECK_EQUAL(byteArray[1], 0);
    b1.set(7);
    b1.set(8);
    BitsetUtil::BitsetToBytes(b1, byteArray, numBits);
    b2 = BitsetUtil::BytesToBitset(byteArray, numBits);
    BOOST_CHECK_EQUAL(b1, b2);
    BOOST_CHECK_EQUAL(byteArray[0], 137);
    BOOST_CHECK_EQUAL(byteArray[1], 1);
}

BOOST_AUTO_TEST_CASE(Bitset_ConversionToHex)
{
    bitset_t b;
    std::string s;
    BOOST_REQUIRE(BITSETSIZE >= 128);
    s = BitsetUtil::BitsetToHex(b, 124);
    BOOST_CHECK_EQUAL(BitsetUtil::HexToBitset(s), b);
    BOOST_CHECK_EQUAL(s, "0000000000000000000000000000000");
    b.set(3);
    b.set(5);
    b.set(6);
    s = BitsetUtil::BitsetToHex(b, 124);
    BOOST_CHECK_EQUAL(BitsetUtil::HexToBitset(s), b);
    BOOST_CHECK_EQUAL(s, "8600000000000000000000000000000");
    b.set(4);
    b.flip(6);
    b.set(8);
    s = BitsetUtil::BitsetToHex(b, 64);
    BOOST_CHECK_EQUAL(BitsetUtil::HexToBitset(s), b);
    BOOST_CHECK_EQUAL(s, "8310000000000000");
    b.set(65);
    s = BitsetUtil::BitsetToHex(b, 64);
    BOOST_CHECK_EQUAL(s, "8310000000000000");
    BOOST_CHECK(BitsetUtil::HexToBitset(s) != b);
    BOOST_CHECK(BitsetUtil::IsSubsetOf(BitsetUtil::HexToBitset(s), b));
}

BOOST_AUTO_TEST_CASE(Bitset_Subtraction)
{
    bitset_t b1, b2;
    BOOST_REQUIRE(BITSETSIZE > 2);
    b1.set(0);
    b2.set(1);
    BOOST_CHECK_EQUAL(BitsetUtil::Subtract(b1, b2), b1);
    BOOST_CHECK_EQUAL(b1 - b2, b1);
    BOOST_CHECK_EQUAL(BitsetUtil::Subtract(b2, b1), b2);
    BOOST_CHECK_EQUAL(b2 - b1, b2);
    b2 |= b1;
    // b1=100...0, b2=110...0
    BOOST_CHECK(BitsetUtil::Subtract(b1, b2).none());
    BOOST_CHECK((b1 - b2).none());
    BOOST_CHECK_EQUAL(BitsetUtil::Subtract(b2, b1).count(), 1u);
    BOOST_CHECK_EQUAL((b2 - b1).count(), 1u);
    b2 ^= b1;
    b1.flip();
    // b1=011...1, b2=010...0
    BOOST_CHECK(BitsetUtil::Subtract(b2, b1).none());
    BOOST_CHECK((b2 - b1).none());
    BOOST_CHECK_EQUAL(BitsetUtil::Subtract(b1, b2).count(), 
                      (std::size_t)(BITSETSIZE-2));
    BOOST_CHECK_EQUAL((b1 - b2).count(), (std::size_t)(BITSETSIZE-2));
    b2.flip();
    BOOST_CHECK_EQUAL(BitsetUtil::Subtract(b1, b2).count(), 1u);
    BOOST_CHECK_EQUAL((b1 - b2).count(), 1u);
    BOOST_CHECK(BitsetUtil::Subtract(b1, b2).test(1));
    BOOST_CHECK((b1 - b2).test(1));
    BOOST_CHECK_EQUAL(BitsetUtil::Subtract(b2, b1).count(), 1u);
    BOOST_CHECK_EQUAL((b2 - b1).count(), 1u);
    BOOST_CHECK(BitsetUtil::Subtract(b2, b1).test(0));
    BOOST_CHECK((b2 - b1).test(0));
}

BOOST_AUTO_TEST_CASE(Bitset_Comparison)
{
    // Note: cannot assume any order for IsLessThan, so can only
    // check that it is transitive and non-reflexive
    bitset_t b1, b2, b3;
    bool order1, order2, order3, order4, order5, order6;
    BOOST_CHECK(!BitsetUtil::IsLessThan(b1, b2));
    BOOST_CHECK(BitsetUtil::IsSubsetOf(b1, b2));
    BOOST_REQUIRE(BITSETSIZE >= 8);
    b2.set(0);
    // b1 = 000...0, b2=100...0, b3=000...0
    BOOST_CHECK(BitsetUtil::IsLessThan(b1, b2) !=
		BitsetUtil::IsLessThan(b2, b1));
    BOOST_CHECK(BitsetUtil::IsSubsetOf(b1, b2));
    b1.set(1);
    BOOST_CHECK(BitsetUtil::IsLessThan(b1, b2) !=
		BitsetUtil::IsLessThan(b2, b1));
    BOOST_CHECK(!BitsetUtil::IsSubsetOf(b1, b2));
    BOOST_CHECK(BitsetUtil::IsSubsetOf(b3, b1));
    BOOST_CHECK(BitsetUtil::IsSubsetOf(b3, b2));
    b3 = b2;
    b2.flip();
    // b1=010...0, b2=011...1, b3=100...0
    BOOST_CHECK(!BitsetUtil::IsLessThan(b1, b1));
    BOOST_CHECK(!BitsetUtil::IsLessThan(b2, b2));
    BOOST_CHECK(!BitsetUtil::IsLessThan(b3, b3));
    order1 = BitsetUtil::IsLessThan(b1, b2);
    order2 = BitsetUtil::IsLessThan(b1, b3);
    order3 = BitsetUtil::IsLessThan(b2, b1);
    order4 = BitsetUtil::IsLessThan(b2, b3);
    order5 = BitsetUtil::IsLessThan(b3, b1);
    order6 = BitsetUtil::IsLessThan(b3, b2);
    BOOST_CHECK(order1 != order3);
    BOOST_CHECK(order2 != order5);
    BOOST_CHECK(order4 != order6);
    BOOST_CHECK_EQUAL(order1 && order4 && order5, false);
    BOOST_CHECK_EQUAL(order2 && order3 && order6, false);
    BOOST_CHECK(BitsetUtil::IsSubsetOf(b1, b2));
    BOOST_CHECK(!BitsetUtil::IsSubsetOf(b1, b3));
    b2.flip(1);
    // b1=010...0, b2=001...1, b3=100...0
    BOOST_CHECK(!BitsetUtil::IsLessThan(b1, b1));
    BOOST_CHECK(!BitsetUtil::IsLessThan(b2, b2));
    BOOST_CHECK(!BitsetUtil::IsLessThan(b3, b3));
    order1 = BitsetUtil::IsLessThan(b1, b2);
    order2 = BitsetUtil::IsLessThan(b1, b3);
    order3 = BitsetUtil::IsLessThan(b2, b1);
    order4 = BitsetUtil::IsLessThan(b2, b3);
    order5 = BitsetUtil::IsLessThan(b3, b1);
    order6 = BitsetUtil::IsLessThan(b3, b2);
    BOOST_CHECK(order1 != order3);
    BOOST_CHECK(order2 != order5);
    BOOST_CHECK(order4 != order6);
    BOOST_CHECK_EQUAL(order1 && order4 && order5, false);
    BOOST_CHECK_EQUAL(order2 && order3 && order6, false);
    BOOST_CHECK(!BitsetUtil::IsSubsetOf(b1, b2));
    b1.flip();
    b3.flip(2);
    // b1=101...1, b2=001...1, b3=101...0
    BOOST_CHECK(!BitsetUtil::IsLessThan(b1, b1));
    BOOST_CHECK(!BitsetUtil::IsLessThan(b2, b2));
    BOOST_CHECK(!BitsetUtil::IsLessThan(b3, b3));
    order1 = BitsetUtil::IsLessThan(b1, b2);
    order2 = BitsetUtil::IsLessThan(b1, b3);
    order3 = BitsetUtil::IsLessThan(b2, b1);
    order4 = BitsetUtil::IsLessThan(b2, b3);
    order5 = BitsetUtil::IsLessThan(b3, b1);
    order6 = BitsetUtil::IsLessThan(b3, b2);
    BOOST_CHECK(order1 != order3);
    BOOST_CHECK(order2 != order5);
    BOOST_CHECK(order4 != order6);
    BOOST_CHECK_EQUAL(order1 && order4 && order5, false);
    BOOST_CHECK_EQUAL(order2 && order3 && order6, false);
    BOOST_CHECK(!BitsetUtil::IsSubsetOf(b1, b2));
    BOOST_CHECK(BitsetUtil::IsSubsetOf(b2, b1));
    BOOST_CHECK(BitsetUtil::IsSubsetOf(b3, b1));
    BOOST_CHECK(!BitsetUtil::IsSubsetOf(b3, b2));
    b2.flip();
    // b1=101...1, b2=110...0, b3=101...0
    BOOST_CHECK(!BitsetUtil::IsLessThan(b1, b1));
    BOOST_CHECK(!BitsetUtil::IsLessThan(b2, b2));
    BOOST_CHECK(!BitsetUtil::IsLessThan(b3, b3));
    order1 = BitsetUtil::IsLessThan(b1, b2);
    order2 = BitsetUtil::IsLessThan(b1, b3);
    order3 = BitsetUtil::IsLessThan(b2, b1);
    order4 = BitsetUtil::IsLessThan(b2, b3);
    order5 = BitsetUtil::IsLessThan(b3, b1);
    order6 = BitsetUtil::IsLessThan(b3, b2);
    BOOST_CHECK(order1 != order3);
    BOOST_CHECK(order2 != order5);
    BOOST_CHECK(order4 != order6);
    BOOST_CHECK_EQUAL(order1 && order4 && order5, false);
    BOOST_CHECK_EQUAL(order2 && order3 && order6, false);
    BOOST_CHECK(!BitsetUtil::IsSubsetOf(b1, b2));
    BOOST_CHECK(!BitsetUtil::IsSubsetOf(b2, b1));
    BOOST_CHECK(BitsetUtil::IsSubsetOf(b3, b1));
    BOOST_CHECK(!BitsetUtil::IsSubsetOf(b3, b2));
    b1.set(1);
    b2.set();
    // b1=111...1, b2=111...1, b3=101...0
    BOOST_CHECK_EQUAL(b1, b2);
    BOOST_CHECK(!BitsetUtil::IsLessThan(b2, b1));
    BOOST_CHECK(!BitsetUtil::IsLessThan(b3, b3));
    order1 = BitsetUtil::IsLessThan(b1, b3);
    order2 = BitsetUtil::IsLessThan(b3, b1);
    BOOST_CHECK(order1 != order2);
    BOOST_CHECK(BitsetUtil::IsSubsetOf(b1, b2));
    BOOST_CHECK(BitsetUtil::IsSubsetOf(b2, b1));
    BOOST_CHECK(!BitsetUtil::IsSubsetOf(b2, b3));
    BOOST_CHECK(BitsetUtil::IsSubsetOf(b3, b2));
}

BOOST_AUTO_TEST_CASE(Bitset_ConversionToVector)
{
    bitset_t b;
    std::vector<int> moves;
    BitsetUtil::BitsetToVector(b, moves);
    BOOST_CHECK(moves.empty());
    BOOST_REQUIRE(BITSETSIZE >= 16);
    b.set(1);
    BitsetUtil::BitsetToVector(b, moves);
    BOOST_CHECK_EQUAL(moves.size(), 1u);
    BOOST_CHECK_EQUAL(moves[0], 1);
    b.set(14);
    BitsetUtil::BitsetToVector(b, moves);
    BOOST_CHECK_EQUAL(moves.size(), 2u);
    BOOST_CHECK_EQUAL(moves[0], 1);
    BOOST_CHECK_EQUAL(moves[1], 14);
    b.flip();
    BitsetUtil::BitsetToVector(b, moves);
    BOOST_CHECK_EQUAL(moves.size(), (std::size_t)(BITSETSIZE-2));
    BOOST_CHECK_EQUAL(moves[moves.size()-1], BITSETSIZE-1);
}

BOOST_AUTO_TEST_CASE(Bitset_FindSingleton)
{
    bitset_t b;
    BOOST_REQUIRE(BITSETSIZE > 2);
    b.set(0);
    BOOST_CHECK_EQUAL(BitsetUtil::FindSetBit(b), 0);
    b.flip(1);
    b.flip(0);
    BOOST_CHECK_EQUAL(BitsetUtil::FindSetBit(b), 1);
    b.reset();
    b.set(BITSETSIZE-1);
    BOOST_CHECK_EQUAL(BitsetUtil::FindSetBit(b), BITSETSIZE-1);
}

}

//---------------------------------------------------------------------------
