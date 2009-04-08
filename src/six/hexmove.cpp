#include "hexmove.h"

#include <cassert>

HexMove::HexMove(HexMark m, bool swap, bool resign, bool forfeit)
  : _null(false), _swap(swap), _resign(resign), _forfeit(forfeit), _mark(m)
{
  assert((_swap ? 1 : 0) + (_resign ? 1 : 0) + (_forfeit ? 1 : 0) == 1);
}

HexMove::HexMove()
  : _null(true), _swap(false), _resign(false), _forfeit(false),
    _mark(HEX_MARK_EMPTY)
{
}

HexMove::HexMove(HexMark m, HexField f)
  : _null(false), _swap(false), _resign(false), _forfeit(false),
    _mark(m), _field(f)
{
  assert(_mark != HEX_MARK_EMPTY);
}

HexMove HexMove::createSwap(HexMark m)
{
  return HexMove(m, true, false, false);
}

HexMove HexMove::createResign(HexMark m)
{
  return HexMove(m, false, true, false);
}

HexMove HexMove::createForfeit(HexMark m)
{
  return HexMove(m, false, false, true);
}

bool HexMove::operator ==(const HexMove &m) const
{
  return ((_null && m._null) ||
          (_mark == m._mark &&
           (_resign && m._resign) ||
           (_swap && m._swap) ||
           (_forfeit && m._forfeit) ||
           (isNormal() && m.isNormal() && _field == m._field)));
}

bool HexMove::operator !=(const HexMove &m) const
{
  return !((*this) == m);
}

bool HexMove::isNull() const
{
  return _null;
}

bool HexMove::isSwap() const
{
  return _swap;
}

bool HexMove::isResign() const
{
  return _resign;
}

bool HexMove::isForfeit() const
{
  return _forfeit;
}

bool HexMove::isNormal() const
{
  return !_null && !_swap && !_resign && !_forfeit;
}

HexMark HexMove::mark() const
{
  assert(!_null);
  return _mark;
}

HexField HexMove::field() const
{
  assert(isNormal());
  return _field;
}
