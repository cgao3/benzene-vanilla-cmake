//----------------------------------------------------------------------------
/** @file VCList.hpp */
//----------------------------------------------------------------------------

#ifndef VCOR_HPP
#define VCOR_HPP

#include "Hex.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

std::vector<bitset_t> VCOr(const std::vector<bitset_t>& new_semis,
                           const std::vector<bitset_t>& old_semis,
                           const std::vector<bitset_t>& fulls,
                           bitset_t capturedSet);

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // VCOR_HPP
