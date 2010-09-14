//----------------------------------------------------------------------------
/** @file CacheBook.hpp
 */
//----------------------------------------------------------------------------

#ifndef CACHEBOOK_HPP
#define CACHEBOOK_HPP

#include "Hex.hpp"
#include "StateDB.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Uses previously-generated opening moves that have been recorded
    to prevent re-computing them.

    A cache book is just a map of state hashes to HexPoints.
*/

class CacheBook : public StateMap<HexPoint>
{
public:
    CacheBook();
private:
    void ParseFile(std::ifstream& inFile);
    std::vector<HexPoint> ReadPoints(std::istringstream& in) const;
};

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // CACHEBOOK_HPP
