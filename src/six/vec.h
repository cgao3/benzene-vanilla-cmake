// Yo Emacs, this -*- C++ -*-
#ifndef VEC_H
#define VEC_H

#include <cstring>
#include <iostream>
#include <cassert>

using std::istream;
using std::ostream;

/**
 * Vector of primitive numeric values.
 *
 * This implementation is built for speed.
 * It does not work with non-primitive types!
 */
template <class T>
class Vec
{
public:
  explicit Vec(int size = 0);
  Vec(const Vec &);
  ~Vec();

  void setSize(int size);
  inline int size() const;
  inline T &operator [](int n);
  inline const T &operator [](int n) const;
  inline T &operator ()(int n);
  inline const T &operator ()(int n) const;

  Vec<T> &operator =(const Vec<T> &);
  Vec<T> operator +(const Vec<T> &) const;
  Vec<T> &operator +=(const Vec<T> &);
  Vec<T> operator -(const Vec<T> &) const;
  Vec<T> &operator -=(const Vec<T> &);
  Vec<T> operator -() const;

  Vec<T> &operator =(const T &);
  Vec<T> operator *(const T &) const;
  Vec<T> &operator *=(const T &);

  bool operator ==(const Vec<T> &) const;
  bool operator !=(const Vec<T> &) const;
  bool operator <(const Vec<T> &) const;
  bool operator <=(const Vec<T> &) const;
  bool operator >(const Vec<T> &) const;
  bool operator >=(const Vec<T> &) const;

  bool operator ==(const T &t) const;
  bool operator !=(const T &t) const;
  bool operator <(const T &t) const;
  bool operator <=(const T &t) const;
  bool operator >(const T &t) const;
  bool operator >=(const T &t) const;
private:
  int _size;
  T *_v;
};

template<class T>
Vec<T> operator *(const T &t, const Vec<T> &v);

template<class T>
ostream &operator <<(ostream &os, const Vec<T> &v);

//
//				IMPLEMENTATION
//

template <class T>
Vec<T>::Vec(int size) : _size(size), _v(0)
{
  if(_size > 0)
    _v = new T[_size];
}

template <class T>
Vec<T>::Vec(const Vec &m) : _size(m._size), _v(0)
{
  if(_size > 0) {
    _v = new T[_size];
    // this shortcut may not work with non-primitive types
    memcpy(_v, m._v, _size * sizeof(T));
    //for(int i = 0; i < _size; i++)
    //  _v[i] = m._v[i];
  }
}

template <class T>
Vec<T>::~Vec()
{
  if(_v)
    delete [] _v;
}

template <class T>
inline void Vec<T>::setSize(int size)
{
  if(_size != size) {
    if(_v) {
      delete [] _v;
      _v = 0;
    }
    _size = size;
    if(_size > 0)
      _v = new T[_size];
  }
}

template <class T>
inline int Vec<T>::size() const
{
  return _size;
}

template <class T>
inline T &Vec<T>::operator [](int n)
{
  assert(n >= 0 && n < _size);
  return _v[n];
}

template <class T>
inline const T &Vec<T>::operator [](int n) const
{
  assert(n >= 0 && n < _size);
  return _v[n];
}

template <class T>
inline T &Vec<T>::operator ()(int n)
{
  assert(n >= 0 && n < _size);
  return _v[n];
}

template <class T>
inline const T &Vec<T>::operator ()(int n) const
{
  assert(n >= 0 && n < _size);
  return _v[n];
}

template <class T>
Vec<T> &Vec<T>::operator =(const Vec<T> &m)
{
  if(_size != m._size) {
    if(_v) {
      delete [] _v;
      _v = 0;
    }
    _size = m._size;
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
Vec<T> Vec<T>::operator +(const Vec<T> &m) const
{
  assert(_size == m._size);
  Vec<T> r(_size);
  for(int i = 0; i < _size; i++)
    r[i] = _v[i] + m._v[i];
  return r;
}

template <class T>
Vec<T> &Vec<T>::operator +=(const Vec<T> &m)
{
  assert(_size == m._size);
  Vec<T> r(_size);
  for(int i = 0; i < _size; i++)
    _v[i] += m._v[i];
  return *this;
}

template <class T>
Vec<T> Vec<T>::operator -(const Vec<T> &m) const
{
  assert(_size == m._size);
  Vec<T> r(_size);
  for(int i = 0; i < _size; i++)
    r[i] = _v[i] - m._v[i];
  return r;
}

template <class T>
Vec<T> &Vec<T>::operator -=(const Vec<T> &m)
{
  assert(_size == m._size);
  Vec<T> r(_size);
  for(int i = 0; i < _size; i++)
    _v[i] -= m._v[i];
  return *this;
}

template <class T>
Vec<T> Vec<T>::operator -() const
{
  Vec<T> r(_size);
  for(int i = 0; i < _size; i++)
    r[i] = -_v[i];
  return r;
}

template <class T>
Vec<T> &Vec<T>::operator =(const T &t)
{
  for(int i = 0; i < _size; i++)
    _v[i] = t;
  return *this;
}

template <class T>
Vec<T> Vec<T>::operator *(const T &t) const
{
  Vec<T> r(_size);
  for(int i = 0; i < _size; i++)
    r[i] = _v[i] * t;
  return r;
}

template <class T>
Vec<T> &Vec<T>::operator *=(const T &t)
{
  for(int i = 0; i < _size; i++)
    _v[i] *= t;
  return *this;
}

template <class T>
bool Vec<T>::operator ==(const Vec<T> &m) const
{
  assert(_size == m._size);
  for(int i = 0; i < _size; i++) {
    if(!(_v[i] == m._v[i]))
      return false;
  }
  return true;
}

template <class T>
bool Vec<T>::operator !=(const Vec<T> &m) const
{
  assert(_size == m._size);
  for(int i = 0; i < _size; i++) {
    if(_v[i] != m._v[i])
      return true;
  }
  return false;
}

template <class T>
bool Vec<T>::operator <(const Vec<T> &m) const
{
  assert(_size == m._size);
  for(int i = 0; i < _size; i++) {
    if(!(_v[i] < m._v[i]))
      return false;
  }
  return true;
}

template <class T>
bool Vec<T>::operator <=(const Vec<T> &m) const
{
  assert(_size == m._size);
  for(int i = 0; i < _size; i++) {
    if(!(_v[i] <= m._v[i]))
      return false;
  }
  return true;
}

template <class T>
bool Vec<T>::operator >(const Vec<T> &m) const
{
  assert(_size == m._size);
  for(int i = 0; i < _size; i++) {
    if(!(_v[i] > m._v[i]))
      return false;
  }
  return true;
}

template <class T>
bool Vec<T>::operator >=(const Vec<T> &m) const
{
  assert(_size == m._size);
  for(int i = 0; i < _size; i++) {
    if(!(_v[i] >= m._v[i]))
      return false;
  }
  return true;
}

template <class T>
bool Vec<T>::operator ==(const T &t) const
{
  for(int i = 0; i < _size; i++) {
    if(!(_v[i] == t))
      return false;
  }
  return true;
}

template <class T>
bool Vec<T>::operator !=(const T &t) const
{
  for(int i = 0; i < _size; i++) {
    if(_v[i] != t)
      return true;
  }
  return false;
}

template <class T>
bool Vec<T>::operator <(const T &t) const
{
  for(int i = 0; i < _size; i++) {
    if(!(_v[i] < t))
      return false;
  }
  return true;
}

template <class T>
bool Vec<T>::operator <=(const T &t) const
{
  for(int i = 0; i < _size; i++) {
    if(!(_v[i] <= t))
      return false;
  }
  return true;
}

template <class T>
bool Vec<T>::operator >(const T &t) const
{
  for(int i = 0; i < _size; i++) {
    if(!(_v[i] > t))
      return false;
  }
  return true;
}

template <class T>
bool Vec<T>::operator >=(const T &t) const
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
Vec<T> operator *(const T &t, const Vec<T> &v)
{
  Vec<T> r(v.size());
  for(int i = 0; i < r.size(); i++)
    r[i] = t * v[i];
  return r;
}

template <class T>
ostream &operator <<(ostream &os, const Vec<T> &v)
{
  os << "[";
  for(int i = 0; i < v.size(); i++) {
    if(i) os << ", ";
    os << v[i];
  }
  os << "]";
  return os;
}

#endif
