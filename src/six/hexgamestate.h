// Yo Emacs, this -*- C++ -*-
#ifndef HEXGAMESTATE_H
#define HEXGAMESTATE_H

#include "hexmove.h"
#include "hexboard.h"

#include <iostream>
#include <vector>

/**
 * State of a hex game.
 * Encapsulates everything a player needs to know to play a move:
 * board position, who's next, is swap allowed.
 *
 * In short this class contains the rules of Hex.
 */
class HexGameState
{
public:
  /**
   * Constructs a game state of board position <code>b</code>,
   * with the player of mark <code>next</code> to play next,
   * with the swap option allowed according to <code>swappable</code>
   * and the <code>winner</code> of this game set.
   */
  HexGameState(const HexBoard &b = HexBoard(), HexMark next = HEX_MARK_VERT,
               bool swappable = true, HexMark winner = HEX_MARK_EMPTY);

  /**
   * @return the board position
   */
  const HexBoard &board() const;

  /**
   * @return the mark of the player to play next
   */
  HexMark next() const;

  /**
   * @return true iff this game was created as swappable and no swap was
   * played, yet
   */
  bool swappable() const;

  /**
   * Tests if <code>move</code> is valid in this game state.
   */
  bool isValidMove(const HexMove &move) const;

  /**
   * Returns <code>HEX_MARK_EMPTY</code> if this game is not won yet;
   * or the mark of winner if a winning connection is completed or
   * the opponent has resigned.
   */
  HexMark winner() const;

  /**
   * Plays the valid <code>move</code> on the board
   * updates @ref next() and @ref swappable() if necessary.
   */
  void play(const HexMove &move);

  /**
   * Prints a move with proper coordinates instead of field values.
   *
   * @param withMark controls whether the mark of the player is printed
   */
  void printMove(ostream &os, const HexMove &m, bool withMark = true) const;

  /**
   * Like HexBoard::transvert() but invert _next and _winner as well.
   */
  HexGameState transvert() const;

  /**
   * Prints this game.
   */
  friend ostream &operator <<(ostream &os, const HexGameState &b);
protected:
  HexBoard _board;
  HexMark _next;
  bool _swappable;
  HexMark _winner;
};

#endif
