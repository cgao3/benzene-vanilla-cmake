#include "hexgamestate.h"

#include <cassert>

using std::endl;

HexGameState::HexGameState(const HexBoard &b, HexMark next, bool swappable,
                           HexMark winner)
  : _board(b), _next(next), _swappable(swappable), _winner(winner)
{
  assert(_next != HEX_MARK_EMPTY);
  if(_winner == HEX_MARK_EMPTY)
    _winner = _board.winner();
}

const HexBoard &HexGameState::board() const
{
  return _board;
}

HexMark HexGameState::next() const
{
  return _next;
}

bool HexGameState::swappable() const
{
  return _swappable;
}

bool HexGameState::isValidMove(const HexMove &move) const
{
  return (_winner == HEX_MARK_EMPTY && !move.isNull() &&
          move.mark() == _next &&
          ((move.isSwap() && _swappable && _board.nMark() == 1) ||
           move.isResign() || move.isForfeit() ||
           (move.isNormal() &&
            _board.isNormalField(move.field()) &&
            _board.get(move.field()) == HEX_MARK_EMPTY)));
}

HexMark HexGameState::winner() const
{
  return _winner;
}

void HexGameState::play(const HexMove &move)
{
  assert(isValidMove(move));
  if(move.isSwap()) {
    //_board = _board.transvert();
    _swappable = false;
  } else if(move.isNormal()) {
    _board.set(move.field(), move.mark());
    _winner = _board.winner();
    _next = INVERT_HEX_MARK(_next);
  } else if(move.isResign() || move.isForfeit()) {
    _winner = INVERT_HEX_MARK(move.mark());
    _next = INVERT_HEX_MARK(_next);
  }
}

void HexGameState::printMove(ostream &os, const HexMove &m,
                             bool withMark) const
{
  assert(!m.isNull());
  if(m.isSwap()) {
    os << "Swap";
  } else if(m.isResign()) {
    os << "Resign";
  } else if(m.isForfeit()) {
    os << "Forfeit";
  } else {
    if(withMark)
      os << m.mark();
    _board.printField(os, m.field());
  }
}

HexGameState HexGameState::transvert() const
{
  return HexGameState(_board.transvert(), INVERT_HEX_MARK(_next),
                      _swappable, INVERT_HEX_MARK(_winner));
}

ostream &operator <<(ostream &os, const HexGameState &g)
{
  os << "Position:" << endl << g._board;
  os << "Next: " << g._next << endl;
  os << "Swap: " << g._swappable << endl;
  return os;
}
