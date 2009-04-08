// Yo Emacs, this -*- C++ -*-
#ifndef HEXMARK_H
#define HEXMARK_H

#include <iostream>

using std::istream;
using std::ostream;

enum HexMarkT { HEX_MARK_EMPTY, HEX_MARK_VERT, HEX_MARK_HORI };
typedef enum HexMarkT HexMark;

ostream &operator <<(ostream &os, const HexMark &m);
istream &operator >>(istream &is, HexMark &m);

#define INVERT_HEX_MARK(m) ((m == HEX_MARK_EMPTY) ? \
                            HEX_MARK_EMPTY : \
                            ((m == HEX_MARK_VERT) ? \
                             HEX_MARK_HORI : HEX_MARK_VERT))

#endif
