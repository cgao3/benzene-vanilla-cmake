//----------------------------------------------------------------------------
/** @file HashedPatternSet.cpp */
//----------------------------------------------------------------------------

#include "HashedPatternSet.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

HashedPatternSet::HashedPatternSet()
    : m_godel_list(RingGodel::ValidGodels().size())
{
}

HashedPatternSet::~HashedPatternSet()
{
}

const RotatedPatternList& 
HashedPatternSet::ListForGodel(const RingGodel& godel) const
{
    return m_godel_list[godel.Index()];
}

void HashedPatternSet::Hash(const PatternSet& patterns)
{
    const std::vector<RingGodel>& valid_godels = RingGodel::ValidGodels();
    // Hash each pattern rotation into the proper list
    for (std::size_t i = 0; i < patterns.size(); ++i) 
    {
        const Pattern* pat = &patterns[i];
        for (int angle = 0; angle < Pattern::NUM_SLICES; ++angle) 
        {
            RotatedPattern rot(pat, angle);
            for (std::size_t h = 0; h < valid_godels.size(); ++h) 
            {
                RingGodel godel = valid_godels[h];
                if (pat->RingGodel(angle).MatchesGodel(godel)) 
                    m_godel_list[h].push_back(rot);
            }
        }
    }
}

//----------------------------------------------------------------------------
