#include "carrier.h"

#include <algorithm>
#include <cassert>

static bool initialized = false;

int Carrier::bitsIn16[0x1u << 16];

static int bitcount (unsigned int n)  
{  
  int count = 0;
  while(n) {
    count++;
    n &= (n - 1);
  }
  return count;
}

void Carrier::init()
{
  for(unsigned int i = 0; i <= 0xffffu; i++)
    bitsIn16[i] = bitcount(i);
  initialized = true;
}

const vector<int> Carrier::fields() const
{
  vector<int> r;
  for(unsigned i = 0; i < VSIZE; i++) {
    for(unsigned b = 0; b < N_WORD_BITS; b++) {
      if(_v[i] & (((Word)1) << b)) {
        r.push_back(i * N_WORD_BITS + b);
      }
    }
  }
  return r;
}

ostream &operator <<(ostream &os, const Carrier &c)
{
  assert(initialized);
  vector<int> v;
  for(unsigned i = 0; i < Carrier::VSIZE; i++) {
    for(unsigned b = 0; b < Carrier::N_WORD_BITS; b++) {
      if(c._v[i] & (((Carrier::Word)1) << b)) {
        v.push_back(i * Carrier::N_WORD_BITS + b);
      }
    }
  }
  os << "[";
  for(unsigned int i = 0; i < v.size(); i++) {
    if(i)
      os << ", ";
    os << v[i];
  }
  os << "]";
  return os;
}
