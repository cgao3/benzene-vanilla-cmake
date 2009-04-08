#include "hexboard.h"

#include <cassert>
#include <cstring>
#include <cctype>

using std::endl;
using std::ws;
using std::ios;

//
// HexBoard::Iterator
//

static int normalOffsets[6][2] = { {0, -1}, {1, 0}, {0, 1},
                                   {-1, 1}, {-1, 0}, {0, -1} };

HexBoard::Iterator::Iterator()
{
  _isEnd = true;
}

HexBoard::Iterator::Iterator(const HexBoard *b, HexField center)
{
  _isEnd = false;
  _center = center;
  _board = b;
  _current = 0;
  if(_center == HexBoard::TOP_EDGE) {
    _x = (*_board).xs() - 1;
    _y = 0;
  } else if(_center == HexBoard::BOTTOM_EDGE) {
    _x = (*_board).xs() - 1;
    _y = (*_board).ys() - 1;
  } else if(_center == HexBoard::LEFT_EDGE) {
    _x = 0;
    _y = (*_board).ys() - 1;
  } else if(_center == HexBoard::RIGHT_EDGE) {
    _x = (*_board).xs() - 1;
    _y = (*_board).ys() - 1;
  } else {
    (*_board).field2Coords(_center, &_x, &_y);
    _x += normalOffsets[_current][0];
    _y += normalOffsets[_current][1];
  }
}

HexField HexBoard::Iterator::operator *() const
{
  assert(!_isEnd);
  return (*_board).coords2Field(_x, _y);
}

HexBoard::Iterator &HexBoard::Iterator::operator ++()
{
  assert(!_isEnd);
  if(_center == HexBoard::TOP_EDGE || _center == HexBoard::BOTTOM_EDGE) {
    if(_x)
      _x--;
    else
      _isEnd = true;
  } else if(_center == HexBoard::LEFT_EDGE ||
            _center == HexBoard::RIGHT_EDGE) {
    if(_y)
      _y--;
    else
      _isEnd = true;
  } else {
    if(_current < 5) {
      _current++;
      _x += normalOffsets[_current][0];
      _y += normalOffsets[_current][1];
    } else {
      _isEnd = true;
    }
  }
  return *this;
}

bool HexBoard::Iterator::operator ==(const HexBoard::Iterator &it) const
{
  return ((_isEnd && it._isEnd) ||
          (!_isEnd && !it._isEnd && _center == it._center &&
           _current == it._current));
}

bool HexBoard::Iterator::operator !=(const HexBoard::Iterator &it) const
{
  return !((*this) == it);
}

//
// HexBoard
//

HexBoard::Iterator HexBoard::_neighbourEnd;

HexBoard::HexBoard(int xs, int ys)
  : _xs(xs), _ys(ys), _size(_xs * _ys + FIRST_NORMAL_FIELD)
{
  assert(_xs > 0 && _ys > 0);
  _v = new HexMark[_size];
  int i;
  _v[TOP_EDGE] = _v[BOTTOM_EDGE] = HEX_MARK_VERT;
  _v[LEFT_EDGE] = _v[RIGHT_EDGE] = HEX_MARK_HORI;
  for(i = FIRST_NORMAL_FIELD; i < _size; i++) {
    _v[i] = HEX_MARK_EMPTY;
  }
}

HexBoard::HexBoard(const HexBoard &b) : _xs(b._xs), _ys(b._ys), _size(b._size)
{
  _v = new HexMark[_size];
  int i;
  for(i = 0; i < _size; i++) {
    _v[i] = b._v[i];
  }
}

HexBoard::~HexBoard()
{
  delete [] _v;
}

HexBoard &HexBoard::operator =(const HexBoard &b)
{
  delete [] _v;
  _xs = b._xs;
  _ys = b._ys;
  _size = b._size;
  _v = new HexMark[_size];
  int i;
  for(i = 0; i < _size; i++)
    _v[i] = b._v[i];
  return *this;
}

int HexBoard::size() const
{
  return _size;
}

int HexBoard::xs() const
{
  return _xs;
}

int HexBoard::ys() const
{
  return _ys;
}

HexMark HexBoard::get(HexField f) const
{
  assert(f >= 0 && f < _size);
  return _v[f];
}

void HexBoard::set(HexField f, HexMark m)
{
  assert(f >= FIRST_NORMAL_FIELD && f < _size);
  _v[f] = m;
}

bool HexBoard::isNormalField(HexField f) const
{
  return f >= FIRST_NORMAL_FIELD && f < _size;
}

bool HexBoard::isEmpty() const
{
  HexField f;
  for(f = FIRST_NORMAL_FIELD; f < _size; f++)
    if(get(f) != HEX_MARK_EMPTY)
      return false;
  return true;
}

int HexBoard::nMark() const
{
  HexField f;
  int r = 0;
  for(f = FIRST_NORMAL_FIELD; f < _size; f++)
    if(get(f) != HEX_MARK_EMPTY)
      r++;
  return r;
}

HexMark HexBoard::winner() const
{
  int *s = new int[_size];
  HexMark r = winner(s);
  delete [] s;
  return r;
}

int HexBoard::nNeighbours(int s[], HexField f) const
{
  int r = 0;
  Iterator neighbour = neighbourBegin(f);
  while(neighbour != neighbourEnd()) {
    if(s[*neighbour])
      r++;
    ++neighbour;
  }
  return r;
}

bool HexBoard::isConnected(int s[], HexField f, HexField to) const
{
  assert(s[f] == 2);
  if(f == to) {
    return true;
  } else {
    Iterator neighbour = neighbourBegin(f);
    while(neighbour != neighbourEnd()) {
      if(s[*neighbour] == 1) {
        s[*neighbour]++;
        bool c = isConnected(s, *neighbour, to);
        s[*neighbour]--;
        if(c)
          return true;
      }
      ++neighbour;
    }
    return false;
  }
}

bool HexBoard::isWinningPath(int s[]) const
{
  assert(s[HexBoard::TOP_EDGE] == 1 || s[HexBoard::LEFT_EDGE] == 1);
  HexField start, end;
  if(s[HexBoard::TOP_EDGE] == 1) {
    start = TOP_EDGE;
    end = BOTTOM_EDGE;
  } else {
    start = LEFT_EDGE;
    end = RIGHT_EDGE;
  }
  s[start]++;
  bool c = isConnected(s, start, end);
  s[start]--;
  return c;
}

pair<HexMark, vector<HexField> > HexBoard::winningPath() const
{
  int *s = new int[_size];
  pair<HexMark, vector<HexField> > r;
  r.first = winner(s);
  if(r.first != HEX_MARK_EMPTY) {
    // We got a winning path, but it's not minimal, so we try to remove
    // normal fields from it without cutting the connection between the edges.
    // If all normal fields that make up the path have 2 neighbours in the
    // path then the path is minimal.
    assert(isWinningPath(s));
    bool changed = true;
    while(changed) {
      changed = false;
      for(HexField f = FIRST_NORMAL_FIELD; f < _size; f++) {
        if(s[f]) {
          s[f] = 0;
          if(isWinningPath(s)) {
            changed = true;
          } else {
            s[f] = 1;
          }
        }
      }
    }
    for(HexField f = 0; f < _size; f++) {
      if(s[f])
        r.second.push_back(f);
    }
  }
  delete [] s;
  return r;
}

HexMark HexBoard::winner(int *expanded) const
{
  memset(expanded, 0, sizeof(int) * _size);
  if(expand(expanded, TOP_EDGE, BOTTOM_EDGE)) {
    return get(TOP_EDGE);
  } else {
    memset(expanded, 0, sizeof(int) * _size);
    if(expand(expanded, LEFT_EDGE, RIGHT_EDGE))
      return get(LEFT_EDGE);
    else
      return HEX_MARK_EMPTY;
  }
}

bool HexBoard::expand(int *s, HexField f, HexField goal) const
{
  HexMark m = get(f);
  assert(m != HEX_MARK_EMPTY);
  if(s[f]) {
    return false;
  } else {
    s[f] = 1;
    if(f == goal)
      return true;
    else {
      Iterator neighbour = neighbourBegin(f);
      while(neighbour != neighbourEnd()) {
        if(get(*neighbour) == m && expand(s, *neighbour, goal))
          return true;
        ++neighbour;
      }
      return false;
    }
  }
}

HexBoard HexBoard::transvert() const
{
  HexBoard r(_ys, _xs);
  int x, y;
  HexMark m;
  for(y = 0; y < _ys; y++) {
    for(x = 0; x < _xs; x++) {
      m = INVERT_HEX_MARK(get(coords2Field(x, y)));
      r.set(r.coords2Field(y, x), m);
    }
  }
  return r;
}

HexField HexBoard::transvert(const HexField &f) const
{
  int x, y;
  field2Coords(f, &x, &y);
  return coords2Field(y, x);
}

HexField HexBoard::coords2Field(int x, int y) const
{
  if(y < 0)
    return TOP_EDGE;
  else if(y >= _ys)
    return BOTTOM_EDGE;
  else if(x < 0)
    return LEFT_EDGE;
  else if(x >= _xs)
    return RIGHT_EDGE;
  else
    return FIRST_NORMAL_FIELD + y * _xs + x;
}

void HexBoard::field2Coords(HexField f, int *x, int *y) const
{
  assert(f >= FIRST_NORMAL_FIELD && f < _size);
  if(x)
    *x = ((f - FIRST_NORMAL_FIELD) % _xs);
  if(y)
    *y = ((f - FIRST_NORMAL_FIELD) / _xs);
}

bool HexBoard::adjacentFields(HexField f1, HexField f2) const
{
  Iterator cur = neighbourBegin(f1);
  Iterator end = neighbourEnd();
  while(cur != end) {
    if(*cur == f2)
      return true;
    ++cur;
  }
  return false;
}

HexBoard::Iterator HexBoard::neighbourBegin(HexField f) const
{
  return Iterator(this, f);
}

const HexBoard::Iterator &HexBoard::neighbourEnd() const
{
  return _neighbourEnd;
}

void HexBoard::printField(ostream &os, const HexField &f) const
{
  if(f == TOP_EDGE)
    os << "TOP_EDGE";
  else if(f == BOTTOM_EDGE)
    os << "BOTTOM_EDGE";
  else if(f == LEFT_EDGE)
    os << "LEFT_EDGE";
  else if(f == RIGHT_EDGE)
    os << "RIGHT_EDGE";
  else {
    int x, y;
    field2Coords(f, &x, &y);
    os << (char)('A' + x) << y + 1;
  }
}

bool HexBoard::operator ==(const HexBoard &b) const
{
  if(_xs != b._xs || _ys != b._ys)
    return false;
  for(int i = 0; i < _size; i++) {
    if(_v[i] != b._v[i])
      return false;
  }
  return true;
}

ostream &operator <<(ostream &os, const HexBoard &b)
{
  int x, y;
  int i;

  os << "  ";
  for(x = 0; x < b.xs(); x++) {
    os << (char)('A' + x) << " ";
  }
  os << endl;

  for(y = 0; y < b.ys(); y++) {
    for(i = 0; i < y; i++)
      os << " ";
    if(y < 9)
      os << " ";
    os << y + 1;
    for(x = 0; x < b.xs(); x++) {
      os << " ";
      os << b.get(b.coords2Field(x, y));
    }
    os << " " << y + 1 << endl;
  }

  for(i = 0; i < b.ys() + 3; i++)
    os << " ";
  for(x = 0; x < b.xs(); x++)
    os << (char)('A' + x) << " ";
  os << endl;

  return os;
}

// another ugly reading routine
istream &operator >>(istream &is, HexBoard &b)
{
  int xs = 0;
  char c;
  is >> ws;
  // read the "A B C D ..." header
  while(is && is.peek() == 'A' + xs) {
    is >> c;
    xs++;
    is >> ws;
  }
  // read lines
  vector<HexMark> marks;
  int ys = 0;
  int i;
  while(is) {
    is >> ws;
    if(isdigit(is.peek())) {
      // let's read the row number that starts a line
      is >> i;
      if(i == ys + 1) {
        ys++;
        HexMark m;
        int n = 0;
        while(is) {
          is >> ws;
          if(isdigit(is.peek())) {
            // let's read the row number that ends a line
            is >> i;
            if(i != ys)
              is.setstate(ios::failbit);
            break;
          } else {
            is >> m;
            n++;
            marks.push_back(m);
          }
        }
        if(n != xs) {
          is.setstate(ios::failbit);
          break;
        }
      } else {
        is.setstate(ios::failbit);
        break;
      }
    } else {
      break;
    }
  }
  if(!(xs > 0 && ys > 0))
    is.setstate(ios::failbit);
  // read the "A B C D ..." footer
  for(int i = 0; is && i < xs; i++) {
    char c;
    is >> ws;
    is >> c;
    if(c != 'A' + i) {
      is.setstate(ios::failbit);
      break;
    }
  }
  if(is) {
    HexBoard b0(xs, ys);
    assert((signed)marks.size() == b0.size() - HexBoard::FIRST_NORMAL_FIELD);
    for(HexField f = HexBoard::FIRST_NORMAL_FIELD; f < b0.size(); f++) {
      b0.set(f, marks[f - HexBoard::FIRST_NORMAL_FIELD]);
    }
    b = b0;
  }
  return is;
}
