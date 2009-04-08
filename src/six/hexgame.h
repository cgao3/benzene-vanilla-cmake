// Yo Emacs, this -*- C++ -*-
#ifndef HEXGAME_H
#define HEXGAME_H

#include "hexgamestate.h"

#include <iostream>
#include <vector>

/**
 * HexGame keeps track of game states (see @ref HexGameState),
 * move and clock history and allows navigation in this history.
 */
class HexGame
{
public:
  /**
   * Encapsulates information pertaining to a move: the move itself
   * and clock times.
   */
  class MoveLogEntry
  {
    friend class HexGame;
  public:
    /**
     * Default constructor to allow compilation (vector's resize needs it).
     */
    MoveLogEntry();

    /**
     * Constructs a log entry for move <code>m</code> that took
     * <code>clock</code> milliseconds and the previous move was
     * <code>prevMove</code>.
     */
    MoveLogEntry(const HexMove &m, long clock,
                 const MoveLogEntry *prevMove = 0);
    /**
     * @returns the time for the side that made the move
     */
    long clock() const;

    /**
     * @returns the time the vertical player spent (0 if it's not his turn)
     */
    long vertClock() const;

    /**
     * @returns the time the horizontal player spent (0 if it's not his turn)
     */
    long horiClock() const;

    /**
     * @returns the total time spent by the vertical player until this move
     */
    long vertClockTotal() const;

    /**
     * @returns the total time spent by the horizontal player until this move
     */
    long horiClockTotal() const;

    /**
     * The move the log entry is about.
     */
    const HexMove &move() const;
  private:
    long _vertClock;
    long _horiClock;
    long _vertClockTotal;
    long _horiClockTotal;
    HexMove _move;
  };

  /**
   * STL style iterator type.
   */
  typedef vector<MoveLogEntry>::const_iterator Iterator;

  /**
   * STL style reverse iterator type.
   */
  typedef vector<MoveLogEntry>::const_reverse_iterator ReverseIterator;

  /**
   * Constructs a game that starts from <code>initialState</code>.
   */
  HexGame(const HexGameState &initialState);

  /**
   * Constructs a game that starts from the position on board <code>b</code>,
   * with the player of mark <code>first</code> to play first,
   * and with the swap option allowed according to <code>swappable</code>.
   */
  HexGame(const HexBoard &b = HexBoard(), HexMark next = HEX_MARK_VERT,
          bool swappable = true);

  /**
   * @return the game state at the start of the game
   */
  const HexGameState &initialState() const;

  /**
   * @return the current game state
   */
  const HexGameState &currentState() const;

  /**
   * Shortcut for currentState().board()
   */
  const HexBoard &board() const;

  /**
   * Shortcut for currentState().next()
   */
  HexMark next() const;

  /**
   * Shortcut for currentState().swappable()
   */
  bool swappable() const;

  /**
   * Shortcut for currentState().isValid()
   */
  bool isValidMove(const HexMove &move) const;

  /**
   * Shortcut for currentState().winner()
   */
  HexMark winner() const;

  /**
   * Just like @ref HexGameState::play(const HexMove &),
   * but updates the move trace.
   */
  void play(const HexMove &move, long milliseconds);

  /**
   * Shortcut for currentState().printMove()
   */
  void printMove(ostream &os, const HexMove &m, bool withMark = true) const;

  /**
   * Returns true iff one or more moves were made since the creation
   * of this game or since when it was loaded.
   *
   * Note that after a successful save, @ref clearChanged() should be called.
   */
  bool isChanged() const;

  /**
   * Sets the changed flag. Useful after saving/loading.
   */
  void setChanged(bool isChanged);

  /**
   * Returns true iff play branched from a position in game history since the
   * creation of this game or since when it was loaded.
   *
   * Note that after a successful save, @ref clearBranched() should be called.
   */
  bool isBranched() const;

  /**
   * Clears the branched flag. Useful after saving/loading.
   */
  void setBranched(bool isBranched);

  /**
   * @return true iff it is possible to move backward in history:
   * there is a move to undo.
   */
  bool canBack() const;

  /**
   * @return true iff it is possible to move forward in history:
   * there is a move to redo.
   */
  bool canForward() const;

  /**
   * Undoes a the last move.
   *
   * Precondition: @ref canBack() is true.
   */
  void back();

  /**
   * Redoes the last move that has been undone by @ref back()
   * if no move was made since then.
   *
   * Precondition: @ref canForward() is true.
   */
  void forward();

  /**
   * Returns the total time (in milliseconds) used by
   * the vertical player during this game.
   */
  long vertClockTotal() const;

  /**
   * Returns the total time (in milliseconds) used by
   * the horizontal player during this game.
   */
  long horiClockTotal() const;

  /**
   * Iterator pointing to the start of a MoveLogEntry sequence.
   */
  Iterator begin() const;

  /**
   * Iterator pointing to the end of a MoveLogEntry sequence.
   */
  Iterator end() const;

  /**
   * Reverse iterator pointing to the start of a reversed MoveLogEntry
   * sequence.
   */
  ReverseIterator rbegin() const;

  /**
   * Reverse iterator pointing to the end of a reversed MoveLogEntry sequence.
   */
  ReverseIterator rend() const;

  /**
   * Saves this game to a stream.
   * Test failbit on the stream for status information.
   */
  void save(ostream &os) const;

  /**
   * Loads a game from a stream.
   * Test failbit on the stream for status information.
   */
  void load(istream &is);

  /**
   * Imports a PBEM game from Richard's PBEM server.
   */
  void importPBEMGame(istream &is);

  /**
   * Returns a new game where the colors and the board are swapped.
   */
  HexGame transvert() const;

  /**
   * Returns the matching move in the transverted game.
   */
  HexMove transvert(const HexMove &) const;

  /**
   * Prints this game.
   */
  friend ostream &operator <<(ostream &os, const HexGame &b);

  /**
   * Parse string s into move m. Return true iff successful.
   */
  bool parseMove(HexMove *m, const char *s) const;

private:
  static bool parseMove(const HexBoard &b, HexMark mark, HexMove *m,
                        const char *s);
  MoveLogEntry transvertMoveLogEntry(const MoveLogEntry &) const;
  HexGameState _initialState;
  HexGameState _currentState;
  vector<MoveLogEntry> _moveLog;
  int _nextEntry;
  bool _isChanged;
  bool _isBranched;
};

#endif
