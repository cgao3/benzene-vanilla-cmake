#include "hexgame.h"

#include <cassert>
#include <cctype>
#include <cstdio>
#include <cstdlib>

#include <string>

using std::endl;
using std::string;
using std::ios;
using std::ws;

//
// HexGame::MoveLogEntry
//

HexGame::MoveLogEntry::MoveLogEntry()
{
}

HexGame::MoveLogEntry::MoveLogEntry(const HexMove &m, long clock,
                                    const HexGame::MoveLogEntry *prevEntry)
  : _move(m)
{
  if(_move.mark() == HEX_MARK_VERT) {
    _vertClock = clock;
    _horiClock = 0;
  } else {
    _vertClock = 0;
    _horiClock = clock;
  }
  if(prevEntry) {
    _vertClockTotal = (*prevEntry).vertClockTotal() + _vertClock;
    _horiClockTotal = (*prevEntry).horiClockTotal() + _horiClock;
  } else {
    _vertClockTotal = _vertClock;
    _horiClockTotal = _horiClock;
  }
}

long HexGame::MoveLogEntry::clock() const
{
  if(_move.mark() == HEX_MARK_VERT)
    return _vertClock;
  else
    return _horiClock;
}

long HexGame::MoveLogEntry::vertClock() const
{
  return _vertClock;
}

long HexGame::MoveLogEntry::horiClock() const
{
  return _horiClock;
}

long HexGame::MoveLogEntry::vertClockTotal() const
{
  return _vertClockTotal;
}

long HexGame::MoveLogEntry::horiClockTotal() const
{
  return _horiClockTotal;
}

const HexMove &HexGame::MoveLogEntry::move() const
{
  return _move;
}

//
// HexGame
//

HexGame::HexGame(const HexGameState &initialState)
  : _initialState(initialState), _currentState(initialState),
    _nextEntry(0), _isChanged(false), _isBranched(false)
{
}

HexGame::HexGame(const HexBoard &b, HexMark next, bool swappable)
  : _initialState(b, next, swappable), _currentState(b, next, swappable),
    _nextEntry(0), _isChanged(false), _isBranched(false)
{
}

const HexGameState &HexGame::initialState() const
{
  return _initialState;
}

const HexGameState &HexGame::currentState() const
{
  return _currentState;
}

const HexBoard &HexGame::board() const
{
  return _currentState.board();
}

HexMark HexGame::next() const
{
  return _currentState.next();
}

bool HexGame::swappable() const
{
  return _currentState.swappable();
}

bool HexGame::isValidMove(const HexMove &move) const
{
  return _currentState.isValidMove(move);
}

HexMark HexGame::winner() const
{
  return _currentState.winner();
}

void HexGame::play(const HexMove &move, long milliseconds)
{
  const MoveLogEntry *prev =
    ((_nextEntry > 0) ? &_moveLog[_nextEntry - 1] : 0);
  MoveLogEntry m(move, milliseconds, prev);
  if((signed)_moveLog.size() != _nextEntry) {
    _moveLog.resize(_nextEntry);
    _isBranched = true;
  }
  _moveLog.push_back(m);
  _nextEntry++;
  _currentState.play(move);
  _isChanged = true;
}

void HexGame::printMove(ostream &os, const HexMove &m, bool withMark) const
{
  _currentState.printMove(os, m, withMark);
}

bool HexGame::isChanged() const
{
  return _isChanged;
}

void HexGame::setChanged(bool isChanged)
{
  _isChanged = isChanged;
}

bool HexGame::isBranched() const
{
  return _isBranched;
}

void HexGame::setBranched(bool isBranched)
{
  _isBranched = isBranched;
}

bool HexGame::canBack() const
{
  return _nextEntry > 0;
}

void HexGame::back()
{
  assert(canBack());
  _nextEntry--;
  const HexMove &m = _moveLog[_nextEntry].move();
  HexMark newNext = INVERT_HEX_MARK(next());
  if(m.isSwap()) {
    _currentState = HexGameState(board().transvert(), newNext, true);
  } else {
    HexBoard b(board());
    if(!m.isResign() && !m.isForfeit()) {
      b.set(m.field(), HEX_MARK_EMPTY);
    }
    _currentState = HexGameState(b, newNext, swappable());
  }
}

bool HexGame::canForward() const
{
  return _nextEntry < (signed)_moveLog.size();
}

void HexGame::forward()
{
  assert(canForward());
  _currentState.play(_moveLog[_nextEntry].move());
  _nextEntry++;
}

long HexGame::vertClockTotal() const
{
  assert(_nextEntry >= 0 && _nextEntry <= (signed)_moveLog.size());
  if(_nextEntry > 0)
    return _moveLog[_nextEntry - 1].vertClockTotal();
  else
    return 0;
}

long HexGame::horiClockTotal() const
{
  assert(_nextEntry >= 0 && _nextEntry <= (signed)_moveLog.size());
  if(_nextEntry > 0)
    return _moveLog[_nextEntry - 1].horiClockTotal();
  else
    return 0;
}

HexGame::Iterator HexGame::begin() const
{
  return _moveLog.begin();
}

HexGame::Iterator HexGame::end() const
{
  return _moveLog.begin() + _nextEntry;
}

HexGame::ReverseIterator HexGame::rbegin() const
{
  return _moveLog.rbegin() + (_moveLog.size() - _nextEntry);
}

HexGame::ReverseIterator HexGame::rend() const
{
  return _moveLog.rend();
}

void HexGame::save(ostream &os) const
{
  if(initialState().board().isEmpty()) {
    os << "BoardSize: " << initialState().board().xs() << "x"
       << initialState().board().ys() << endl;
  } else {
    os << "InitialBoard: " << endl << initialState().board() << endl;
  }
  os << "First: " << initialState().next() << endl;
  os << "Swap: " << initialState().swappable() << endl;
  os << "Moves:";
  for(unsigned int i = 0; i < _moveLog.size(); i++) {
    if((i % 2) == 0)
      os << endl << "  " << (i / 2 + 1) << ".";
    os << " ";
    printMove(os, _moveLog[i].move(), false);
    os << " (" << _moveLog[i].clock() << " ms)";
  }
  os << endl;
}

bool HexGame::parseMove(HexMove *m, const char *s) const
{
  return parseMove(board(), next(), m, s);
}

bool HexGame::parseMove(const HexBoard &b, HexMark mark, HexMove *m,
                               const char *s)
{
  assert(m);
  assert(s);
  string ls(s);
  for(unsigned i = 0; i < ls.size(); i++)
    ls[i] = tolower(ls[i]);

  if(ls == "swap") {
    *m = HexMove::createSwap(mark);
    return true;
  } else if(ls == "resign") {
    *m = HexMove::createResign(mark);
    return true;
  } else if(ls == "forfeit") {
    *m = HexMove::createForfeit(mark);
    return true;
  } else {
    int x = ls[0] - 'a';
    int y = atoi(s + 1) - 1;
    *m = HexMove(mark, b.coords2Field(x, y));
    return true;
  }
  return false;
}

void HexGame::load(istream &is)
{
  HexBoard b;
  HexMark first = HEX_MARK_EMPTY;
  bool swap;
  vector<pair<HexMove, long> > moves;

  {
    string boardHeader;
    is >> boardHeader;
    if(boardHeader == "BoardSize:") {
      char skip;
      int xs, ys;
      is >> xs;
      is >> skip;
      is >> ys;
      b = HexBoard(xs, ys);
    } else if(boardHeader == "InitialBoard:") {
      is >> b;
    } else {
      is.setstate(ios::failbit);
      return;
    }
  }

  {
    string firstHeader;
    is >> firstHeader;
    if(firstHeader != "First:") {
      is.setstate(ios::failbit);
      return;
    }
    is >> first;
  }

  {
    string swapHeader;
    is >> swapHeader;
    if(swapHeader != "Swap:") {
      is.setstate(ios::failbit);
      return;
    }
    is >> swap;
  }
  
  {
    string movesHeader;
    is >> movesHeader;
    if(movesHeader != "Moves:") {
      is.setstate(ios::failbit);
      return;
    }
  }

  is >> ws;

#define LINELEN 10000
  // all this scannig is ugly
  char line[LINELEN + 1];
  char move0[LINELEN + 1];
  char move1[LINELEN + 1];
  is.getline(line, LINELEN);
  int n;
  HexMark next = first;
  while(is) {
    long c0, c1;
    int sstat = sscanf(line, "  %d. %s (%ld ms) %s (%ld ms)", &n,
                       move0, &c0, move1, &c1);
    if(sstat != 3 && sstat != 5) {
      is.setstate(ios::failbit);
      return;
    }
    HexMove m0, m1;
    if(!parseMove(b, next, &m0, move0)) {
      is.setstate(ios::failbit);
      return;
    } else {
      moves.push_back(pair<HexMove, long>(m0, c0));
      next = INVERT_HEX_MARK(next);
    }
    if(sstat == 5) {
      if(!parseMove(b, next, &m1, move1)) {
        is.setstate(ios::failbit);
        return;
      } else {
        moves.push_back(pair<HexMove, long>(m1, c1));
        next = INVERT_HEX_MARK(next);
      }
    } else {
      break;
    }
    is.getline(line, LINELEN);
  }
  is.clear();

  // replay moves
  _initialState = HexGameState(b, first, swap);
  _currentState = _initialState;
  _moveLog.clear();
  for(unsigned i = 0; i < moves.size(); i++) {
    if(!isValidMove(moves[i].first)) {
      is.setstate(ios::failbit);
      return;
    } else {
      play(moves[i].first, moves[i].second);
    }
  }
  _isChanged = false;
  _isBranched = false;
}



/*
 *Summary of Hex Board 588
 *
 *It is morteux's turn.
 *
 *   Vert         Horz
 *   ski          morteux
 *   5 D8         6 D9
 *   7 C10        8 C9
 *   9 E8        10 E9
 *  11 G8
 *
 *
 *      A B C D E F G H I J K
 *    1  . . . . . . . . . . .  1
 *     2  . V . . . . . . . . .  2
 *      3  . . . . . . . . . . .  3
 *       4  . . . . . . . . . . .  4
 *        5  . . . . . . . . . . .  5
 *         6  . . . . . H . . . . .  6
 *          7  . . V . . . . . . . .  7
 *           8  . . . V V . V . . . .  8
 *            9  . . H H H . . . . . .  9
 *            10  . H V . . . . . . . .  10
 *             11  . . . . . . . . . . .  11
 *                  A B C D E F G H I J K
 *
 *
 *Subscribers: dispbot
 */
void HexGame::importPBEMGame(istream &is)
{
  // find the beginning of the move list
  bool foundVertHorz = false;
  string lastWord, thisWord;
  is >> lastWord;
  while(is && !foundVertHorz) {
    is >> thisWord;
    if(lastWord == "Vert" && thisWord == "Horz") {
      foundVertHorz = true;
    } else {
      lastWord = thisWord;
    }
  }
  if(!foundVertHorz) {
    is.setstate(ios::failbit);
    return;
  }

  // skip the player names
  string vertPlayer;
  string horiPlayer;
  is >> vertPlayer;
  is >> horiPlayer;

  // let's scan the move list
  vector<string> moves;
  while(is) {
    is >> ws;
    if(isdigit(is.peek())) {
      int n;
      string move;
      is >> n;
      is >> move;
      moves.push_back(move);
    } else {
      break;
    }
  }

  // read the board position
  HexBoard b;
  is >> b;

  // let's unplay the moves to get the initial board position
  for(int i = moves.size() - 1; i >= 0; i--) {
    HexMove m;
    if(!parseMove(b, HEX_MARK_VERT, &m, moves[i].c_str()) || m.isSwap() ||
       (m.isNormal() && !b.isNormalField(m.field()))) {
      is.setstate(ios::failbit);
      return;
    }
    assert(!m.isSwap());
    assert(m.isNull() || m.isResign() || m.isForfeit() ||
           b.isNormalField(m.field()));
    if(m.isNormal())
      b.set(m.field(), HEX_MARK_EMPTY);
  }

  // let's replay the moves from the initial board position
  HexGame g(b, HEX_MARK_VERT);
  HexMark next = HEX_MARK_VERT;
  for(int i = 0; i < (signed)moves.size(); i++) {
    HexMove m;
    if(!parseMove(b, next, &m, moves[i].c_str()) || m.isSwap() ||
       (!m.isNull() && !g.isValidMove(m))) {
      is.setstate(ios::failbit);
      return;
    }
    assert(m.isNull() || (!m.isSwap() && g.isValidMove(m)));
    if(!m.isNull()) {
      g.play(m, 0L);
      next = INVERT_HEX_MARK(next);
    }
  }

  if(is) {
    *this = g;
  }
  _isChanged = false;
  _isBranched = false;
}

HexMove HexGame::transvert(const HexMove &m) const
{
  HexMark mark = INVERT_HEX_MARK(m.mark());
  if(m.isNormal())
    return HexMove(mark, board().transvert(m.field()));
  else if(m.isSwap())
    return HexMove::createSwap(mark);
  else if(m.isResign())
    return HexMove::createResign(mark);
  else if(m.isForfeit())
    return HexMove::createForfeit(mark);
  else
    assert(0);
  // suppress warning
  return HexMove();
}

HexGame::MoveLogEntry 
HexGame::transvertMoveLogEntry(const MoveLogEntry &e) const
{
  MoveLogEntry r;
  r._vertClock = e._horiClock;
  r._horiClock = e._vertClock;
  r._vertClockTotal = e._horiClockTotal;
  r._horiClockTotal = e._vertClockTotal;
  r._move = transvert(e._move);
  return r;
}

HexGame HexGame::transvert() const
{
  HexGame r(_initialState.transvert());
  r._currentState = _currentState.transvert();
  for(int i = 0; i < (signed)_moveLog.size(); i++) {
    r._moveLog.push_back(transvertMoveLogEntry(_moveLog[i]));
  }
  r._nextEntry = _nextEntry;
  r._isChanged = _isChanged;
  r._isBranched = _isBranched;
  return r;
}

ostream &operator <<(ostream &os, const HexGame &g)
{
  os << "Initial Position:" << endl << g.initialState().board();
  os << "Initial Next: " << g.initialState().next() << endl;
  os << "Initial Swap: " << g.initialState().swappable() << endl;

  os << "Position:" << endl << g.board();
  os << "Next: " << g.next() << endl;
  os << "Swap: " << g.swappable() << endl;

  return os;
}
