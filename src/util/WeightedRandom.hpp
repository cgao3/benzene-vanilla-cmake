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
    WeightedRandom();

    WeightedRandom(int size);

    /** Sets all weights to 0.0f */
    void Clear();

    /** Access weight of p. O(1). 
        Changing this value does not cause tree to be updated. */
    float& operator[](int p);

    /** Access weight of p. O(1). */
    const float& operator[](int p) const;

    /** Sets weight of p and updates tree. O(lg(size)). */
    void SetWeightAndUpdate(int p, float w);

    /** Sets weight of p and updates total weight. O(1). */
    void SetWeight(int p, float w);

    /** Builds tree. O(size). */
    void Build();

    /** Returns sum of all leafs. */
    float Total() const;

    /** Select a leaf. O(lg(size)). */
    int Choose(SgRandom& random) const;

    /** Select a leaf. O(size). */
    int ChooseLinear(SgRandom& random) const;

private:
    int m_size;
    float* m_weights;
    void Init(int size);

    /** Non-copyable */
    WeightedRandom(const WeightedRandom& other);
    void operator=(const WeightedRandom& other);
};

inline float& WeightedRandom::operator[](int p)
{
    return m_weights[p + m_size];
}

inline const float& WeightedRandom::operator[](int p) const
{
    return m_weights[p + m_size];
}

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // WEIGHTEDRANDOM_HPP
