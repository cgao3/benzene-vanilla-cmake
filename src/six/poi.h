// Yo Emacs, this -*- C++ -*-
#ifndef POI_H
#define POI_H

#include <cassert>

/**
 * Reference counted pointer to elements of type <code>T</code>.
 *
 * Very handy for objects with expensive copy constructors,
 * large objects, or in any other case when creation of a new object is
 * too expensive or impossible, and in cases when safe sharing of
 * objects is required.
 *
 * An object of a class without default and copy constructors can be
 * wrapped in a Poi and used in value based containers.
 */
template <class T>
class Poi
{
public:
  /**
   * Constructs a pointer.
   */
  explicit Poi(T *t = 0);

  /**
   * Copies pointer <code>p</code>.
   * If not null, both pointers will point to the same object
   * and their reference counter will be incremented.
   */
  Poi(const Poi<T> &p);

  /**
   * Destroys this pointer, if not null decreases the reference counter
   * and frees the object it points to if neccesary.
   */
  ~Poi();

  /**
   * The assignment operator releases its current reference and copies
   * the reference of <code>p</code>.
   */
  Poi<T> &operator =(const Poi<T> &p);

  /**
   * The assignment operator releases its current reference and
   * creates a new reference.
   */
  Poi<T> &operator =(T *t);

  /**
   * Pointer emulation.
   */
  T &operator *() const;

  /**
   * @return true iff this is a null pointer
   */
  bool null() const;

  /**
   * Just like with normal pointers, this method tests if the two pointers
   * point to the same object.
   */
  bool operator ==(const Poi<T> &) const;
  bool operator ==(const T *) const;

  /**
   * Just like with normal pointers, this method tests if the two pointers
   * point to the different objects.
   */
  bool operator !=(const Poi<T> &) const;
  bool operator !=(const T *) const;

  /**
   * Compares the real pointers.
   */
  bool operator <(const Poi<T> &) const;
  bool operator <(const T *) const;

  /**
   * @return the number of references to its current value;
   * 0 for null pointer, at least 1 for non-null pointers
   */
  int nRef() const;

  /**
   *
   */
  const T *pointer() const;
  T *pointer();
private:
  void newReference(T *);
  void copyReference(const Poi<T> &);
  void releaseReference();
  T *v;
  int *count;
};

//
//                              IMPLEMENTATION
//

template <class T>
Poi<T>::Poi(T *t)
{
  newReference(t);
}

template <class T>
Poi<T>::Poi(const Poi<T> &r)
{
  copyReference(r);
}

template <class T>
Poi<T>::~Poi()
{
  releaseReference();
}

template <class T>
Poi<T> &Poi<T>::operator =(const Poi<T> &r)
{
  if(v != r.v) {
    releaseReference();
    copyReference(r);
  }
  return *this;
}

template <class T>
Poi<T> &Poi<T>::operator =(T *t)
{
  releaseReference();
  newReference(t);
  return *this;
}

template <class T>
inline T &Poi<T>::operator *() const
{
  assert(v);
  return *v;
}

template <class T>
inline bool Poi<T>::null() const
{
  return v == 0;
}

template <class T>
bool Poi<T>::operator ==(const Poi<T> &p) const
{
  return v == p.v;
}

template <class T>
bool Poi<T>::operator ==(const T *p) const
{
  return v == p;
}

template <class T>
bool Poi<T>::operator !=(const Poi<T> &p) const
{
  return v != p.v;
}

template <class T>
bool Poi<T>::operator <(const Poi<T> &p) const
{
  return v < p.v;
}

template <class T>
bool Poi<T>::operator <(const T *p) const
{
  return v < p;
}

template <class T>
bool Poi<T>::operator !=(const T *p) const
{
  return v != p;
}

template <class T>
int Poi<T>::nRef() const
{
  if(count == 0) {
    return 0;
  } else {
    return *count;
  }
}

template <class T>
const T *Poi<T>::pointer() const
{
  return v;
}

template <class T>
T *Poi<T>::pointer()
{
  return v;
}

template <class T>
void Poi<T>::newReference(T *t)
{
  v = t;
  if(v) {
    count = new int(1);
  } else {
    count = 0;
  }
}

template <class T>
void Poi<T>::copyReference(const Poi<T> &r)
{
  v = r.v;
  count = r.count;
  if(count) {
    ++*count;
  }
}

template <class T>
void Poi<T>::releaseReference()
{
  if(v) {
    --*count;
    if(*count == 0) {
      delete count;
      count = 0;
      delete v;
      v = 0;
    }
  }
}

#endif
