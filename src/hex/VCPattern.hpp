//----------------------------------------------------------------------------
/** @file VCPattern.hpp */
//----------------------------------------------------------------------------

#ifndef VC_PATTERN_HPP
#define VC_PATTERN_HPP

#include "Hex.hpp"
#include "StoneBoard.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

class VCPattern;

typedef std::vector<VCPattern> VCPatternSet;

//----------------------------------------------------------------------------

/** Precomputed pattern specifying a virtual connection/ladder. */
class VCPattern
{        
public: 
    VCPattern(HexPoint end1, HexPoint end2, const bitset_t& must_have, 
              const bitset_t& not_oppt);

    ~VCPattern();

    /** Returns the set of patterns for the given boardsize; creates
        patterns if they currently do not exist. Returned set of
        patterns will always be empty if width does not equal
        height. */
    static const VCPatternSet& GetPatterns(int width, int height, 
                                           HexColor color);

    /** Returns cells that this player must have. */
    bitset_t MustHave() const;

    /** Returns cells that must not be opponent stones. */
    bitset_t NotOpponent() const;

    /** Returns the endpoints, i must be in [0, 1]. */
    HexPoint Endpoint(int i) const;

    /** Returns true if this pattern matches the given board. */
    bool Matches(HexColor color, const StoneBoard& brd) const;

    /** Shifts the pattern in direction dir, if possible. Returns true
        on success, false if shifted pattern goes off board. 
        ONLY USE THIS IF YOU KNOW WHAT YOU ARE DOING! */
    bool ShiftPattern(HexDirection dir, const StoneBoard& brd);

private:
    /** Cells that must be occupied. */
    bitset_t m_must_have;

    /** Cells that cannot be opponent stones. */
    bitset_t m_not_oppt;

    /** Endpoints connected by this VC. */
    HexPoint m_end1, m_end2;

    //------------------------------------------------------------------------

    typedef std::map< std::pair<int, int>, VCPatternSet > VCPatternSetMap;
    struct GlobalData
    {
        VCPatternSetMap constructed[BLACK_AND_WHITE];
    };

    /** Returns the set of PatternSets already constructed. */
    static VCPatternSetMap& GetConstructed(HexColor color);

    /** Creates the patterns for a given boardsize. */
    static void CreatePatterns(int width, int height);
};

inline bitset_t VCPattern::MustHave() const
{
    return m_must_have;
}

inline bitset_t VCPattern::NotOpponent() const
{
    return m_not_oppt;
}

inline HexPoint VCPattern::Endpoint(int i) const
{
    BenzeneAssert(0 <= i && i <= 1);
    return (i == 0) ? m_end1 : m_end2;
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // VC_PATTERN_HPP
