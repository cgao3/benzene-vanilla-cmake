// Yo Emacs, this -*- C++ -*-
#ifndef CARRIER_H
#define CARRIER_H

#include <iostream>
#include <vector>

using std::vector;
using std::istream;
using std::ostream;

#include <cassert>
#include <cstring>
#include "config.h"
#ifdef HAVE_STDINT_H
#include <stdint.h>
#else
#include <sys/types.h>
#endif

/**
 * A carrier is a set of fields on the board. Usually it is the set of
 * empty fields between the two ends of a subgame. It "carries" the
 * connection between the ends.
 *
 * It is just a set of integers, this implementation is intended to be
 * lightweight and specialized for the needs of this application.
 *
 * The size of the set is fixed: see @ref limit().
 */
class Carrier
{
private:
  typedef uint32_t Word;
public:
  /**
   * Initialize static data structures.
   */
  static void init();

  /**
   * Constructs an empty carrier.
   */
  Carrier() : _size(0)
  {
    memset(_v, 0, VSIZE_IN_BYTES);
  }

  /**
   * Copy constructor.
   */
  Carrier(const Carrier &c) : _size(c._size)
  {
    memcpy(_v, c._v, VSIZE_IN_BYTES);
  }

  /**
   * Assigns <code>c</code> to this carrier and returns a reference to this
   * carrier.
   */
  Carrier &operator =(const Carrier &c)
  {
    memcpy(_v, c._v, VSIZE_IN_BYTES);
    _size = c._size;
    return *this;
  }

  /**
   * Tests if this carrier is equal to <code>c</code>.
   * Two carriers are equal iff they contain the exact same fields.
   */
  bool operator ==(const Carrier &c) const
  {
    for(unsigned i = 0; i < VSIZE; i++) {
      if(_v[i] != c._v[i]) {
        return false;
      }
    }
    return true;
  }

  /**
   * Tests if this carrier is different from <code>c</code>.
   */
  bool operator !=(const Carrier &c) const
  {
    for(unsigned i = 0; i < VSIZE; i++) {
      if(_v[i] != c._v[i]) {
        return true;
      }
    }
    return false;
  }

  /**
   * Perfroms lexicographical comparison of the sorted lists of fields
   * in the two carriers.
   * Handy for use in sorted containers such as STL sets, maps.
   *
   * @return true iff this carrier is less than <code>c</code>.
   */
  bool operator <(const Carrier &c) const
  {
    for(unsigned i = 0; i < VSIZE; i++) {
      if(_v[i] != c._v[i]) {
        return _v[i] < c._v[i];
      }
    }
    return false;
  }

  /**
   * Remove all elements.
   */
  void clear()
  {
    memset(_v, 0, VSIZE_IN_BYTES);
    _size = 0;
  }

  /**
   * Add all elements.
   */
  void fill()
  {
    memset(_v, 255, VSIZE_IN_BYTES);
    _size = limit();
  }

  /**
   * Tests if this carrier - as a set - includes <code>c</code>.
   */
  bool includes(const Carrier &c) const
  {
    for(unsigned i = 0; i < VSIZE; i++) {
      if(c._v[i] & ~_v[i]) {
        return false;
      }
    }
    return true;
  }

  /**
   * Tests if this carrier is empty.
   */
  bool empty() const
  {
    return size() == 0;
  }

  /**
   * Returns the number of fields in this carrier.
   *
   * When called first this method is a bit expensive,
   * subsequent calls a very cheap.
   */
  int size() const
  {
    if(_size < 0) {
      // bypass const protection with vengeance
      int *s = (int *)&_size;
      *s = 0;
      for(unsigned i = 0; i < VSIZE; i++) {
        (*s) += (bitsIn16[_v[i] & 0xffffu] +
                 bitsIn16[(_v[i] >> 16) & 0xffffu]);
      }
    }
    return _size;
  }

  /**
   * The upper limit for fields that can be stored in this carrier.
   * Valid fields are the integers in the [0, limit()) range.
   */
  int limit() const
  {
    return N_WORD_BITS * VSIZE;
  }

  /**
   * Adds a fields to this carrier.
   */
  void addField(const int field)
  {
    assert(field >= 0);
    const unsigned int i = (field >> 5);
    const Word b = (((Word)1) << (field & 31));
    assert(i < VSIZE);
    _v[i] |= b;
    _size = -1;
  }

  /**
   * Removes a fields from this carrier.
   */
  void removeField(const int field)
  {
    assert(field >= 0);
    const unsigned int i = (field >> 5);
    const Word b = (((Word)1) << (field & 31));
    assert(i < VSIZE);
    _v[i] &= (~b);
    _size = -1;
  }

  /**
   * Tests if <code>field</code> is in this carrier.
   */
  bool has(const int field) const
  {
    const unsigned int i = (field >> 5);
    const Word b = (((Word)1) << (field & 31));
    assert(i < VSIZE);
    return _v[i] & b;
  }

  /**
   * Constructs a vector of fields from the fields in this carrier.
   */
  const vector<int> fields() const;

  /**
   * Calculates the union of this carrier and <code>c</code> in place.
   */
  void unite(const Carrier &c)
  {
    for(register unsigned i = 0; i < VSIZE; i++) {
      _v[i] |= c._v[i];
    }
    _size = -1;
  }

  /**
   * Calculates the intersection of this carrier and <code>c</code> in place.
   */
  void intersect(const Carrier &c)
  {
    for(unsigned i = 0; i < VSIZE; i++) {
      _v[i] &= c._v[i];
    }
    _size = -1;
  }

  /**
   * Removes the fields in <code>c</code> in place.
   */
  void remove(const Carrier &c)
  {
    for(unsigned i = 0; i < VSIZE; i++) {
      _v[i] &= ~c._v[i];
    }
    _size = -1;
  }

  /**
   * Checks if this carrier and <code>c</code> are disjunct.
   */
  bool disjunct(const Carrier &c) const
  {
    for(unsigned i = 0; i < VSIZE; i++) {
      if(_v[i] & c._v[i]) {
        return false;
      }
    }
    return true;
  }

  static bool setToIntersection(Carrier &target,
                                const Carrier &c1, const Carrier &c2)
  {
    target._size = 0;
    for(unsigned i = 0; i < VSIZE; i++) {
      target._v[i] = (c1._v[i] & c2._v[i]);
      target._size += (bitsIn16[target._v[i] & 0xffffu] +
                       bitsIn16[(target._v[i] >> 16) & 0xffffu]);
    }
    return target._size == 0;
  }

  static bool setToUnion(Carrier &target,
                         const Carrier &c1, const Carrier &c2)
  {
    target._size = 0;
    for(unsigned i = 0; i < VSIZE; i++) {
      target._v[i] = (c1._v[i] | c2._v[i]);
      target._size += (bitsIn16[target._v[i] & 0xffffu] +
                       bitsIn16[(target._v[i] >> 16) & 0xffffu]);
    }
    return target._size == 0;
  }

  /**
   * Writes <code>c</code> in the form of <code>[0,3,4,7]</code>
   * to <code>os</code>.
   */
  friend ostream &operator <<(ostream &os, const Carrier &c);
private:
  static int bitsIn16[0x1u << 16];
  static const unsigned int N_WORD_BITS = (sizeof(Word) * 8);
#ifdef OLYMPICS
  static const unsigned int VSIZE = 4;
#else
  static const unsigned int VSIZE = 8;
#endif
  static const unsigned int VSIZE_IN_BYTES = sizeof(Word) * VSIZE;
  Word _v[VSIZE];
  // The result of the last size() calculation is in _size,
  // if there was no size() call or the carrier was modified
  // since then it is negative.
  int _size;
};

#endif
