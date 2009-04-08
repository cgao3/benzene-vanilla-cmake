// Yo Emacs, this -*- C++ -*-
#ifndef HEXMATCH_H
#define HEXMATCH_H

#include "poi.h"
#include "hexgame.h"
#include "hexplayer.h"

#include <sys/time.h>

#include <qobject.h>
#include <qtimer.h>

/**
 * A match between two players (see @ref HexPlayer).
 * This class provides additional functionality on top of a @ref HexGame:
 * moving back and forth in history, keeping track of time, and acting
 * as a controller for the players.
 */
class HexMatch : public QObject
{
  Q_OBJECT
private:
  /**
   * HexMatch cannot be copied.
   */
  HexMatch(const HexMatch &m);

  /**
   * HexMatches cannot be assigned.
   */
  const HexMatch &operator =(const HexMatch &m);
public:
  enum StatusT {
    MATCH_ON,
    MATCH_OFF,
    MATCH_FINISHED
  };

  /**
   * A match has one of the following statuses:
   * <ul>
   * <li><code>MATCH_ON</code>: the match is not won yet, the clock is ticking.
   * <li><code>MATCH_OFF</code>: the match is not won yet,
   * but the clock is not ticking.
   * <li><code>MATCH_FINISHED</code>: the match is won,
   * the clock is not ticking
   * </ul>
   */
  typedef enum StatusT Status;

  /**
   * Constructs a match of <code>MATCH_OFF</code> status
   * based the game <code>g</code>, between vertical player
   * <code>vert</code> and horizontal player <code>hori</code>.
   */
  HexMatch(const HexGame &g, const Poi<HexPlayer> &vert = Poi<HexPlayer>(0),
           const Poi<HexPlayer> &hori = Poi<HexPlayer>(0));

  /**
   * Sets the vertical player.
   */
  void setVerticalPlayer(const Poi<HexPlayer> &vert);

  /**
   * Sets the horizontal player.
   */
  void setHorizontalPlayer(const Poi<HexPlayer> &hori);

  /**
   * @return the embedded game object.
   */
  const HexGame &game() const;

  /**
   * Returns the total time (in milliseconds) used by
   * the vertical player during this match.
   */
  long vertClockTotal() const;

  /**
   * Returns the total time (in milliseconds) used by
   * the horizontal player during this match.
   */
  long horiClockTotal() const;

  /**
   * Turns on the match.
   * Initializes unitialized players
   * (see @ref HexPlayer::init(const HexGame *g, HexMark yourMark))
   * for example at the first call or after @ref back() or @ref forward()
   * was called.
   * Then, it calls the player on turn to make its move
   * (see @ref HexPlayer::play()).
   * If the player really made a move it updates the clock.
   * If the game is won is sets the status to finished.
   *
   * Precondition: match is not finished.
   */
  bool doSome();

  /**
   * If the match is off, then it is turned on, and the clock is started.
   */
  void on();

  /**
   * If the match is on, then it is turned off, and the clock is stopped.
   */
  void off();

  /**
   * @return the status of this match
   */
  Status status() const;

  /**
   * Calls @ref HexGame::clearChanged() for the embedded game.
   */
  void setChanged(bool isChanged);

  /**
   * Calls @ref HexGame::clearBranched() for the embedded game.
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
   * Undoes all moves.
   */
  void backAll();

  /**
   * Redoes all moves.
   */
  void forwardAll();

  /**
   * Saves the match into a stream.
   */
  void save(ostream &os) const;
  
  /**
   * Prints the match.
   */
  friend ostream &operator <<(ostream &os, const HexMatch &b);
signals:
  /**
   * Emitted when a player in this match changes.
   */
  void signalPlayerChange();

  /**
   * Emitted when the status of this match changes.
   */
  void signalStatusChange();

  /**
   * Emitted when the clock changes: when a move is made,
   * navigating in move history, and every second during
   * thinking. The latter is only possible if the events are processed
   * during the turn of the player in question.
   */
  void signalClockChange();

  /**
   * Emitted when the current position changes.
   */
  void signalPositionChange();

  /**
   * Emitted when the change status of the game changes
   * (see @ref HexGame::isChanged() and @ref HexGame::isBranched())
   */
  void signalChangedGameStatus();
public slots:
  void timerDone();
private:
  HexGame _game;

  Poi<HexPlayer> _vert, _hori;
  bool _vertNew, _horiNew;

  void setStatus(HexMatch::Status status);
  Status _status;

  void resetClock();
  void clockOn();
  void clockOff();
  QTimer _timer;
  struct timeval _startTime;
  clock_t _startClock;
  long _millisecondsMove;
};

#endif
