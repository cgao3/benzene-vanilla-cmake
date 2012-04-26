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

class WeightedRandom
{
public:
    WeightedRandom(int size, SgRandom& random);

    void Clear();

    void InitWeight(int p, float w);

    void SetWeight(int p, float w);

    void Build();

    float Total() const;

    int Choose();

private:
    int m_size;
    std::vector<float> m_weights;
    SgRandom& m_random;
};

//----------------------------------------------------------------------------

_END_BENZENE_NAMESPACE_

#endif // WEIGHTEDRANDOM_HPP
