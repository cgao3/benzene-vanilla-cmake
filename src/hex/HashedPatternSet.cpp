//----------------------------------------------------------------------------
/** @file
 */
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

void HashedPatternSet::hash(const PatternSet& patterns)
{
    const std::vector<RingGodel>& valid_godels = RingGodel::ValidGodels();

    // hash each pattern rotation into the proper list
    for (unsigned i=0; i<patterns.size(); ++i) {
        const Pattern* pat = &patterns[i];

        for (int angle=0; angle<Pattern::NUM_SLICES; ++angle) {
            RotatedPattern rot(pat, angle);

            for (int h=0; h<(int)valid_godels.size(); ++h) {
                RingGodel godel = valid_godels[h];
                if (pat->RingGodel(angle).MatchesGodel(godel)) {
                    m_godel_list[h].push_back(rot);
                }
            }
        }
    }
}

//----------------------------------------------------------------------------
