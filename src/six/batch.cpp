#include "batch.h"

//
//
//

ostream &operator <<(ostream &os, const GroupPair &gp)
{
  os << "(GroupPair " << *gp.minGroup() << " " 
     << *gp.maxGroup() << ")";
  return os;
}

//
//
//

Batch::Batch(Group *g0, Group *g1, unsigned softLimit, unsigned hardLimit)
  : GroupPair(g0, g1), _softLimit(softLimit), _hardLimit(hardLimit),
    _nUnprocessed(0), _processedIntersectionValid(true)
{
}

bool Batch::operator ==(const Batch &b) const
{
  if(size() != b.size())
    return false;
  BatchImpl::const_iterator cur = _carriers.begin();
  BatchImpl::const_iterator end = _carriers.end();
  while(cur != end) {
    BatchImpl::const_iterator bCur = b._carriers.begin();
    BatchImpl::const_iterator bEnd = b._carriers.end();
    while(bCur != bEnd && (*cur).carrier != (*bCur).carrier)
      ++bCur;
    if((*cur).carrier != (*bCur).carrier)
      return false;
    ++cur;
  }
  return true;
}

void Batch::setLimits(unsigned softLimit, unsigned hardLimit)
{
  assert(softLimit <= hardLimit);
  _softLimit = softLimit;
  _hardLimit = hardLimit;
  if(_carriers.size() > _hardLimit) {
    _carriers.resize(_hardLimit);
  }
  _nUnprocessed = (unsigned)-1;
  _processedIntersectionValid = false;
}

// bool Batch::add(const Carrier &c, bool processed)
// {
//   int cSize = c.size();
//   unsigned n = 0;
//   // We may quit before modifying anything, yet, so let's count in this var.
//   int nUnprocessed = 0;
//   BatchImpl::iterator cur = _carriers.begin();
//   BatchImpl::iterator end = _carriers.end();
//   // Loop over carriers of at most the same size and look for thinner ones.
//   while(cur != end && cSize >= (*cur).carrier.size()) {
//     if(c.includes((*cur).carrier)) {
//       return false;
//     }
//     if((!(*cur).processed) && (n < _softLimit))
//       ++nUnprocessed;
//     ++cur;
//     ++n;
//   }
//   if(n >= _hardLimit) {
//     _nUnprocessed = nUnprocessed;
//     return false;
//   } else {
//     // The rest of the carriers has bigger size than c => it cannot
//     // include any of them => so we can add it now.
//     _carriers.insert(cur, PCarrier(c, processed));
//     if(processed && n < _softLimit && _processedIntersectionValid) {
//       _processedIntersection.intersect(c);
// //       _processedIntersectionValid = false;
//     }
//     if(!processed && n < _softLimit)
//       ++nUnprocessed;
//     ++n;
//     // Kill the ones that include c.
//     while(cur != end) {
//       if((*cur).carrier.includes(c)) {
//         if(n < _softLimit && (*cur).processed)
//           _processedIntersectionValid = false;
//         cur = _carriers.erase(cur);
//       } else {
//         if((!(*cur).processed) && n < _softLimit)
//           ++nUnprocessed;
//         ++cur;
//         ++n;
//       }
//     }
//     _nUnprocessed = nUnprocessed;
//     return true;
//   }
// }

bool Batch::add(const Carrier &c, bool processed)
{
  int cSize = c.size();
  unsigned n = 0;
  BatchImpl::iterator cur = _carriers.begin();
  BatchImpl::iterator end = _carriers.end();
  // Loop over carriers of at most the same size and look for thinner ones.
  while(cur != end && cSize >= (*cur).carrier.size()) {
    if(c.includes((*cur).carrier)) {
      return false;
    }
    ++cur;
    ++n;
  }
  if(n >= _hardLimit) {
    return false;
  } else {
    // The rest of the carriers are at least the size of c so it cannot
    // include any of them, so we can add it now.
    _carriers.insert(cur, PCarrier(c, processed));
    // Kill the ones that include c.
    while(cur != end) {
      if((*cur).carrier.includes(c)) {
        cur = _carriers.erase(cur);
      } else {
        ++cur;
      }
    }
    _nUnprocessed = (unsigned)-1;
    _processedIntersectionValid = false;
    return true;
  }
}

inline Batch::Iterator Batch::erase(const Iterator &i)
{
  _nUnprocessed = (unsigned)-1;
  _processedIntersectionValid = false;
  return Iterator(_carriers.erase(i._current));
}

bool Batch::hasUnprocessed() const
{
  if(_nUnprocessed == (unsigned)-1) {
    // bypass const protection with vengeance
    unsigned n = 0;
    unsigned *nUnprocessed = (unsigned *)&_nUnprocessed;
    *nUnprocessed = 0;
    BatchImpl::const_iterator cur = _carriers.begin();
    BatchImpl::const_iterator end = _carriers.end();
    while(cur != end && n < _softLimit) {
      if(!(*cur).processed) {
        ++(*nUnprocessed);
        // For the time being, the exact number is not used, so it's
        // enough to know that there is at least one.
        break;
      }
      ++cur;
      ++n;
    }
  }
  return _nUnprocessed != 0;
}

bool Batch::includesAny(const Carrier &c) const
{
  // If it does not include the intersection of all carriers in the
  // batch, then it does not include any of them.
  // if(!c.includes(intersection()))
  //   return false;
  int size = c.size();
  BatchImpl::const_iterator cur = _carriers.begin();
  BatchImpl::const_iterator end = _carriers.end();
  while(cur != end && (*cur).carrier.size() <= size) {
    if(c.includes((*cur).carrier))
      return true;
    ++cur;
  }
  return false;
}

const Carrier &Batch::processedIntersection() const
{
  if(!_processedIntersectionValid) {
    Carrier *r = (Carrier *)&_processedIntersection;
    (*r).fill();
    unsigned n = 0;
    BatchImpl::const_iterator cur = _carriers.begin();
    BatchImpl::const_iterator end = _carriers.end();
    while(cur != end && n < _softLimit) {
      if((*cur).processed)
        (*r).intersect((*cur).carrier);
      ++cur;
      ++n;
    }
    (*(bool *)&_processedIntersectionValid) = true;
  }
  return _processedIntersection;
}

//
//
//

DualBatch::DualBatch(Group *g0, Group *g1,
                     unsigned softMaxConn, unsigned hardMaxConn,
                     unsigned softMaxSemi, unsigned hardMaxSemi)
  : GroupPair(g0, g1),
    _conns(g0, g1, softMaxConn, hardMaxConn),
    _semis(g0, g1, softMaxSemi, hardMaxSemi)
{
}

bool DualBatch::operator ==(const DualBatch &db) const
{
  return connBatch() == db.connBatch() && semiBatch() == db.semiBatch();
}

void DualBatch::addConn(const Carrier &c, bool processed)
{
  if(_conns.add(c, processed))
    erase(_semis, c);
}

void DualBatch::addSemi(const Carrier &c, bool processed)
{
  assert(!c.empty());
  if(!_conns.includesAny(c))
    _semis.add(c, processed);
}

void DualBatch::removeConn(const Batch::Iterator &i)
{
  _conns.erase(i);
}

void DualBatch::removeSemi(const Batch::Iterator &i)
{
  _semis.erase(i);
}

void DualBatch::setConnProcessed(Batch::Iterator &i)
{
  _conns.setProcessed(i);
}

void DualBatch::setSemiProcessed(Batch::Iterator &i)
{
  _semis.setProcessed(i);
}

bool DualBatch::hasUnprocessed() const
{
  return (_conns.hasUnprocessed() || _semis.hasUnprocessed());
//       return (_conns.hasUnprocessed() ||
//               (_semis.hasUnprocessed() &&
//                _conns.size() < _conns.softLimit()));
}

bool DualBatch::empty() const
{
  return _conns.empty() && _semis.empty();
}

void DualBatch::setConnLimits(unsigned softLimit, unsigned hardLimit)
{
  _conns.setLimits(softLimit, hardLimit);
}

void DualBatch::setSemiLimits(unsigned softLimit, unsigned hardLimit)
{
  _semis.setLimits(softLimit, hardLimit);
}

void DualBatch::erase(Batch &b, const Carrier &c)
{
  Batch::Iterator cur = b.begin();
  Batch::Iterator end = b.end();
  while(cur != end) {
    if((*cur).includes(c))
      cur = b.erase(cur);
    else
      ++cur;
  }
}
