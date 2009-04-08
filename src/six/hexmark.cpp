#include "hexmark.h"
#include <cassert>

using std::ios;

ostream &operator <<(ostream &os, const HexMark &m)
{
  switch(m) {
  case HEX_MARK_EMPTY:
    os << ".";
    break;
  case HEX_MARK_VERT:
    os << "V";
    break;
  case HEX_MARK_HORI:
    os << "H";
    break;
  default: assert(0);
  }
  return os;
}

istream &operator >>(istream &is, HexMark &m)
{
  char c;
  is >> c;
  switch(c) {
  case '.':
    m = HEX_MARK_EMPTY;
    break;
  case 'V':
    m = HEX_MARK_VERT;
    break;
  case 'H':
    m = HEX_MARK_HORI;
    break;
  default: is.setstate(ios::failbit);
  }
  return is;
}
