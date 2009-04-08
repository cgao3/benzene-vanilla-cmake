// Yo Emacs, this -*- C++ -*-
#ifndef SIXPLAYER_H
#define SIXPLAYER_H

#include "hexplayer.h"
#include "connector.h"
#include "circuit.h"

/**
 * A player that uses @ref Connector and @ref Circuit and performs a
 * shallow game tree search.
 *
 * SixPlayer is capable of playing at four different levels: beginner,
 * intermediate, advanced and expert levels.
 */
class SixPlayer : public HexPlayer
{
public:
  enum LevelT { BEGINNER, INTERMEDIATE, ADVANCED, EXPERT };
  typedef enum LevelT Level;

  /**
   * Constructs a new player of <code>level</code>. During call to
   * @ref play() <code>task</code> will be periodically called. See
   * @ref Connector::setTask(SlicedTask *task).
   */
  SixPlayer(Level level = BEGINNER, bool allowResign = false,
            SlicedTask *task = 0);

  void init(const HexGame *g, HexMark yourMark);
  void played(const HexMove &m);
  pair<bool, HexMove> play();

  /**
   * When called during a @ref play() it returns the best move found
   * so far. If this player is not thinking at the moment of call or
   * no move was found so far it returns a null (invalid) move.
   */
  HexMove candidateMove() const;

  /**
   * If this player is thinking it is cancelled and @ref play()
   * returns at the first possible moment.
   */
  void cancelMove();

  /**
   * If set SixPlayer resigns when encounterig a hopeless position.
   */
  void allowResign(bool allow);

  /** Sets the skill level. */
  void setSkillLevel(Level level);

  /** Returns current skill level. */
  Level skillLevel() const;

private:
  /**
   * A move with a value and connectors for calculating updated
   * connections.
   */
  class Move
  {
  public:
    Move() {}
    explicit Move(HexMove m, double v = 0.0) : move(m), value(v) {}
    HexMove move;
    double value;
    Poi<Connector> vert;
    Poi<Connector> hori;
    bool operator <(const Move &m) const
    {
      return (move.isSwap() || (value > m.value));
    }
  };

  /**
   * A proxy SlicedTask that tracks invocations.
   */
  class TaskTracker : public SlicedTask
  {
  public:
    /**
     * Every @ref doSlice() call will invoke
     * <code>t->doSlice()</code>. While the call is active
     * <code>activeConnector</code> is set to <code>c</code>.
     */
    TaskTracker(SlicedTask *t = 0, Connector *c = 0,
                Connector **activeConnector = 0);
    void doSlice();
  private:
    SlicedTask *_task;
    Connector *_connector;
    Connector **_activeConnector;
  };

  /**
   * Appends all non-swap moves of <code>mark</code> possible in this
   * board position (@ref Connector::board()) and estimates the @ref
   * Move::value.
   *
   * If the opponent threatens to win (see @ref
   * Connector::semiWinner()), then only moves in the opponent's
   * critical path are considered.
   *
   * A move's estimated value is the sum of the energy levels of the
   * group according to the conductances for HEX_MARK_VERT and
   * HEX_MARK_HORI. See @ref Circuit::energy(GroupIndex).
   */
  void generateMoves(const HexGame &game,
                     unsigned depth,
                     const Connector &vert, const Circuit &vertCond,
                     const Connector &hori, const Circuit &horiCond,
                     HexMark mark, vector<Move> *moves);

  /**
   * Evaluates the position for the player of <code>mark</code>.
   * Position must be not be won (see @ref HexBoard::winner()) but
   * there must be a potential winner.
   *
   * @param depth influences the value
   * @param bestMove is set to the highest energy move in the carrier of
   * winning virtual connection.
   */
  double evalPotentialWinner(const Connector &vert,
                             const Connector &hori,
                             HexMark mark,
                             HexMove *bestMove, unsigned depth);

  /**
   * Evaluates the position for player with <code>mark</code>. For
   * HEX_MARK_VERT, the value is log(Rv/Rh) where Rv is the resistence
   * for HEX_MARK_VERT and Rh is the resistence for HEX_MARK_HORI.
   */
  double evalPos(const Circuit &vertCond,
                 const Circuit &oldVertCond,
                 const Circuit &horiCond,
                 const Circuit &oldHoriCond,
                 HexMark mark);

  /**
   * Detect won games.
   * 
   * @return true iff this position should not be anylized further
   */
  bool tryToCut(const Connector &vert,
                const Connector &hori,
                HexMark mark,
                HexMove *bestMove, double alpha,
                unsigned depth,
                double *value);

  /**
   * Creates connectors from <code>oldVert</code> and <code>oldHori</code>
   * for move <code>m</code> and stores them in the move.
   */
  void move(const Connector &oldVert, const Connector &oldHori, Move *m);

  /**
   * Performs alpha-(beta) game tree search, evaluates the position,
   * suggests a move.
   *
   * @param widths controls the number of candidate moves evaluated
   * at each depth;
   * <code>widths.size()</code> is the depth of the game tree search
   * @param depth is the depth of this node in the game tree;
   * starts from zero.
   */
  double eval(const HexGame &game,
              const Connector &vert, const Circuit &oldVertCond,
              const Connector &hori, const Circuit &oldHoriCond,
              HexMove *bestMove, double alpha, double beta,
              const vector<unsigned> &widths, unsigned depth);

  /**
   * Creates new connectors if their parameters do not match these
   * parameters.
   */
  void updateConnectors(const Poi<Connector::DualBatchLimiter> &limiter,
                        unsigned mio, bool ue, bool ip);

  long usedTime();
  long remainingTime();

  pair<bool, HexMove> beginnerPlay();
  pair<bool, HexMove> intermediatePlay();
  pair<bool, HexMove> advancedPlay();
  pair<bool, HexMove> expertPlay();
  pair<bool, HexMove> commonPlay(const Poi<Connector::DualBatchLimiter> &
                                 limiter,
                                 unsigned mio, bool ue, bool ip,
                                 const vector<unsigned> &widths);

  TaskTracker _vertTracker;
  TaskTracker _horiTracker;
  Poi<Connector> _vert;
  Poi<Connector> _hori;
  HexMove _candidateMove;
  bool _resignAllowed;

  Level _level;
  const HexGame *_game;
  HexMark _myMark;
  int _nNode;
  int _nCond;
  int _nMove;

  SlicedTask *_task;
  Connector *_activeConnector;
  bool _cancelMove;
  bool _thinking;
  Poi<Circuit::DualBatchConductance> _conductance;
};

inline SixPlayer::Level SixPlayer::skillLevel() const
{
    return _level;
}

inline void SixPlayer::setSkillLevel(SixPlayer::Level level)
{
    _level = level;
}

#endif
