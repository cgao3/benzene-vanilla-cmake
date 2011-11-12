//----------------------------------------------------------------------------
/** @file VCList.hpp */
//----------------------------------------------------------------------------

#ifndef VCOR_HPP
#define VCOR_HPP

#include "Hex.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

class CarrierList;

std::vector<bitset_t> VCOr(const CarrierList& semis,
                           const CarrierList& fulls,
                           bitset_t capturedSet);

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // VCOR_HPP
