#include "connector.h"
#include "misc.h"

#include <cassert>
#include <cmath>
#include <algorithm>

//
// added by broderic
//
extern int g_doOrs;
//////////////////

using std::cout;
using std::endl;

//
//
//

Connector::Connector(const Poi<DualBatchLimiter> &limiter,
                     unsigned maxInOrRule, bool useEdgePivot, 
                     bool includePivotInCarrier)
{
  _stop = false;
  _task = 0;

  _limiter = limiter;
  _maxInOrRule = maxInOrRule;
  _useEdgePivot = useEdgePivot;
  _includePivotInCarrier = includePivotInCarrier;
}

Connector::Connector(const Connector &c)
{
  assert(!c._stop);
  _stop = false;
  _task = 0;

  _limiter = c._limiter;
  _maxInOrRule = c._maxInOrRule;
  _useEdgePivot = c._useEdgePivot;
  _includePivotInCarrier = c._includePivotInCarrier;

  _groups = c._groups;
  _queue = c._queue;
  _map = c._map;
  initFanMap();
  _winner = c._winner;
  _connWinner = c._connWinner;
}

void Connector::setTask(SlicedTask *task)
{
  _task = task;
}

void Connector::stop()
{
  _stop = true;
}

bool Connector::stopped() const
{
  return _stop;
}

const Poi<Connector::DualBatchLimiter> &Connector::limiter() const
{
  return _limiter;
}

unsigned Connector::maxInOrRule() const
{
  return _maxInOrRule;
}

bool Connector::useEdgePivot() const
{
  return _useEdgePivot;
}

bool Connector::includePivotInCarrier() const
{
  return _includePivotInCarrier;
}

void Connector::enqueue(DualBatch &db)
{
  _queue.push_back(db);
}

void Connector::addToFanMap(DualBatch &db)
{
  Fan &minFan = _fanMap[db.minGroup()];
  if(std::find(minFan.begin(), minFan.end(), &db) == minFan.end())
    minFan.push_back(&db);
  Fan &maxFan = _fanMap[db.maxGroup()];
  if(std::find(maxFan.begin(), maxFan.end(), &db) == maxFan.end())
    maxFan.push_back(&db);
}

void Connector::initFanMap()
{
  _fanMap.clear();
  DualBatchMap::iterator mapCur = _map.begin();
  DualBatchMap::iterator mapEnd = _map.end();
  while(mapCur != mapEnd) {
    if(!(*mapCur).second.connBatch().empty())
      addToFanMap((*mapCur).second);
    ++mapCur;
  }
}

void Connector::processConnsForBatch(Group *x, Group *y, Group *middle,
                                     const vector<Carrier> &conns,
                                     const Batch &b)
{
  unsigned n = 0;
  bool toBeProcessed = false;
  Batch::Iterator cur = b.begin();
  Batch::Iterator end = b.end();
  vector<Carrier>::const_iterator vcur, vend;
  DualBatch *db = 0;
  while(cur != end && n < b.softLimit()) {
    if(b.processed(cur)) {
      vcur = conns.begin();
      vend = conns.end();
      while(vcur != vend) {
        toBeProcessed |= applyAnd(x, y, middle, *vcur, *cur, db);
        ++vcur;
      }
    }
    ++n;
    ++cur;
  }
  if(toBeProcessed) {
    assert(db);
    // It can happen that there is nothing to process. Adding conns
    // above the soft limit can make an unprocessed semi fall below
    // the soft limit then be deleted. But since it happens so
    // rarely and adding it does not hurt, it is always enqueued.
    enqueue(*db);
  }
}

void Connector::processConns(DualBatch &db, const vector<Carrier> &conns)
{
  assert(!conns.empty());
  Group *x0 = db.minGroup();
  Group *y0 = db.maxGroup();
  Group *x, *y, *middle;
  const Fan &xFan = _fanMap[x0];
  const Fan &yFan = _fanMap[y0];
  Fan::const_iterator fanCur = xFan.begin();
  Fan::const_iterator fanEnd = xFan.end();
  Carrier connsIntersection(conns[0]);
  for(unsigned i = 1; i < conns.size(); i++) {
    connsIntersection.intersect(conns[i]);
  }
  while(fanCur != fanEnd) {
    if(connsIntersection.disjunct((**fanCur).connBatch().
                                  processedIntersection()))
      if(batchConcatenatable(x0, y0,
                             (**fanCur).minGroup(), (**fanCur).maxGroup(),
                             &x, &y, &middle)) {
        processConnsForBatch(x, y, middle, conns, (**fanCur).connBatch());
    }
    ++fanCur;
  }
  fanCur = yFan.begin();
  fanEnd = yFan.end();
  while(fanCur != fanEnd) {
    if(connsIntersection.disjunct((**fanCur).connBatch().
                                  processedIntersection()))
      if(batchConcatenatable(x0, y0,
                             (**fanCur).minGroup(), (**fanCur).maxGroup(),
                             &x, &y, &middle)) {
        processConnsForBatch(x, y, middle, conns, (**fanCur).connBatch());
    }
    ++fanCur;
  }
}

// class CarrierIntersectionComparator
// {
// public:
//   CarrierIntersectionComparator(const Carrier &c) : _c(c)
//   {
//   }

//   bool operator ()(const Carrier &c0, const Carrier &c1) const
//   {
//     Carrier i0(_c);
//     i0.intersect(c0);
//     Carrier i1(_c);
//     i1.intersect(c1);
//     if(i0.size() < i1.size()) {
//       return true;
//     } else if(i0.size() > i1.size()) {
//       return false;
//     } else {
//       return c0.size() < c1.size();
//     }
//   }
// private:
//   const Carrier &_c;
// };

void Connector::processSemi(DualBatch &db, const Batch::Iterator &semi)
{
  assert(!db.semiBatch().processed(semi));

    ////////////////////
    // added by brderic
    g_doOrs++;
    ////////////////////


  vector<Carrier> semis = semisWithSamePath(db.semiBatch(), semi);
  if(semis.empty())
    return;

  // it seems that if the semis are sorted by carrier size
  // this sort is not worth it:
  // sort(semis.begin(), semis.end(), CarrierIntersectionComparator(*semi));

  vector<Carrier> tailIntersections(semis.size());
  Carrier semiUnion(*semi);
  tailIntersections.back() = semis.back();
  semiUnion.unite(semis.back());
  for(int i = ((signed)semis.size()) - 2; i >= 0; --i) {
    Carrier::setToIntersection(tailIntersections[i],
                               tailIntersections[i + 1], semis[i]);
    semiUnion.unite(semis[i]);
  }

  Carrier intersection(*semi);
  if(_includePivotInCarrier)
    intersection.intersect(_groups.emptyFields());
  intersection.intersect(tailIntersections[0]);
  if(!intersection.empty())
    return;

  {
    // One connection is going to be added for sure (semiUnion), so if
    // it has no conns yet then it will and it can be added to fan
    // map.
    if(db.connBatch().empty())
      addToFanMap(db);

    applyOr(db, semis.begin(), semis.end(), tailIntersections.begin(),
            *semi, *semi, 1);

    db.addConn(semiUnion);
    if(!db.connBatch().empty())
      setConnWinner(db);
    // No need to enqueue db, because all fresh connections are in
    // this DualBatch and those are going to be processed right after
    // the semis.
    //     if(db.hasUnprocessed())
    //       enqueue(db);
  }
}

bool Connector::batchConcatenatable(Group *x0, Group *y0, Group *x1, Group *y1,
                                    Group **x, Group **y, Group **middle)
{
  if(x0 == x1 && y0 != y1) {
    *middle = x0;
    *x = y0;
    *y = y1;
  } else if(x0 == y1 && y0 != x1) {
    *middle = x0;
    *x = y0;
    *y = x1;
  } else if(y0 == x1 && x0 != y1) {
    *middle = y0;
    *x = x0;
    *y = y1;
  } else if(y0 == y1 && x0 != x1) {
    *middle = y0;
    *x = x0;
    *y = x1;
  } else {
    return false;
  }
  return (_useEdgePivot || !(*middle)->edge());
}

bool Connector::applyAnd(Group *x, Group *y, Group *middle,
                         const Carrier &c0, const Carrier &c1,
                         DualBatch *&db)
{
  if(c0.disjunct(c1) &&
     !c0.has(x->fields()[0]) && !c0.has(y->fields()[0]) &&
     !c1.has(x->fields()[0]) && !c1.has(y->fields()[0])) {
    Carrier c(c0);
    c.unite(c1);
    if(db == 0) {
      GroupPair k(x, y);
      DualBatchMap::iterator dbi = _map.find(k);
      if(dbi == _map.end()) {
        dbi = _map.insert(pair<GroupPair, DualBatch>
                          (k, DualBatch(x, y))).first;
        (*_limiter).init((*dbi).second);
      }
      db = &(*dbi).second;
    }
    bool hadUnprocessed = (*db).hasUnprocessed();
    if(middle->mark() != HEX_MARK_EMPTY) {
      if((*db).connBatch().empty())
        addToFanMap(*db);
      if(_includePivotInCarrier)
        c.unite(middle->coverage());
      (*db).addConn(c);
      if(x->edge() && y->edge())
        _connWinner = _groups.mark();
      if(c.empty()) {
        (*db).setConnLimits(1, 1);
        (*db).setSemiLimits(0, 0);
      }
    } else {
      c.addField(middle->fields()[0]);
      (*db).addSemi(c);
    }
    return !hadUnprocessed && (*db).hasUnprocessed();
  }
  return false;
}

void Connector::applyOr(DualBatch &db,
                        vector<Carrier>::const_iterator semiCur,
                        vector<Carrier>::const_iterator semiEnd,
                        vector<Carrier>::const_iterator tailIntersectionCur,
                        const Carrier &un, const Carrier &in,
                        unsigned int depth)
{
  if(depth < _maxInOrRule) {
    Carrier newIn;
    while(semiCur != semiEnd) {
      if(Carrier::setToIntersection(newIn, in, *semiCur)) {
        Carrier newUn(un);
        newUn.unite(*semiCur);
        db.addConn(newUn);
        setConnWinner(db);
      } else {
        if(depth + 1 < _maxInOrRule && !(newIn == in) &&
           semiCur + 1 != semiEnd) {
          if(newIn.disjunct(*(tailIntersectionCur + 1))) {
            Carrier newUn(un);
            newUn.unite(*semiCur);
            if(!db.connBatch().includesAny(newUn)) {
              applyOr(db, semiCur + 1, semiEnd, tailIntersectionCur + 1,
                      newUn, newIn, depth + 1);
            }
          }
        }
      }
      ++semiCur;
      ++tailIntersectionCur;
    }
  }
}

vector<Carrier> Connector::semisWithSamePath(const Batch &b,
                                             const Batch::Iterator &i)
{
  Batch::Iterator cur = b.begin();
  Batch::Iterator end = b.end();
  vector<Carrier> r;
  unsigned n = 0;
  while(cur != end && n < b.softLimit()) {
    if(cur != i && b.processed(cur)) {
      r.push_back(*cur);
    }
    ++cur;
    ++n;
  }
  return r;
}

void Connector::init(const Grouping &grouping, bool doCalc)
{
  const Carrier emptyCarrier;
  _groups = grouping;
  _queue.clear();
  _map.clear();
  _fanMap.clear();
  _connWinner = _winner = board().winner();
  if(_winner == HEX_MARK_EMPTY) {
    for(int i = 0; i < _groups.size(); i++) {
      const Poi<Group> g = _groups[i];
      if((*g).mark() == HEX_MARK_EMPTY) {
        const set<Poi<Group> > &neighbours = _groups.neighbouringGroups(g);
        set<Poi<Group> >::const_iterator curNeighbour = neighbours.begin();
        set<Poi<Group> >::const_iterator endNeighbour = neighbours.end();
        while(curNeighbour != endNeighbour) {
          GroupPair k(&*g, &**curNeighbour);
          DualBatchMap::iterator db = _map.find(k);
          if(db == _map.end()) {
            db = _map.insert(pair<GroupPair, DualBatch>
                             (k, DualBatch(&*g, &**curNeighbour,
                                           1, 1, 0, 0))).first;
            addToFanMap((*db).second);
            enqueue((*db).second);
            (*db).second.addConn(emptyCarrier);
          }
          ++curNeighbour;
        }
      }
    }
  }
  if(doCalc)
    calc();
}

void Connector::init(const HexBoard &b, HexMark mark, bool doCalc)
{
  init(Grouping(b, mark), doCalc);
}

static bool included(const vector<Group *> &v, Group *p)
{
  for(unsigned int i = 0; i < v.size(); i++) {
    if(v[i] == p)
      return true;
  }
  return false;
}

enum TaintT { TAINT_NOTHING,
              TAINT_END_GROWN_TO, TAINT_END_GROWN, TAINT_END_CHANGED_TYPE,
              TAINT_FATAL };

typedef enum TaintT Taint;

static Taint untaint(const GroupPair &gp, GroupPair *untainted,
                     Group *ng, Group *eg,
                     const vector<Group *> &ug,
                     const vector<Group *> &dg)
{
  assert(untainted);
  if(included(dg, gp.minGroup()) || included(dg, gp.maxGroup())) {
    // kill it
    return TAINT_FATAL;
  } else if(!ng) {
    assert(ug.size() == 0);
    // we don't have a new group to untaint with
    if(gp.hasGroup(eg)) {
      // let's return the most serious one, so it gets deleted ...
      return TAINT_FATAL;
    }
    return TAINT_NOTHING;
  } else {
    // At this point, we know that there is a new group (ng), neither
    // of the end points was deleted.
    bool minTainted = (included(ug, gp.minGroup()) || eg == gp.minGroup());
    bool maxTainted = (included(ug, gp.maxGroup()) || eg == gp.maxGroup());
    if(minTainted || maxTainted) {
      if(minTainted && maxTainted) {
        // The end points would be the same.
        return TAINT_FATAL;
      } else {
        Group *x = (minTainted ? ng : gp.minGroup());
        Group *y = (maxTainted ? ng : gp.maxGroup());
        *untainted = GroupPair(x, y);
        if((minTainted && gp.minGroup()->mark() != x->mark()) ||
           (maxTainted && gp.maxGroup()->mark() != y->mark())) {
          return TAINT_END_CHANGED_TYPE;
        } else {
          assert(!ug.empty());
          if((minTainted && gp.minGroup() == ug.front()) ||
             (maxTainted && gp.maxGroup() == ug.front())) {
            // Of the united groups one need not be reprocessed. This
            // is the largest of the united groups.
            return TAINT_END_GROWN_TO;
          } else {
            return TAINT_END_GROWN;
          }
        }
      }
    } else {
      return TAINT_NOTHING;
    }
  }
}

void Connector::updateConnections(Group *newGroup, Group *emptyGroup,
                                  const vector<Group *> &unitedGroups,
                                  const vector<Group *> &deletedGroups)
{
  Carrier removedEmptyFields;
  if(emptyGroup)
    removedEmptyFields.addField(emptyGroup->fields()[0]);
  for(unsigned i = 0; i < deletedGroups.size(); i++) {
    if(deletedGroups[i]->mark() == HEX_MARK_EMPTY)
      removedEmptyFields.addField(deletedGroups[i]->fields()[0]);
  }
  _fanMap.clear();
  {
    GroupPair untainted(0, 0);
    DualBatchMap::iterator mapCur = _map.begin();
    DualBatchMap::iterator mapEnd = _map.end();
    DualBatchMap::iterator mapNext;
    while(mapCur != mapEnd) {
      mapNext = mapCur;
      ++mapNext;

      DualBatch &db = (*mapCur).second;
      Taint taint = untaint(db, &untainted, newGroup, emptyGroup,
                            unitedGroups, deletedGroups);
      assert(taint != TAINT_FATAL ||
             (untainted == db) == (taint == TAINT_NOTHING));
//       DBG << db << " -> " << taint << " -> ";
//       if(taint != TAINT_FATAL && taint != TAINT_NOTHING)
//         DBG << untainted;
//       DBG << std::endl;

      if(taint == TAINT_FATAL) {
        _map.erase(mapCur);
      } else if(taint == TAINT_NOTHING) {
        bool hadUnprocessed = db.hasUnprocessed();

        const Batch &conns = db.connBatch();
        Batch::Iterator connCur = conns.begin();
        Batch::Iterator connEnd = conns.end();
        Batch::Iterator connNext;
        while(connCur != connEnd) {
          if(!(*connCur).disjunct(removedEmptyFields)) {
            connNext = connCur;
            ++connNext;
            db.removeConn(connCur);
            connCur = connNext;
          } else {
            ++connCur;
          }
        }

        const Batch &semis = db.semiBatch();
        Batch::Iterator semiCur = semis.begin();
        Batch::Iterator semiEnd = semis.end();
        Batch::Iterator semiNext;
        while(semiCur != semiEnd) {
          if(!(*semiCur).disjunct(removedEmptyFields)) {
            semiNext = semiCur;
            ++semiNext;
            db.removeSemi(semiCur);
            semiCur = semiNext;
          } else {
            ++semiCur;
          }
        }
        if(db.empty()) {
          _map.erase(mapCur);
        } else {
          if(!hadUnprocessed && db.hasUnprocessed())
            enqueue(db);
          if(!db.connBatch().empty())
            addToFanMap(db);
        }
      } else {
        // Get or create target DualBatch.
        // Could processing this new DualBatch be avoided?
        DualBatchMap::iterator dbi = _map.find(untainted);
        if(dbi == _map.end()) {
          dbi = _map.insert(pair<GroupPair, DualBatch>
                            (untainted, DualBatch(untainted.minGroup(),
                                                  untainted.maxGroup()))).
            first;
          (*_limiter).init((*dbi).second);
        }
        DualBatch &db2 = (*dbi).second;
        bool hadConns = (!db2.connBatch().empty());
        bool hadUnprocessed = db2.hasUnprocessed();
        
        // ? If carrier is intruded by its own mark ?

        const Batch &conns = db.connBatch();
        Batch::Iterator connCur = conns.begin();
        Batch::Iterator connEnd = conns.end();
        while(connCur != connEnd) {
//           if(taint == TAINT_END_GROWN_TO) {
//             if(!(*connCur).disjunct(removedEmptyFields)) {
//               Carrier newCarrier(*connCur);
//               newCarrier.remove(removedEmptyFields);
//               db2.addConn(newCarrier, false);
//             } else {
//               db2.addConn((*connCur), conns.processed(connCur));
//             }
//           } else if(taint == TAINT_END_GROWN ||
//                     taint == TAINT_END_CHANGED_TYPE) {
//             if(!(*connCur).disjunct(removedEmptyFields)) {
//               Carrier newCarrier(*connCur);
//               newCarrier.remove(removedEmptyFields);
//               db2.addConn(newCarrier, false);
//             } else {
//               db2.addConn((*connCur), false);
//             }
//           }
          if((*connCur).disjunct(removedEmptyFields)) {
            if(taint == TAINT_END_GROWN_TO) {
              db2.addConn((*connCur), conns.processed(connCur));
            } else if(taint == TAINT_END_GROWN ||
                      taint == TAINT_END_CHANGED_TYPE) {
              db2.addConn((*connCur), false);
            }
          }
          ++connCur;
        }

        const Batch &semis = db.semiBatch();
        Batch::Iterator semiCur = semis.begin();
        Batch::Iterator semiEnd = semis.end();
        while(semiCur != semiEnd) {
//           if(taint == TAINT_END_GROWN_TO ||
//              (taint == TAINT_END_CHANGED_TYPE && unitedGroups.empty())) {
//             if(!(*semiCur).disjunct(removedEmptyFields)) {
//               Carrier newCarrier(*semiCur);
//               newCarrier.remove(removedEmptyFields);
//               if(!newCarrier.empty())
//                 db2.addSemi(newCarrier, false);
//             } else {
//               db2.addSemi((*semiCur), semis.processed(semiCur));
//             }
//           } else if(taint == TAINT_END_GROWN ||
//                     taint == TAINT_END_CHANGED_TYPE) {
//             if(!(*semiCur).disjunct(removedEmptyFields)) {
//               Carrier newCarrier(*semiCur);
//               newCarrier.remove(removedEmptyFields);
//               if(!newCarrier.empty())
//                 db2.addSemi(newCarrier, false);
//             } else {
//               db2.addSemi((*semiCur), false);
//             }
//           }
          if((*semiCur).disjunct(removedEmptyFields)) {
            if(taint == TAINT_END_GROWN_TO ||
               (taint == TAINT_END_CHANGED_TYPE && unitedGroups.empty())) {
              db2.addSemi((*semiCur), semis.processed(semiCur));
            } else if(taint == TAINT_END_GROWN ||
                      taint == TAINT_END_CHANGED_TYPE) {
              db2.addSemi((*semiCur), false);
            }
          }
          ++semiCur;
        }

        _map.erase(mapCur);
        if(db2.empty()) {
          _map.erase(db2);
        } else {
          (*_limiter).init(db2);
          if(!hadUnprocessed && db2.hasUnprocessed())
            enqueue(db2);
          if(!hadConns && !db2.connBatch().empty())
            addToFanMap(db2);
        }
      }
      mapCur = mapNext;
    }
  }
}

class GroupPAreaComparator
{
public:
  bool operator ()(const Group *g0, const Group *g1)
  {
    return g0->area() > g1->area();
  }
};

void Connector::move(const HexMove &move, bool doReinitOnEdge, bool doCalc)
{
  assert(move.isSwap() || move.isNormal());
  if(move.isSwap()) {
    HexBoard b(_groups.board().transvert());
    init(b, _groups.mark(), doCalc);
  } else {
    Grouping::Change change = _groups.move(move.field(), move.mark());
    Group *newGroup = change.newGroup.pointer();
    Group *emptyGroup = change.emptyGroup.pointer();
    assert(newGroup == 0 || newGroup->mark() == move.mark());
    assert(emptyGroup == 0 || emptyGroup->mark() == HEX_MARK_EMPTY);
    assert(change.unitedGroups.empty() || (_groups.mark() == move.mark()));
    bool touchesEdge = false;
    vector<Group *> unitedGroups;
    for(unsigned i = 0; i < change.unitedGroups.size(); i++) {
      unitedGroups.push_back(change.unitedGroups[i].pointer());
      if(unitedGroups.back()->edge())
        touchesEdge = true;
    }
    vector<Group *> deletedGroups;
    for(unsigned i = 0; i < change.deletedGroups.size(); i++) {
      deletedGroups.push_back(change.deletedGroups[i].pointer());
    }
    if(_useEdgePivot || !touchesEdge || (touchesEdge && !doReinitOnEdge)) {
      sort(unitedGroups.begin(), unitedGroups.end(), GroupPAreaComparator());
      updateConnections(newGroup, emptyGroup, unitedGroups, deletedGroups);
      _connWinner = _winner = _groups.board().winner();
      DualBatchMap::const_iterator wPaths = _map.find(winningGroupPair());
      if(wPaths != _map.end() && !(*wPaths).second.connBatch().empty()) {
        _connWinner = _groups.mark();
      }
      if(doCalc)
        calc();
    } else {
      HexBoard b(_groups.board());
      init(b, _groups.mark(), doCalc);
    }
  }
}

void Connector::setConnWinner(const GroupPair &k)
{
  if(k.minGroup()->edge() && k.maxGroup()->edge()) {
    // it cannot have the same edge at both ends
    // so it's a winning virtual connection
    _connWinner = _groups.mark();
    assert(_connWinner != HEX_MARK_EMPTY);
  }
}

void Connector::processSemiBatch(DualBatch &db)
{
  if(db.semiBatch().hasUnprocessed()) {
    unsigned n = 0;
    Batch::Iterator cur = db.semiBatch().begin();
    Batch::Iterator end = db.semiBatch().end();
    while(cur != end && n < db.semiBatch().softLimit()) {
      if(!db.semiBatch().processed(cur)) {
        processSemi(db, cur);
        db.setSemiProcessed(cur);
      }
      ++n;
      ++cur;
    }
  }
}

void Connector::processConnBatch(DualBatch &db)
{
  if(db.connBatch().hasUnprocessed()) {
    unsigned n = 0;
    vector<Carrier> unprocessedConns;
    Batch::Iterator cur = db.connBatch().begin();
    Batch::Iterator end = db.connBatch().end();
    while(cur != end && n < db.connBatch().softLimit()) {
      if(!db.connBatch().processed(cur)) {
        unprocessedConns.push_back(*cur);
        db.setConnProcessed(cur);
      }
      ++n;
      ++cur;
    }
    processConns(db, unprocessedConns);
  }
}

void Connector::processDualBatch(DualBatch &db)
{
  processSemiBatch(db);
  processConnBatch(db);
}

void Connector::calc()
{
    g_doOrs = 0;

  int n = 0;
  while(!_stop &&
        !_queue.empty() &&
        _winner == HEX_MARK_EMPTY && _connWinner == HEX_MARK_EMPTY) {
    const GroupPair &k = _queue.front();
    DualBatchMap::iterator db = _map.find(k);
    if(db != _map.end()) {
      processDualBatch((*db).second);
    }
    _queue.pop_front();

    if((n % 5) == 0 && _task) {
      _task->doSlice();
    }

    n++;
  }

  const Connector::DualBatchMap& dbm = connections();
  Connector::DualBatchMap::const_iterator cur = dbm.begin();
  Connector::DualBatchMap::const_iterator end = dbm.end();
        
  int fulls = 0;
  int semis = 0;
  while (cur != end) {
      DualBatch db = cur->second;
      fulls += db.connBatch().size();
      semis += db.semiBatch().size();
      ++cur;
  }
  
#if 0
  DBG << fulls << " fulls, " << semis << " semis; "
      << g_doOrs << " ors." << std::endl;
#endif
}

GroupPair Connector::winningGroupPair() const
{
  HexField e0 = ((_groups.mark() == HEX_MARK_VERT) ?
                 HexBoard::TOP_EDGE : HexBoard::LEFT_EDGE);
  HexField e1 = ((_groups.mark() == HEX_MARK_VERT) ?
                 HexBoard::BOTTOM_EDGE : HexBoard::RIGHT_EDGE);
  return GroupPair(&*_groups(e0), &*_groups(e1));
}

HexMark Connector::winner() const
{
  return _winner;
}

HexMark Connector::connWinner() const
{
  return _connWinner;
}

HexMark Connector::semiWinner() const
{
  DualBatchMap::const_iterator db = _map.find(winningGroupPair());
  if(db != _map.end() && !(*db).second.semiBatch().empty()) {
    return _groups.mark();
  } else {
    return HEX_MARK_EMPTY;
  }
}

Carrier Connector::winningConnCarrier() const
{
  assert(winner() == HEX_MARK_EMPTY);
  assert(connWinner() == _groups.mark());
  DualBatchMap::const_iterator db = _map.find(winningGroupPair());
  assert(db != _map.end());
  const Batch &b = (*db).second.connBatch();
  assert(!b.empty());
  // It might contain colored fields if _includePivotInCarrier.
  Carrier r(_groups.emptyFields());
  r.intersect(*b.begin());
  return r;
}

Carrier Connector::winningSemiCarrier() const
{
  assert(winner() == HEX_MARK_EMPTY);
  assert(semiWinner() == _groups.mark());
  DualBatchMap::const_iterator db = _map.find(winningGroupPair());
  assert(db != _map.end());
  const Batch &b = (*db).second.semiBatch();
  assert(!b.empty());
  // It might contain colored fields if _includePivotInCarrier.
  Carrier r(_groups.emptyFields());
  r.intersect(*b.begin());
  return r;
}

Carrier Connector::criticalPath() const
{
  assert(winner() == HEX_MARK_EMPTY);
  assert(connWinner() == HEX_MARK_EMPTY);
  DualBatchMap::const_iterator db = _map.find(winningGroupPair());
  assert(db != _map.end());
  const Batch &b = (*db).second.semiBatch();
  assert(!b.empty());
  Batch::Iterator cur = b.begin();
  Batch::Iterator end = b.end();
  // It might contain colored fields if _includePivotInCarrier.
  Carrier r(_groups.emptyFields());
  while(cur != end) {
    r.intersect((*cur));
    ++cur;
  }
  return r;
}

const Connector::DualBatchMap &Connector::connections() const
{
  return _map;
}

const Grouping &Connector::grouping() const
{
  return _groups;
}

const HexBoard &Connector::board() const
{
  return _groups.board();
}
