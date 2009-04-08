// Yo Emacs, this -*- C++ -*-
#ifndef HEXPLAYER_H
#define HEXPLAYER_H

#include "hexgame.h"

/**
 * Interface for players.
 */
class HexPlayer
{
public:
  /**
   * Virtual desctructor.
   */
  virtual ~HexPlayer() {};

  /**
   * Tells the player that it is to play game <code>g</code>
   * with mark <code>yourMark</code>.
   */
  virtual void init(const HexGame *g, HexMark yourMark) = 0;

  /**
   * Tells the player that move <code>m</code> was made.
   */
  virtual void played(const HexMove &m) = 0;

  /**
   * Makes the player think about its next move.
   *
   * @return true as member <code>first</code> in the return value
   * iff thinking is done and a move was selected.
   */
  virtual pair<bool, HexMove> play() = 0;
};

#endif
