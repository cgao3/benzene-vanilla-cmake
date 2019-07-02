//----------------------------------------------------------------------------
/** @file IcePatternSet.hpp */
//----------------------------------------------------------------------------

#ifndef ICE_PATTERN_SET_HPP
#define ICE_PATTERN_SET_HPP

#include "Pattern.hpp"
#include "HashedPatternSet.hpp"

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Collection of Patterns and HashedPatterns for use in ICE. */
class IcePatternSet
{
public:

    /** Creates an empty set of patterns. */
    IcePatternSet();

    /** Destructor. */
    ~IcePatternSet();

    /** Loads the patterns from the given file. */
    void LoadPatterns(std::string name);

    /** @name Pattern Access Methods */
    // @{
    const HashedPatternSet& HashedEFillin() const;
    const HashedPatternSet& HashedFillin(HexColor color) const;
    const HashedPatternSet& HashedSReversible(HexColor color) const;
    const HashedPatternSet& HashedTReversible(HexColor color) const;
    const HashedPatternSet& HashedInferior(HexColor color) const;

    // Subset of HashedFillin.
    const HashedPatternSet& HashedCaptured(HexColor color) const;
    // Subset of HashedSReversible.
    const HashedPatternSet& HashedVulnerable(HexColor color) const;

    // Independant set, not containing the strongly reversible.
    const HashedPatternSet& HashedReversible(HexColor color) const;
  
    // @}

private:

    /** @name Patterns 
        Patterns for each type of Inferior cell. */
    // @{
    std::vector<Pattern> m_e_fillin;  
    std::vector<Pattern> m_fillin[BLACK_AND_WHITE];
    std::vector<Pattern> m_s_reversible[BLACK_AND_WHITE];
    std::vector<Pattern> m_t_reversible[BLACK_AND_WHITE];
    std::vector<Pattern> m_inferior[BLACK_AND_WHITE];
    std::vector<Pattern> m_captured[BLACK_AND_WHITE];
    std::vector<Pattern> m_vulnerable[BLACK_AND_WHITE];
    std::vector<Pattern> m_reversible[BLACK_AND_WHITE];
    // @}

    /** @name Hashed Paterns
        HashedPatternSets for each type of inferior cell. */
    // @{
    HashedPatternSet m_hashed_e_fillin;
    HashedPatternSet m_hashed_fillin[BLACK_AND_WHITE];
    HashedPatternSet m_hashed_s_reversible[BLACK_AND_WHITE];
    HashedPatternSet m_hashed_t_reversible[BLACK_AND_WHITE];
    HashedPatternSet m_hashed_inferior[BLACK_AND_WHITE];
    HashedPatternSet m_hashed_captured[BLACK_AND_WHITE];
    HashedPatternSet m_hashed_vulnerable[BLACK_AND_WHITE];
    HashedPatternSet m_hashed_reversible[BLACK_AND_WHITE];
    // @}
};

inline const HashedPatternSet& IcePatternSet::HashedEFillin() const
{
    return m_hashed_e_fillin;
}

inline const HashedPatternSet& 
IcePatternSet::HashedFillin(HexColor color) const
{
    return m_hashed_fillin[color];
}

inline const HashedPatternSet& 
IcePatternSet::HashedSReversible(HexColor color) const
{
    return m_hashed_s_reversible[color];
}

inline const HashedPatternSet& 
IcePatternSet::HashedTReversible(HexColor color) const
{
    return m_hashed_t_reversible[color];
}

inline const HashedPatternSet& 
IcePatternSet::HashedInferior(HexColor color) const
{
    return m_hashed_inferior[color];
}

inline const HashedPatternSet& 
IcePatternSet::HashedCaptured(HexColor color) const
{
    return m_hashed_captured[color];
}

inline const HashedPatternSet& 
IcePatternSet::HashedVulnerable(HexColor color) const
{
    return m_hashed_vulnerable[color];
}

inline const HashedPatternSet& 
IcePatternSet::HashedReversible(HexColor color) const
{
    return m_hashed_reversible[color];
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // ICE_PATTERN_SET_HPP
