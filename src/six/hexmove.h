// Yo Emacs, this -*- C++ -*-
#ifndef HEXMOVE_H
#define HEXMOVE_H

#include "hexmark.h"
#include "hexfield.h"

/**
 * A move by a player in a game.
 */
class HexMove
{
public:
  /**
   * Constructs an invalid (null) move.
   */
  HexMove();

  /**
   * Constructs a resign move.
   */
  static HexMove createSwap(HexMark m);

  /**
   * Constructs a resign move.
   */
  static HexMove createResign(HexMark m);

  /**
   * Constructs a forfeit move.
   */
  static HexMove createForfeit(HexMark m);

  /**
   * Constructs a move on field <code>f</code> with mark <code>m</code>
   */
  HexMove(HexMark m, HexField f);

  /**
   * Tests if <code>m</code> and this move are the same move.
   */
  bool operator ==(const HexMove &m) const;

  /**
   * Inverse of ==
   */
  bool operator !=(const HexMove &m) const;

  /**
   * @return true iff this move is a null move
   */
  bool isNull() const;

  /**
   * @return true iff this move is a swap
   */
  bool isSwap() const;

  /**
   * @return true iff this move is a resignation
   */
  bool isResign() const;

  /**
   * @return true iff this move is a forfeit
   */
  bool isForfeit() const;

  /**
   * @return true iff this move is not null, swap or resignation
   */
  bool isNormal() const;

  /**
   * For non-swap moves it returns the mark of the player who makes this move.
   */
  HexMark mark() const;

  /**
   * For non-swap moves it return the field on which the move is made.
   */
  HexField field() const;
private:
  HexMove(HexMark m, bool swap, bool resign, bool forfeit);
  bool _null;
  bool _swap;
  bool _resign;
  bool _forfeit;
  HexMark _mark;
  HexField _field;
};

#endif
