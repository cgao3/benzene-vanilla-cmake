//----------------------------------------------------------------------------
/** @file VCOr.hpp */
//----------------------------------------------------------------------------

#ifndef VCOR_HPP
#define VCOR_HPP

#include "Hex.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

class CarrierList;

std::vector<bitset_t> VCOr(const CarrierList& semis, const CarrierList& fulls,
                           bitset_t xCapturedSet, bitset_t yCapturedSet);

std::vector<bitset_t> VCOr(CarrierList semis, bitset_t fulls_intersect,
                           bitset_t xCapturedSet, bitset_t yCapturedSet);

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // VCOR_HPP
