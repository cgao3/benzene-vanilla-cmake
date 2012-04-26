//----------------------------------------------------------------------------
/** @file WeightedRandom.hpp */
//----------------------------------------------------------------------------

#ifndef WEIGHTEDRANDOM_HPP
#define WEIGHTEDRANDOM_HPP

#include <vector>
#include "Benzene.hpp"

class SgRandom;

_BEGIN_BENZENE_NAMESPACE_

//----------------------------------------------------------------------------

/** Weighted random selection.
    Weights are arbitrary floats greater than 0. Selection and updates
    in O(lg(size)) time. */
class WeightedRandom
{
public:
    WeightedRandom(int size);

    /** Sets all weights to 0.0f */
    void Clear();

    /** Access weight of p. O(1). 
        Changing this value does not cause tree to be updated. */
    float& operator[](int p);

    /** Sets weight of p and updates tree. O(lg(size)). */
    void SetWeightAndUpdate(int p, float w);

    /** Builds tree. O(size). */
    void Build();

    /** Returns sum of all leafs. */
    float Total() const;

    /** Select a leaf. O(lg(size)). */
    int Choose(SgRandom& random);

private:
    int m_size;
    std::vector<float> m_weights;
};

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // WEIGHTEDRANDOM_HPP
