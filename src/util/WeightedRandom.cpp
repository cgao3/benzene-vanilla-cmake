//----------------------------------------------------------------------------
/** @file WeightedRandom.cpp */
//----------------------------------------------------------------------------

#include "SgSystem.h"
#include "SgRandom.h"
#include "WeightedRandom.hpp"
#include "Logger.hpp"

using namespace benzene;

//----------------------------------------------------------------------------

WeightedRandom::WeightedRandom(int size, SgRandom& random)
    : m_random(random)
{
    if (size > 256) size = 512;
    else if (size > 128) size = 256;
    else size = 128;
    m_size = size;
    m_weights.resize(2 * m_size);
    Clear();
}

void WeightedRandom::Clear()
{
    for (int i = 0; i < 2 * m_size; ++i)
        m_weights[i] = 0.0f;
}

void WeightedRandom::InitWeight(int p, float w)
{
    m_weights[p + m_size] = w;
}

void WeightedRandom::SetWeight(int p, float w)
{
    p += m_size;
    m_weights[p] = w;
    while (p /= 2)
        m_weights[p] = m_weights[2*p] + m_weights[2*p + 1];
}

void WeightedRandom::Build()
{
    for (int i = m_size - 1; i >= 1; --i)
        m_weights[i] = m_weights[2*i] + m_weights[2*i + 1];
}

float WeightedRandom::Total() const
{
    return m_weights[1];
}

int WeightedRandom::Choose()
{
    int i;
    do {
        float r = m_random.Float(m_weights[1]);
        i = 2;
        while (i < m_size)
        {
            /*
            LogInfo() << "i=" << i << " r=" << r 
                      << " w[" << i << "]=" << m_weights[i] 
                      << " w[" << i+1 << "]=" << m_weights[i+1]
                      << '\n';
            */
            if (r > m_weights[i]) 
            {
                r -= m_weights[i];
                i++;
            }
            i *= 2;
        }
        if (r > m_weights[i])
            i++;
    } while (m_weights[i] <= 1e-7);
    return i - m_size;
}

//----------------------------------------------------------------------------
