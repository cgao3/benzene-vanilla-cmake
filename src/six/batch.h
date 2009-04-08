// Yo Emacs, this -*- C++ -*-
#ifndef BATCH_H
#define BATCH_H

#include "carrier.h"
#include "group.h"

#ifdef __FreeBSD__
#include <machine/limits.h>
#define MAXINT INT_MAX
#else
//#include <values.h>
#include <limits.h>
#define MAXINT INT_MAX
#endif
#include <list>

/**
 * 
 */
class GroupPair
{
public:
  GroupPair(Group *g0, Group *g1)
    : _minGroup(std::min(g0, g1)), _maxGroup(std::max(g0, g1))
  {
  };
  bool operator <(const GroupPair &s) const
  {
    return (_minGroup < s._minGroup ||
            (_minGroup == s._minGroup && _maxGroup < s._maxGroup));
  };
  bool operator ==(const GroupPair &s) const
  {
    return (_minGroup == s._minGroup && _maxGroup == s._maxGroup);
  };
  Group *minGroup() const
  {
    return _minGroup;
  };
  Group *maxGroup() const
  {
    return _maxGroup;
  };
  bool hasGroup(Group *g) const
  {
    return _minGroup == g || _maxGroup == g;
  };
  friend class GroupPairHash;
  friend ostream &operator <<(ostream &, const GroupPair &);
protected:
  Group * _minGroup;
  Group * _maxGroup;
};

class GroupPairHash
{
public:
  size_t operator()(const GroupPair &s) const
  {
    return (size_t)s._minGroup + (size_t)s._maxGroup;
  }
};

class GroupPointerHash
{
public:
  size_t operator()(const Group *s) const
  {
    return (size_t)s;
  }
};

/**
 * A batch is a collection of subgames with the same end
 * points. Minimality of carriers within a batch is maintained.
 */
class Batch : public GroupPair
{
public:
  struct PCarrier
  {
    PCarrier() {};
    PCarrier(const Carrier &c, bool p) : carrier(c), processed(p) {};
    Carrier carrier;
    bool processed;
  };
private:
  typedef std::list<PCarrier> BatchImpl;
public:

  class Iterator
  {
  friend class Batch;
  private:
    Iterator(BatchImpl::iterator i) : _current(i) {}
  public:
    Iterator() {};
    const Carrier &operator *() const
    {
      return (*_current).carrier;
    };
    Iterator &operator ++()
    {
      ++_current;
      return *this;
    };
    bool operator ==(const Iterator &i) const
    {
      return _current == i._current;
    };
    bool operator !=(const Iterator &i) const
    {
      return _current != i._current;
    };
  private:
    BatchImpl::iterator _current;
  };

  Batch(Group *g0, Group *g1,
        unsigned softLimit = MAXINT, unsigned hardLimit = MAXINT);
  bool operator ==(const Batch &) const;
  unsigned softLimit() const { return _softLimit; };
  unsigned hardLimit() const { return _hardLimit; };
  void setLimits(unsigned softLimit, unsigned hardLimit);
  bool add(const Carrier &, bool processed = false);
  Iterator erase(const Iterator &);
  bool empty() const { return _carriers.empty(); };
  unsigned size() const { return _carriers.size(); };
  bool hasUnprocessed() const;
#define UNCONST(l) (*((BatchImpl *)(&l)))
  Iterator begin() const { return Iterator(UNCONST(_carriers).begin()); };
  Iterator end() const { return Iterator(UNCONST(_carriers).end()); };
  bool processed(const Iterator &i) const { return (*i._current).processed; };
  void setProcessed(Iterator &i)
  {
    if(!(*i._current).processed) {
      (*i._current).processed = true;
      _nUnprocessed = (unsigned)-1;
      _processedIntersectionValid = false;
    }
  };
  bool includesAny(const Carrier &c) const;
  const Carrier &processedIntersection() const;
private:
  unsigned _softLimit;
  unsigned _hardLimit;
  unsigned _nUnprocessed;
  BatchImpl _carriers;
  bool _processedIntersectionValid;
  Carrier _processedIntersection;
};


/**
 * A batch is a collection of connections and semi connections with
 * the same end points. It guarantees that a semi connection carrier
 * does not include a connection carrier.
 */
class DualBatch : public GroupPair
{
public:
  DualBatch(Group *g0, Group *g1,
            unsigned softMaxConn = MAXINT, unsigned hardMaxConn = MAXINT,
            unsigned softMaxSemi = MAXINT, unsigned hardMaxSemi = MAXINT);
  bool operator ==(const DualBatch &) const;
  void addConn(const Carrier &, bool processed = false);
  void addSemi(const Carrier &, bool processed = false);
  void removeConn(const Batch::Iterator &);
  void removeSemi(const Batch::Iterator &);
  const Batch &connBatch() const { return _conns; }
  const Batch &semiBatch() const { return _semis; }
  void setConnProcessed(Batch::Iterator &);
  void setSemiProcessed(Batch::Iterator &);
  bool hasUnprocessed() const;
  bool empty() const;
  void setConnLimits(unsigned softLimit, unsigned hardLimit);
  void setSemiLimits(unsigned softLimit, unsigned hardLimit);
private:
  // erase all carriers from batch that include c
  static void erase(Batch &, const Carrier &c);
  Batch _conns;
  Batch _semis;
};

#endif
