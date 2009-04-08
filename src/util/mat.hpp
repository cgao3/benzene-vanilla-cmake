// Yo Emacs, this -*- C++ -*-
#ifndef MAT_H
#define MAT_H

#include "vec.hpp"
//#include "misc.h"

#include <cstring>
#include <iostream>
#include <cassert>

#define SWAP(a1, a2, tmp)  { (tmp) = (a1); (a1) = (a2); (a2) = (tmp); }

using std::istream;
using std::ostream;
using std::endl;

/**
 * Matrix of primitive numeric values.
 *
 * This implementation is built for speed.
 * It does not work with non-primitive types!
 */
template <class T>
class Mat
{
public:
  Mat();
  explicit Mat(int xs,int ys);
  Mat(const Mat &);
  ~Mat();

  void setSize(int xs,int ys);
  inline int xs() const;
  inline int ys() const;
  inline T &operator ()(int x,int y);
  inline const T &operator ()(int x,int y) const;

  Mat<T> operator ~() const;

  inline void swapRows(int r0, int r1);
  inline const T *data() const;
  inline T *data();

  Mat<T> &operator =(const Mat<T> &);
  Mat<T> operator +(const Mat<T> &) const;
  Mat<T> &operator +=(const Mat<T> &);
  Mat<T> operator -(const Mat<T> &) const;
  Mat<T> &operator -=(const Mat<T> &);
  Mat<T> operator *(const Mat<T> &) const;
  Mat<T> &operator *=(const Mat<T> &);
  Mat<T> operator -() const;

  Mat<T> &operator =(const T &);
  Mat<T> operator *(const T &) const;
  Mat<T> &operator *=(const T &);

  Vec<T> operator *(const Vec<T> &v) const;

  bool operator ==(const Mat<T> &) const;
  bool operator !=(const Mat<T> &) const;
  bool operator <(const Mat<T> &) const;
  bool operator <=(const Mat<T> &) const;
  bool operator >(const Mat<T> &) const;
  bool operator >=(const Mat<T> &) const;

  bool operator ==(const T &t) const;
  bool operator !=(const T &t) const;
  bool operator <(const T &t) const;
  bool operator <=(const T &t) const;
  bool operator >(const T &t) const;
  bool operator >=(const T &t) const;
private:
  int _xs,_ys;
  int _size;
  T *_v;
};

template<class T>
ostream &operator <<(ostream &os, const Mat<T> &m);

//
//				IMPLEMENTATION
//

template <class T>
Mat<T>::Mat() : _xs(0), _ys(0), _size(0), _v(0)
{
}

template <class T>
Mat<T>::Mat(int xs,int ys) : _xs(xs), _ys(ys), _size(_xs * _ys), _v(0)
{
  if(_size > 0)
    _v = new T[_size];
}

template <class T>
Mat<T>::Mat(const Mat &m) : _xs(m._xs), _ys(m._ys), _size(_xs * _ys),_v(0)
{
  if(_size > 0)
    _v = new T[_size];

  // this shortcut may not work with non-primitive types
  memcpy(_v, m._v, _size * sizeof(T));
  //for(int i = 0; i < _size; i++)
  //  _v[i] = m._v[i];
}

template <class T>
Mat<T>::~Mat()
{
  if(_v)
    delete [] _v;
}

template <class T>
inline void Mat<T>::setSize(int xs,int ys)
{
  if(_size != xs * ys) {
    if(_v)
      delete [] _v;
    _xs = xs;
    _ys = ys;
    _size = _xs * _ys;
    if(_size > 0)
      _v = new T[_size];
  }
}

template <class T>
inline int Mat<T>::xs() const
{
  return _xs;
}

template <class T>
inline int Mat<T>::ys() const
{
  return _ys;
}

template <class T>
inline T &Mat<T>::operator ()(int x,int y)
{
  assert(x >= 0 && x < _xs && y >= 0 && y < _ys);
  return _v[y * _xs + x];
}

template <class T>
inline const T &Mat<T>::operator ()(int x,int y) const
{
  assert(x >= 0 && x < _xs && y >= 0 && y < _ys);
  return _v[y * _xs + x];
}

template <class T>
Mat<T> Mat<T>::operator ~() const
{
  Mat<T> r(_ys, _xs);
  for(int x = 0; x < _xs; x++) {
    for(int y = 0; y <  _ys; y++)
      r(y, x) = (*this)(x, y);
  }
  return r;
}

template <class T>
void Mat<T>::swapRows(int r0, int r1)
{
  assert(0 <= r0 && r0 < _ys);
  assert(0 <= r1 && r1 < _ys);
  if(r0 != r1) {
    T tmp;
    T *v0 = _v + r0 * _xs;
    T *v1 = _v + r1 * _xs;
    for(int i = 0; i < _xs; i++)
      SWAP(v0[i], v1[i], tmp);
  }
}

template <class T>
const T *Mat<T>::data() const
{
  return _v;
}

template <class T>
T *Mat<T>::data()
{
  return _v;
}

template <class T>
Mat<T> &Mat<T>::operator =(const Mat<T> &m)
{
  if(_size != m._size) {
    if(_v)
      delete [] _v;
    _xs = m._xs;
    _ys = m._ys;
    _size = _xs * _ys;
    if(_size)
      _v = new T[_size];
  }
  // this shortcut may not work with non-primitive types
  memcpy(_v, m._v, _size * sizeof(T));
  //for(int i = 0; i < _size; i++)
  //  _v[i] = m._v[i];
  return *this;
}

template <class T>
Mat<T> Mat<T>::operator +(const Mat<T> &m) const
{
  assert(m._xs == _xs && m._ys == _ys);
  Mat<T> r(_xs,_ys);
  int x, y;
  for(x = 0; x < _xs; x++)
    for(y = 0; y < _ys; y++)
      r(x, y) -= (*this)(x, y) + m(x, y);
  return r;
}

template <class T>
Mat<T> &Mat<T>::operator +=(const Mat<T> &m)
{
  assert(m._xs == _xs && m._ys == _ys);
  int x, y;
  for(x = 0; x < _xs; x++)
    for(y = 0; y < _ys; y++)
      (*this)(x, y) += m(x, y);
  return *this;
}

template <class T>
Mat<T> Mat<T>::operator -(const Mat<T> &m) const
{
  assert(m._xs == _xs && m._ys == _ys);
  Mat<T> r(_xs,_ys);
  int x, y;
  for(x = 0; x < _xs; x++)
    for(y = 0; y < _ys; y++)
      r(x, y) -= (*this)(x, y) - m(x, y);
  return r;
}

template <class T>
Mat<T> &Mat<T>::operator -=(const Mat<T> &m)
{
  assert(m._xs == _xs && m._ys == _ys);
  int x, y;
  for(x = 0; x < _xs; x++)
    for(y = 0; y < _ys; y++)
      (*this)(x, y) -= m(x, y);
  return *this;
}

template <class T>
Mat<T> Mat<T>::operator *(const Mat<T> &m) const
{
  assert(_xs == m._ys);
  Mat<T> r(m._xs, _ys);
  int x, y,i;
  for(x = 0; x < r._xs; x++) {
    for(y = 0; y < r._ys; y++) {
      r(x, y) = 0;
      for(i = 0; i < _xs; i++)
        r(x, y) += (*this)(i, y) * m(x, i);
    }
  }
  return r;
}

template <class T>
Mat<T> &Mat<T>::operator *=(const Mat<T> &m)
{
  (*this) = (*this) * m;
  return *this;
}

template <class T>
Mat<T> Mat<T>::operator -() const
{
  Mat<T> r(_xs,_ys);
  for(int i = 0; i < _size; i++)
    r._v[i] = -_v[i];
  return r;
}

template <class T>
Mat<T> &Mat<T>::operator =(const T &t)
{
  for(int i = 0; i < _size; i++)
    _v[i] = t;
  return *this;
}

template <class T>
Mat<T> Mat<T>::operator *(const T &t) const
{
  Mat<T> r(_xs,_ys);
  int x, y;
  for(x = 0; x < _xs; x++)
    for(y = 0; y < _ys; y++)
      r(x, y) = (*this)(x, y) * t;
  return r;
}

template <class T>
Mat<T> &Mat<T>::operator *=(const T &t)
{
  int x, y;
  for(x = 0; x < _xs; x++)
    for(y = 0; y < _ys; y++)
      (*this)(x, y) *= t;
  return *this;
}

template <class T>
Vec<T> Mat<T>::operator *(const Vec<T> &v) const
{
  assert(_xs == v.size());
  Vec<T> r(_ys);
  for(int y = 0; y < _ys; y++) {
    r(y) = T(0);
    for(int x = 0; x < _xs; x++)
      r(y) += (*this)(x, y) * v(x);
  }
  return r;
}

template <class T>
bool Mat<T>::operator ==(const Mat<T> &m) const
{
  for(int i = 0; i < _size; i++) {
    if(!(_v[i] == m._v[i]))
      return false;
  }
  return true;
}

template <class T>
bool Mat<T>::operator !=(const Mat<T> &m) const
{
  for(int i = 0; i < _size; i++) {
    if(!(_v[i] != m._v[i]))
      return false;
  }
  return true;
}

template <class T>
bool Mat<T>::operator <(const Mat<T> &m) const
{
  assert(_xs == m._xs && _ys == m._ys);
  for(int i = 0; i < _size; i++) {
    if(!(_v[i] < m._v[i]))
      return false;
  }
  return true;
}

template <class T>
bool Mat<T>::operator <=(const Mat<T> &m) const
{
  assert(_xs == m._xs && _ys == m._ys);
  for(int i = 0; i < _size; i++) {
    if(!(_v[i] <= m._v[i])) 
      return false;
  }
  return true;
}

template <class T>
bool Mat<T>::operator >(const Mat<T> &m) const
{
  assert(_xs == m._xs && _ys == m._ys);
  for(int i = 0; i < _size; i++) {
    if(!(_v[i] > m._v[i]))
      return false;
  }
  return true;
}

template <class T>
bool Mat<T>::operator >=(const Mat<T> &m) const
{
  assert(_xs == m._xs && _ys == m._ys);
  for(int i = 0; i < _size; i++) {
    if(!(_v[i] >= m._v[i]))
      return false;
  }
  return true;
}

template <class T>
bool Mat<T>::operator ==(const T &t) const
{
  for(int i = 0; i < _size; i++) {
    if(!(_v[i] == t))
      return false;
  }
  return true;
}

template <class T>
bool Mat<T>::operator !=(const T &t) const
{
  for(int i = 0; i < _size; i++) {
    if(!(_v[i] != t))
      return false;
  }
  return true;
}

template <class T>
bool Mat<T>::operator <(const T &t) const
{
  for(int i = 0; i < _size; i++) {
    if(!(_v[i] < t))
      return false;
  }
  return true;
}

template <class T>
bool Mat<T>::operator <=(const T &t) const
{
  for(int i = 0; i < _size; i++) {
    if(!(_v[i] <= t))
      return false;
  }
  return true;
}

template <class T>
bool Mat<T>::operator >(const T &t) const
{
  for(int i = 0; i< _size; i++) {
    if(!(_v[i] > t))
      return false;
  }
  return true;
}

template <class T>
bool Mat<T>::operator >=(const T &t) const
{
  for(int i = 0; i < _size; i++) {
    if(!(_v[i] >= t))
      return false;
  }
  return true;
}

//
//
//

template <class T>
ostream &operator <<(ostream &os, const Mat<T> &m)
{
  os << "[" << endl;
  for(int y = 0; y < m.ys(); y++) {
    os << "[";
    for(int x = 0; x < m.xs(); x++) {
      if(x) os << ", ";
      os << m(x, y);
    }
    os << "]" << endl;
  }
  os << "]";
  return os;
}

#endif
