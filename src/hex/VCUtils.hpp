//----------------------------------------------------------------------------
// $Id: VCUtils.hpp 1536 2008-07-09 22:47:27Z broderic $
//----------------------------------------------------------------------------

#ifndef VCUTILS_HPP
#define VCUTILS_HPP

#include "Hex.hpp"
#include "StoneBoard.hpp"
#include "VC.hpp"

//----------------------------------------------------------------------------

class HexBoard;

/** Utilities on VCs. */
namespace VCUtils
{

    /** Returns true if carrier defines a valid bridge to the
        edge. Edge and the other endpoint are stored if it is valid.
    */
    bool ValidEdgeBridge(const StoneBoard& brd, 
                         const bitset_t& carrier, 
                         HexPoint& endpoint,
                         HexPoint& edge);
   
    /** This method examines a HexBoard (with VCs up-to-date) and
	finds which chains are virtually-connected for player c.
	It then returns a list of VCs between chains to maintain,
	excluding bridges and AND-rule VCs that can be covered by
	smaller VCs in the list.
     */
    void findMaintainableVCs(const HexBoard& brd, HexColor c,
			     std::vector<VC>& maintain,
                             std::size_t max=999999);
};

//----------------------------------------------------------------------------

#endif // VCUTILS_HPP
