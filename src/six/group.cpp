#include "group.h"

#include <algorithm>
#include <iterator>

using std::back_insert_iterator;
using std::insert_iterator;

//
// RMap
//

Grouping::RMap::RMap(int boardSize) : _v(boardSize)
{
  for(unsigned i = 0; i < _v.size(); i++)
    _v[i] = Poi<Group>(0);
}

Poi<Group> &Grouping::RMap::operator [](HexField f)
{
  assert(0 <= f && f < (int)_v.size());
  return _v[f];
}

const Poi<Group> &Grouping::RMap::operator [](HexField f) const
{
  assert(0 <= f && f < (int)_v.size());
  return _v[f];
}

//
// Grouping
//

void Grouping::addGroup(const Poi<Group> &g)
{
  const vector<HexField> &fields = (*g).fields();
  for(unsigned int i = 0; i < fields.size(); i++) {
    _rmap[fields[i]] = g;
  }
  if((*g).mark() == HEX_MARK_EMPTY)
    _emptyFields.unite((*g).coverage());
  _groups.push_back(g);
}

void Grouping::removeGroup(const Poi<Group> &g)
{
  const vector<HexField> &fields = (*g).fields();
  for(unsigned int i = 0; i < fields.size(); i++) {
    _rmap[fields[i]] = Poi<Group>(0);
  }
  if((*g).mark() == HEX_MARK_EMPTY)
    _emptyFields.remove((*g).coverage());
  _groups.erase(remove(_groups.begin(), _groups.end(), g), _groups.end());
}

set<Poi<Group> > Grouping::neighbouringGroups(const Poi<Group> &g) const
{
  set<Poi<Group> > r;
  const vector<HexField> &fields = (*g).fields();
  for(unsigned int i = 0; i < fields.size(); i++) {
    HexBoard::Iterator cur = _board.neighbourBegin(fields[i]);
    HexBoard::Iterator end = _board.neighbourEnd();
    while(cur != end) {
      Poi<Group> n = (*this)(*cur);
      if(!n.null() && (n != g))
        r.insert(n);
      ++cur;
    }
  }
  return r;
}

set<Poi<Group> > Grouping::removeIfNotEmptyGroup(const set<Poi<Group> > &g)
{
  set<Poi<Group> > r;
  for(set<Poi<Group> >::iterator cur = g.begin(); cur != g.end(); ++cur) {
    if((**cur).mark() == HEX_MARK_EMPTY) {
      r.insert(*cur);
    }
  }
  return r;
}

bool Grouping::hasAdjacentEmptyFields(const set<Poi<Group> > &emptyGroups)
{
  set<Poi<Group> >::iterator cur = emptyGroups.begin();
  set<Poi<Group> >::iterator end = emptyGroups.end();
  while(cur != end) {
    if((**cur).mark() == HEX_MARK_EMPTY) {
      set<Poi<Group> >::iterator cur2 = cur;
      ++cur2;
      while (cur2 != end) {
        if((**cur2).mark() == HEX_MARK_EMPTY &&
           _board.adjacentFields((**cur).fields()[0], (**cur2).fields()[0]))
          return true;
        ++cur2;
      }
    }
    ++cur;
  }
  return false;
}

bool Grouping::isDead(const Poi<Group> &g,
                      set<Poi<Group> > &directNeighbours,
                      set<Poi<Group> > &neighboursOfWouldBeGroup)
{
  assert(!g.null());
  if((*g).mark() != HEX_MARK_EMPTY) {
    // this is a non empty group, so all neighbouring groups must be empty
    directNeighbours = neighbouringGroups(g);
    neighboursOfWouldBeGroup.clear();
    // DEF: A non-empty group G is dead in position P iff for all P'
    // end positions reachable from P: P' has a winning group W => W -
    // G is still a winning group. In other words dead groups cannot
    // meaningfully affect the game.

    //assert: at most one edge in g
    bool isDead =
      // not a winning group and:
      (// do not kill the edges because others may depend on their presence
       //((*g).edge() && directNeighbours.size() == 0) || 
       (!(*g).edge() &&
        ((directNeighbours.size() < 2) ||
         ((directNeighbours.size() == 2) &&
          hasAdjacentEmptyFields(directNeighbours)))));
    //     if(isDead) {
    //       std::cout << _mark;
    //       _board.printField(std::cout, (*g).fields()[0]);
    //       std::cout << ": nNeigh=" << neighbours.size() 
    //                 << ", adj=" << hasAdjacentFields(neighbours)
    //                 << std::endl;
    //     }
    const vector<HexField> &fields = (*g).fields();
    for(unsigned int i = 0; i < fields.size(); i++) {
      _uselessFields.removeField(fields[i]);
    }
    return isDead;
  } else {
    directNeighbours = neighbouringGroups(g);
    unsigned int maxNeighbours = 0;
    neighboursOfWouldBeGroup.clear();
    insert_iterator<set<Poi<Group> > > oi(neighboursOfWouldBeGroup,
                                          neighboursOfWouldBeGroup.begin());
    int nEdges = 0;
    for(set<Poi<Group> >::iterator n = directNeighbours.begin();
        n != directNeighbours.end(); ++n) {
      if((**n).mark() == HEX_MARK_EMPTY) {
        neighboursOfWouldBeGroup.insert(*n);
      } else {
        set<Poi<Group> > neighboursOfN = neighbouringGroups(*n);
        if((**n).edge()) {
          ++nEdges;
        }
        // suppose there are two empty fields behind edges for now
        maxNeighbours = std::max((unsigned int)neighboursOfN.size() +
                                 ((**n).edge() ? 2 : 0),
                                 maxNeighbours);
        set_union(neighboursOfWouldBeGroup.begin(),
                  neighboursOfWouldBeGroup.end(),
                  neighboursOfN.begin(),
                  neighboursOfN.end(),
                  oi);
      }
    }
    neighboursOfWouldBeGroup.erase(g);
    unsigned int nEmpty = neighboursOfWouldBeGroup.size();
    bool isWinningMove = (nEdges == 2);
    // DEF: An empty group G is dead in position P iff the G' group
    // that were formed by playing at G would be dead or there is a
    // neighbouring group that loses liberty by this move.
    bool isDead = (!isWinningMove &&
                   ((nEdges && nEmpty == 0) ||
                    (!nEdges &&
                     ((nEmpty < 2) ||
                      ((nEmpty == 2) &&
                       hasAdjacentEmptyFields(neighboursOfWouldBeGroup)))) ||
                    nEmpty + nEdges * 2 < maxNeighbours));
    // DEF: An empty group G is useless in position P iff the G' group
    // that were formed by playing at G can be killed in one move.
    bool isUseless =
      (!isWinningMove && !isDead &&
       ((nEdges && nEmpty == 1) ||
        (!nEdges &&
         ((nEmpty < 3) ||
          ((nEmpty == 3) &&
           hasAdjacentEmptyFields(neighboursOfWouldBeGroup)))) ||
        nEmpty + nEdges * 2 <= maxNeighbours));
    HexField f = (*g).fields()[0];
    if(isUseless)
      _uselessFields.addField(f);
    else
      _uselessFields.removeField(f);
//     {
//       std::cout << _mark;
//       _board.printField(std::cout, (*g).fields()[0]);
//       std::cout << ": Dead=" << isDead
//                 << ", Useless=" << isUseless
//                 << ", nNeigh=" << directNeighbours.size() 
//                 << ", maxN=" << maxNeighbours
//                 << ", nEmpty=" << nEmpty
//                 << ", adj=" << hasAdjacentEmptyFields(neighboursOfWouldBeGroup)
//                 << std::endl;
//     }
    return isDead;
  }
}

void Grouping::killDeadGroups(deque<Poi<Group> > targets,
                              vector<Poi<Group> > *victims)
{
  set<Poi<Group> > directNeighbours;
  set<Poi<Group> > neighboursOfWouldBeGroup;
  while(!targets.empty()) {
    Poi<Group> g = targets.front();
    if(isDead(g, directNeighbours, neighboursOfWouldBeGroup)) {
      removeGroup(g);
      for(set<Poi<Group> >::iterator n = directNeighbours.begin();
          n != directNeighbours.end(); ++n) {
        if(find(targets.begin(), targets.end(), *n) == targets.end())
          targets.push_back(*n);
      }
      for(set<Poi<Group> >::iterator n = neighboursOfWouldBeGroup.begin();
          n != neighboursOfWouldBeGroup.end(); ++n) {
        if(find(targets.begin(), targets.end(), *n) == targets.end())
          targets.push_back(*n);
      }
      if(victims)
        (*victims).push_back(g);
    }
    targets.pop_front();
  }
}

Grouping::Grouping()
{
}

Grouping::Grouping(const HexBoard &board, HexMark mark)
{
  init(board, mark);
}

void Grouping::init(const HexBoard &b, HexMark mark)
{
  assert(mark != HEX_MARK_EMPTY);
  _board = b;
  _mark = mark;
  _groups.clear();
  _rmap = RMap(_board.size());
  for(HexField f = 0; f < b.size(); f++) {
    if(_rmap[f] == 0 && (b.get(f) == mark || b.get(f) == HEX_MARK_EMPTY)) {
      addGroup(Poi<Group>(new Group(b, f)));
    }
  }
  deque<Poi<Group> > targets;
  back_insert_iterator<deque<Poi<Group> > > oi(targets);
  copy(_groups.begin(), _groups.end(), oi);
  killDeadGroups(targets, 0);
}

Grouping::Change Grouping::move(HexField f, HexMark mark)
{
  assert(_board.isNormalField(f));
  assert(_mark != HEX_MARK_EMPTY);
  assert(_board.get(f) == HEX_MARK_EMPTY);
  _board.set(f, mark);
  Change r;
  r.emptyGroup = (*this)(f);
  // remove group at move
  if(!r.emptyGroup.null()) {
    removeGroup(r.emptyGroup);
  }
  if(mark == _mark) {
    // neighbours of the same color should be deleted -> add to r.second
    HexBoard::Iterator neighbour = _board.neighbourBegin(f);
    while(neighbour != _board.neighbourEnd()) {
      Poi<Group> neighbourGroup = (*this)(*neighbour);
      if(!neighbourGroup.null() && (*neighbourGroup).mark() == _mark) {
        if(find(r.unitedGroups.begin(), r.unitedGroups.end(), neighbourGroup)
           == r.unitedGroups.end()) {
          r.unitedGroups.push_back(neighbourGroup);
          removeGroup(neighbourGroup);
        }
      }
      ++neighbour;
    }
    r.newGroup = Poi<Group>(new Group(_board, f));
    addGroup(r.newGroup);
  }

  // It is unnecessary to examine all groups, but this is not a
  // performance bottleneck.
  deque<Poi<Group> > targets;
  back_insert_iterator<deque<Poi<Group> > > oi(targets);
  copy(_groups.begin(), _groups.end(), oi);
  killDeadGroups(targets, &r.deletedGroups);
  if(find(r.deletedGroups.begin(), r.deletedGroups.end(), r.newGroup) !=
     r.deletedGroups.end()) {
    r.newGroup = 0;
    r.deletedGroups.insert(r.deletedGroups.end(), r.unitedGroups.begin(),
                           r.unitedGroups.end());
    r.unitedGroups.clear();
  }

  return r;
}

int Grouping::size() const
{
  return _groups.size();
}

Poi<Group> Grouping::operator [](int n) const
{
  assert(0 <= n && (unsigned)n < _groups.size());
  return _groups[n];
}

Poi<Group> Grouping::operator ()(HexField f) const
{
  return _rmap[f];
}

Grouping::GroupIndex Grouping::groupIndex(const Group *g) const
{
  vector<Poi<Group> >::const_iterator i =
    find(_groups.begin(), _groups.end(), g);
  assert(i != _groups.end());
  return i - _groups.begin();
}

Grouping::GroupIndex Grouping::groupIndex(const Poi<Group> &g) const
{
  vector<Poi<Group> >::const_iterator i =
    find(_groups.begin(), _groups.end(), g);
  assert(i != _groups.end());
  return i - _groups.begin();
}

const HexBoard &Grouping::board() const
{
  return _board;
}

HexMark Grouping::mark() const
{
  return _mark;
}

const Carrier &Grouping::uselessFields() const
{
  return _uselessFields;
}

const Carrier &Grouping::emptyFields() const
{
  return _emptyFields;
}

ostream &operator <<(ostream &os, const Grouping &g)
{
  int x, y;
  int i;

  os << "  ";
  for(x = 0; x < g._board.xs(); x++) {
    os << (char)('A' + x) << " ";
  }
  os << std::endl;

  for(y = 0; y < g._board.ys(); y++) {
    for(i = 0; i < y; i++)
      os << " ";
    if(y < 9)
      os << " ";
    os << y + 1;
    for(x = 0; x < g._board.xs(); x++) {
      os << " ";
      const Poi<Group> r = g._rmap[g._board.coords2Field(x, y)];
      if(r.null()) {
        os << "*";
      } else {
        os << g.groupIndex(r);
      }
    }
    os << " " << y + 1 << std::endl;
  }

  for(i = 0; i < g._board.ys() + 3; i++)
    os << " ";
  for(x = 0; x < g._board.xs(); x++)
    os << (char)('A' + x) << " ";
  os << std::endl;

  return os;
}

//
// Group
//

Group::Group(const HexBoard &b, HexField f)
{
  _mark = b.get(f);
  _edge = false;
  _area = 0;

  expand(b, f);
}

void Group::add(const HexBoard &b, HexField f)
{
  _fields.push_back(f);
  _coverage.addField(f);
  if(f == HexBoard::TOP_EDGE || f == HexBoard::BOTTOM_EDGE) {
    _edge = true;
    _area += b.xs();
  } else if(f == HexBoard::LEFT_EDGE || f == HexBoard::RIGHT_EDGE) {
    _edge = true;
    _area += b.ys();
  } else {
    _area++;
  }
}

void Group::expand(const HexBoard &b, HexField f)
{
  if(!_coverage.has(f) && b.get(f) == _mark) {
    add(b, f);
    if(_mark != HEX_MARK_EMPTY) {
      HexBoard::Iterator neighbour = b.neighbourBegin(f);
      while(neighbour != b.neighbourEnd()) {
        expand(b, *neighbour);
        ++neighbour;
      }
    }
  }
}

ostream &operator <<(ostream &os, const Group &g)
{
  vector<HexField> fields = g. _fields;
  sort(fields.begin(), fields.end());
  os << g._mark << "{";
  for(unsigned i = 0; i < fields.size(); i++) {
    if(i)
      os << " ";
    os << fields[i];
  }
  os << "}";
  return os;
}
