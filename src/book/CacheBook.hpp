//----------------------------------------------------------------------------
/** @file CacheBook.hpp */
//----------------------------------------------------------------------------

#ifndef CACHEBOOK_HPP
#define CACHEBOOK_HPP

#include "Hex.hpp"
#include "HexState.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Uses previously-generated opening moves that have been recorded
    to prevent re-computing them.

    A cache book is just a map of state hashes to HexPoints.

    Not using StateMap since may not want same behaviour in rotated positions.
*/

class CacheBook
{
public:
    CacheBook();
    bool Exists(const HexState& state) const;
    HexPoint& operator[](const HexState& state);
    std::size_t Size() const;

private:
    std::map<SgHashCode, HexPoint> m_map;

    void ParseFile(std::ifstream& inFile);
    std::vector<HexPoint> ReadPoints(std::istringstream& in) const;
};

inline bool CacheBook::Exists(const HexState& state) const
{
    return m_map.find(state.Hash()) != m_map.end();
}

inline HexPoint& CacheBook::operator[](const HexState& state)
{
    return m_map[state.Hash()];
}

inline std::size_t CacheBook::Size() const
{
    return m_map.size();
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // CACHEBOOK_HPP
