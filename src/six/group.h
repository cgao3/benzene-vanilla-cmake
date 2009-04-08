// Yo Emacs, this -*- C++ -*-
#ifndef GROUP_H
#define GROUP_H

#include "hexboard.h"
#include "carrier.h"
#include "poi.h"
#include "vec.h"

#include <utility>
#include <set>
#include <deque>
#include <vector>

using std::deque;
using std::set;

class Group;

/**
 * A home of @ref Group objects that together make up a segmentation of
 * the fields of a board position.
 */
class Grouping
{
  friend class GroupTest;
public:
  typedef int GroupIndex;
  /**
   * RMap provides the reverse mapping: HexField to group indeces.
   */
  class RMap
  {
  public:
    RMap(int boardSize = 0);
    Poi<Group> &operator [](HexField f);
    const Poi<Group> &operator [](HexField f) const;
  private:
    vector<Poi<Group> > _v;
  };
public:
  /**
   * Creates an empty uninitialized grouping.
   */
  Grouping();

  /**
   * Segments the fields of <code>board</code> into groups.
   *
   * @param mark the mark of interest, fields with the opposite mark are
   * ignored; possible values are HEX_MARK_VERT and HEX_MARK_HORI.
   */
  Grouping(const HexBoard &board, HexMark mark);
    
  /**
   * Reinitializes the grouping.
   */
  void init(const HexBoard &, HexMark mark);

  /**
   * The return value of move(). Describes the changes in the
   * grouping.
   */
  struct Change
  {
    /**
     * The group created by the move. Null iff the move's mark is not
     * the mark of the grouping or the new group was killed at once.
     */
    Poi<Group> newGroup;
    /**
     * The group of the empty cell on which the move was played. Null
     * iff the group did not exist (probably it was killed
     * before). This group is now no longer valid.
     */
    Poi<Group> emptyGroup;
    /**
     * The groups that were united by the move. These groups always
     * have a non-empty mark and are invalid in the new grouping. If
     * newGroup is null then no groups are here, but in deletedGroups.
     */
    vector<Poi<Group> > unitedGroups;
    /**
     * The groups that have no descendants. The groups that were
     * killed by the dead group detection algorithm. These groups are
     * no longer valid.
     */
    vector<Poi<Group> > deletedGroups;
  };

  /**
   * Sets field <code>f</code> to the non-empty <code>mark</code>, and
   * updates the grouping. Unchanged groups are left alone, they keep
   * their addresses (thus the wrapping in Poi), but not their indeces
   * in the grouping.
   */
  Change move(HexField f, HexMark mark);

  /**
   * @return the number of groups in this grouping
   */
  int size() const;

  /**
   * @param i is the index of the group; it must be in the range [0, size())
   *
   * @return the group of index <code>i</code>
   */
  Poi<Group> operator [](GroupIndex i) const;

  /**
   * @return the index of the group field <code>f</code> belongs to;
   * or <code>-1</code> if it does not belong to any group.
   */
  Poi<Group> operator ()(HexField f) const;

  /**
   * @return the index of group <code>g</code> in this grouping;
   */
  GroupIndex groupIndex(const Group *g) const;

  /**
   * @return the index of group <code>g</code> in this grouping;
   */
  GroupIndex groupIndex(const Poi<Group> &g) const;

  /**
   * @return the board that is segmented
   */
  const HexBoard &board() const;

  /**
   * @return the mark of interest
   */
  HexMark mark() const;

  /**
   * Return the neighbours of group g.
   */
  set<Poi<Group> > neighbouringGroups(const Poi<Group> &g) const; 

  /**
   * Return a set of HexFields where it makes no sense to play for
   * <code>mark()</code>.
   */
  const Carrier &uselessFields() const;

  /**
   * Return the set of empty fields in this grouping.
   */
  const Carrier &emptyFields() const;

  friend ostream &operator <<(ostream &os, const Grouping &g);
private:
  void addGroup(const Poi<Group> &g);
  void removeGroup(const Poi<Group> &g);
  set<Poi<Group> > removeIfNotEmptyGroup(const set<Poi<Group> > &);
  bool hasAdjacentEmptyFields(const set<Poi<Group> > &emptyGroups);
  bool isDead(const Poi<Group> &g,
              set<Poi<Group> > &directNeighbours,
              set<Poi<Group> > &neighboursOfWouldBeGroup);
  void killDeadGroups(deque<Poi<Group> > targets,
                      vector<Poi<Group> > *victims);
  HexBoard _board;
  HexMark _mark;
  vector<Poi<Group> > _groups;
  Carrier _uselessFields;
  Carrier _emptyFields;
  RMap _rmap;
};

/**
 * A group is a set of adjacent fields with the same mark.
 * If the mark is empty then the group has exactly one field in it,
 * if the mark is non-empty then the group is maximal in the sense
 * that all fields adjacent to it are of different mark.
 *
 * Groups are immutable objects and always constructed by the
 * @ref Grouping they belong to.
 *
 * Groups always contain at least one field.
 */
class Group
{
  friend class Grouping;
  friend class GroupTest;
public:
  /**
   * The mark of the group.
   */
  HexMark mark() const
  {
    return _mark;
  };

  /**
   * @return true iff this group includes an edge
   */
  bool edge() const
  {
    return _edge;
  };

  /**
   * @return the sum of the area of fields in this group;
   * every normal field has an area of 1,
   * edge fields have an area equal to the width or height of the board
   * dependig on their orientation.
   */
  int area() const
  {
    return _area;
  };

  /**
   * @return the fields in this group
   */
  const vector<HexField> &fields() const
  {
    return _fields;
  };

  /**
   * @return the fields in this group as a carrier
   */
  const Carrier &coverage() const
  {
    return _coverage;
  };

  /**
   * Prints the group.
   */
  friend ostream &operator <<(ostream &os, const Group &g);
private:
  Group(const HexBoard &b, HexField f);
  void add(const HexBoard &b, HexField f);
  void expand(const HexBoard &b, HexField f);
  HexMark _mark;
  vector<HexField> _fields;
  Carrier _coverage;
  bool _edge;
  int _area;
};

#endif
