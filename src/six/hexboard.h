// Yo Emacs, this -*- C++ -*-
#ifndef HEXBOARD_H
#define HEXBOARD_H

#include "hexfield.h"
#include "hexmark.h"

#include <vector>
#include <iostream>

using std::vector;
using std::pair;
using std::istream;
using std::ostream;

/**
 * A HexBoard is a parallelogram (not necessarily rhombic).  It
 * consists of fields arranged like this in case of a board with
 * dimensions 5 and 7:
 *
 * <pre>
 *  A B C D E
 * 1 . . . . . 1
 *  2 . . . . . 2
 *   3 . . . . . 3
 *    4 . . . . . 4
 *     5 . . . . . 5
 *      6 . . . . . 6
 *       7 . . . . . 7
 *          A B C D E
 * </pre>
 *
 * The edges surrounding the normal fields are also considered fields.
 *
 * Coordinates on the X axis are denoted by letters, coordinates on
 * the Y axis are denoted by numbers.
 *
 * The coordinate system has its (0, 0) point at A1, while for
 * instance E1 is (4, 0) and A7 is (0, 6).
 */
class HexBoard
{
public:
  /**
   * HexField constants for edges and invalid field.
   */
  enum {
    TOP_EDGE = 0, BOTTOM_EDGE = 1, LEFT_EDGE = 2, RIGHT_EDGE = 3,
    FIRST_NORMAL_FIELD = 4
  };

  /**
   * Iterator for neighbours of a field on the board. It is very
   * similar to STL iterators.
   */
  class Iterator
  {
  friend class HexBoard;
  private:
    Iterator();
    Iterator(const HexBoard *b, HexField center);
  public:
    HexField operator *() const;
    Iterator &operator ++();
    bool operator ==(const Iterator &it) const;
    bool operator !=(const Iterator &it) const;
  private:
    bool _isEnd;
    HexField _center;
    int _x, _y;
    const HexBoard *_board;
    int _current;
  };

  /**
   * Constructs a hex board of the given dimensions.
   */
  HexBoard(int xs = 11, int ys = 11);

  /**
   * Copy constructor.
   */
  HexBoard(const HexBoard &b);

  /**
   * Destroys this board.
   */
  ~HexBoard();

  /**
   * Assigns <code>b</code> to this board. Board position and
   * dimensions are copied.
   */
  HexBoard &operator =(const HexBoard &b);

  /**
   * @return the size of the board along the X axis (width; the
   * distance for the horizontal player to cover).
   */
  int xs() const;

  /**
   * @return the size of the board along the Y axis (height; the
   * distance for the vertical player to cover).
   */
  int ys() const;

  /**
   * Returns the number of fields on this board. The range <code>[0,
   * size())</code> is valid for fields.
   */
  int size() const;

  /**
   * Returns the mark on field <code>f</code>.
   */
  HexMark get(HexField f) const;

  /**
   * Sets the mark on field <code>f</code>.
   */
  void set(HexField f, HexMark mark);

  /**
   * Returns a board that has x, y dimensions and coordinates swapped
   * and marks substituted for their opposite.
   */
  HexBoard transvert() const;

  /**
   * Returns the matching field for the transverted board.
   */
  HexField transvert(const HexField &) const;

  /**
   * @return true iff <code>f</code> is a valid non-edge field on this
   * board.
   */
  bool isNormalField(HexField f) const;

  /**
   * @return true iff all normal fields on this board are empty.
   */
  bool isEmpty() const;

  /**
   * @return the number of non-empty fields on this board.
   */
  int nMark() const;

  /**
   * Finds the winner in the current position.
   *
   * @return HEX_MARK_VERT if the vertical player has connected the
   *         top and bottom edges;
   *         HEX_MARK_HORI if the horizontal player has connected the
   *         left and right edges;
   *         HEX_MARK_EMPTY if the position on this board is not a winning one.
   */
  HexMark winner() const;

  /**
   * Finds the winner in the current position (see @ref winner()), but
   * also returns normal fields that make up a winning connection.
   */
  pair<HexMark, vector<HexField> > winningPath() const;

  /**
   * @return the field that has the coordinates <code>(x, y)</code>
   */
  HexField coords2Field(int x, int y) const;

  /**
   * Converts a field to a coordinate pair.
   *
   * @param x if null it is not set.
   * @param y if null it is not set.
   */
  void field2Coords(HexField f, int *x, int *y) const;

  /**
   * Returns true iff f1 and f2 are adjacent on this board.
   */
  bool adjacentFields(HexField f1, HexField f2) const;

  /**
   * Returns an iterator that iterates on the fields neighbouring
   * <code>f</code>.
   */
  Iterator neighbourBegin(HexField f) const;

  /**
   * Returns an iterator that marks the end of the sequence (STL style).
   */
  const Iterator &neighbourEnd() const;

  /**
   * Prints coordinates of a field.
   */
  void printField(ostream &os, const HexField &f) const;

  /**
   * Tests if this board is equal to board <code>b</code>.
   *
   * @returns true iff the board dimensions and positions are equal
   */
  bool operator ==(const HexBoard &b) const;

  /**
   * Prints this board.
   */
  friend ostream &operator <<(ostream &os, const HexBoard &b);

  /**
   * Reads this board.
   */
  friend istream &operator >>(istream &is, HexBoard &b);
private:
  int nNeighbours(int s[], HexField f) const;
  bool isConnected(int s[], HexField f, HexField to) const;
  bool isWinningPath(int s[]) const;
  HexMark winner(int *expanded) const;
  bool expand(int *s, HexField f, HexField goal) const;
  int _xs, _ys;
  int _size;
  HexMark *_v;
  static Iterator _neighbourEnd;
};

#endif
