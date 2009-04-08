// Yo Emacs, this -*- C++ -*-
#ifndef CONNECTOR_H
#define CONNECTOR_H

#include "config.h"

#include "hexboard.h"
#include "hexmove.h"
#include "batch.h"
#include "slicedtask.h"

#include <deque>
#include <vector>
#include <list>
#include <set>
#include <string>
#include <fstream>
#include <sstream>

using std::deque;
using std::vector;
using std::list;
using std::set;

#include <map>

/**
 * Connector calculates and updates move by move virtual [semi]
 * connections between groups on the board for either of the two
 * players.
 *
 * The algorithm is based on the idea published by Vadim
 * V. Anshelevich, see @ref http://earthlink.net/~vanshel .
 *
 * Connections and semi-connections are stored in separate data
 * structures for every different (unordered) pair of end groups (see
 * @ref Batch and DualBatch). Carriers in these batches are ordered by
 * size (smallest first). There are some parameters that control the
 * behaviour of these storages:
 *
 * <ul>
 *
 * <li><strong><code>hardMax</code></strong> is an absolute limit for
 * the size of the storage; if the size would exceed hardMax by the
 * addition of a subgame, it is simply discarded.
 *
 * <li><strong><code>softMax</code></strong> is a soft limit for the
 * size of the storage; storage size can exceed softMax but only the
 * first softMax subgames are enqueued for processing and the rest
 * remains waiting until it gets below the softMax limit (if ever).
 *
 * </ul>
 *
 * Setting hard limits too low may create blind spots in the
 * calculation, especially when move() is invoked repeatedly which is
 * often the case in a game. Soft limits on the other hand do not
 * create blind spots that get progressively worse with each move(),
 * in fact temporary blind spots may be explored as the situation
 * changes.
 *
 * There is also a parameter to limit the number of semi-connections
 * on the input side of the OR rule:
 * <strong><code>maxInOrRule</code></strong>.
 *
 * As a questionable heuristic it is possible to discard all
 * connections that result from the AND rule (concatenation) where the
 * common group by which the concatenation is done is one of the
 * edges.  The <strong><code>useEdgePivot</code></strong> parameter
 * controls this behaviour.  It must be said, though, that setting
 * this parameter true, slows the computation down enormously (by a
 * factor 10-20) since it produces so much more subgames. But the gain
 * is also high: the positional (as opposed to tactical) instinct
 * seems to come with it.  It is particularly crucial in the opening
 * phase.
 *
 * The processing order of connections depends on memory addresses of
 * groups (see @ref GroupPair). If soft or hard limits are used, it
 * may cause one connection to be processed before the limit is
 * reached in one case while the same connection might not be
 * processed at all in another. In practice, though, it amounts to
 * little difference since none of the possible processing sequences
 * is better.
 */
class Connector
{
  friend class ConnectorTest;
public:

  /**
   * Manager for for DualBatch limits. It is expected to set proper
   * limits when init() is called.
   */
  class DualBatchLimiter
  {
  public:
    virtual ~DualBatchLimiter() {};
    virtual void init(DualBatch &) = 0;
    virtual std::string to_string() const = 0;
    friend std::ostream &operator <<(std::ostream &os,
                                     const DualBatchLimiter &l)
    {
      os << l.to_string();
      return os;
    }
  };

  /**
   * A very simple limiter.
   */
  class SoftLimiter : public DualBatchLimiter
  {
  public:
    SoftLimiter(unsigned softMaxConn = MAXINT, unsigned hardMaxConn = MAXINT,
                unsigned softMaxSemi = MAXINT, unsigned hardMaxSemi = MAXINT)
      : _softMaxConn(softMaxConn), _hardMaxConn(hardMaxConn),
        _softMaxSemi(softMaxSemi), _hardMaxSemi(hardMaxSemi)
    {
    };

    virtual ~SoftLimiter() {};

    virtual void init(DualBatch &db)
    {
      if(!db.connBatch().empty() && (*db.connBatch().begin()).empty()) {
        db.setConnLimits(1, 1);
        db.setSemiLimits(0, 0);
      } else {
        db.setConnLimits(_softMaxConn, _hardMaxConn);
        db.setSemiLimits(_softMaxSemi, _hardMaxSemi);
      }
    };

    virtual std::string to_string() const
    {
      std::ostringstream os;
      os << "(SoftLimiter ";
      printLimit(_softMaxConn, os);
      os << " ";
      printLimit(_hardMaxConn, os);
      os << " ";
      printLimit(_softMaxSemi, os);
      os << " ";
      printLimit(_hardMaxSemi, os);
      os << ")";
      return os.str();
    }
  private:
    static void printLimit(unsigned l, ostream &os)
    {
      if(l >= MAXINT)
        os << "-";
      else
        os << l;
    }
    unsigned _softMaxConn;
    unsigned _hardMaxConn;
    unsigned _softMaxSemi;
    unsigned _hardMaxSemi;
  };

  typedef std::map<GroupPair, DualBatch> DualBatchMap;

  /**
   * Constructs a connector.
   * FIXME
   * Parameters set here cannot be changed later.
   * The default values are unlimited which makes for
   * an unreasonably slow connector for all but the smallest boards.
   */ 
  Connector(const Poi<DualBatchLimiter> &limiter = 
            Poi<DualBatchLimiter>(new SoftLimiter()),
            unsigned maxInOrRule = MAXINT,
            bool useEdgePivot = false,
            bool includePivotInCarrier = false);

  /**
   * Copies connector <code>c</code>.
   * It recreates every the necessary (shared and mutable) component
   * in order to isolate the connectors from unwanted changes.
   * In short: all those Poi<SubGame> elements are recreated and added
   * to this connector.
   *
   * Tasks are not copied.
   *
   * As a not-so-important limitation of the implementation
   * connectors that have been @ref stopped() cannot be copied.
   */
  Connector(const Connector &c);

  /**
   * Calculation of subgames can take an awfully long time,
   * connector can invoke a <code>task</code> periodically.
   * If threading is out of question, this feature can be
   * used to do background tasks that can be more easily sliced
   * than the calculation performed by connector :-).
   */
  void setTask(SlicedTask *task);

  /**
   * Stops computation immediately and the connector becomes @ref stopped().
   */
  void stop();

  /**
   * If stopped a connector does not do any computation.
   * Once stopped a connector cannot be restarted.
   */
  bool stopped() const;

  /**
   * Returns the limiter.
   */
  const Poi<DualBatchLimiter> &limiter() const;

  /**
   * Limit for the number of semi-connections on the input side of the
   * OR rule.
   */
  unsigned maxInOrRule() const;

  /**
   * Parameter that controls concatenation by edges performed by the
   * AND rule.
   */
  bool useEdgePivot() const;

  /**
   * Parameter that controls whether colored pivot points are included
   * in the carrier by the AND rule.
   */
  bool includePivotInCarrier() const;

  /**
   * Initializes the connector. Sets up an initial set of connections
   * between neighbouring groups then calculates the virtual
   * connections by calling @ref calc() if doCalc is true.
   */
  void init(const Grouping &, bool doCalc = true);

  /**
   * Convenience function that creates a grouping for board and mark.
   */
  void init(const HexBoard &board, HexMark mark, bool doCalc = true);

  /**
   * An initialized connector can be updated incrementally
   * by making a <code>move</code> on its board. This gives a major
   * performance boost.
   *
   * @param doReinitOnEdge if @ref useEdgePivot() is false the
   * connector can be forced to reinitialize (as if with init())
   * itself when the move is made next to an edge of the same mark;
   * while slower to reinitialize, it can worth it, since it results
   * in less subgames speeding up subsequent move() calls.
   */
  void move(const HexMove &move, bool doReinitOnEdge = false,
            bool doCalc = true);

  /**
   * Calculates subgames. Only needed after one or more @ref init() or
   * @ref move() calls where the doCalc parameter was false.
   */
  void calc();

  /**
   * Returns the mark of the winner. It is a shortcut for
   * <code>board().winner()</code> (see @ref HexBoard::winner())
   */
  HexMark winner() const;

  /**
   * Returns the mark of interest of this connector
   (<code>grouping().mark()</code>) if it detected a winning virtual
   * connection. Otherwise it returns @ref winner().
   */
  HexMark connWinner() const;

  /**
   * Returns the mark of interest of this connector
   (<code>grouping().mark()</code>) if it detected a winning virtual
   * _semi_ connection. Otherwise it returns @ref winner().
   */
  HexMark semiWinner() const;

  /**
   * For a position without a @ref winner() but with a @ref connWinner()
   * it returns the winning virtual connection with the smallest carrier.
   */
  Carrier winningConnCarrier() const;

  /**
   * For a position without a @ref winner() but with a @ref semiWinner()
   * it returns the winning virtual _semi_ connection with the smallest
   * carrier.
   */
  Carrier winningSemiCarrier() const;

  /**
   * For a position without a @ref winner() and a @ref connWinner()
   * it returns the intersection of winning virtual semi connections.
   */
  Carrier criticalPath() const;

  /**
   * The grouping for the current @ref board() position.
   */
  const Grouping &grouping() const;

  /**
   * The current board position.
   */
  const HexBoard &board() const;

  /**
   * Connections between groups.
   */
  const DualBatchMap &connections() const;

  void unite(const Connector &);
private:
  /**
   * Not implemeneted. It is here to catch erroneous calls.
   */
  Connector &operator =(const Connector &c);

  typedef deque<GroupPair> GroupPairQueue;
  typedef vector<DualBatch *> Fan;
  typedef std::map<Group *, Fan>  FanMap;

  void enqueue(DualBatch &);
  void addToFanMap(DualBatch &);
  void initFanMap();
  void setLimits(DualBatch &);
  void processConnsForBatch(Group *x, Group *y, Group *middle,
                            const vector<Carrier> &conns,
                            const Batch &b);
  void processConns(DualBatch &db, const vector<Carrier> &conns);
  void processSemi(DualBatch &db, const Batch::Iterator &semi);
  void updateConnections(Group *newGroup, Group *emptyGroup,
                         const vector<Group *> &unitedGroups,
                         const vector<Group *> &deletedGroups);
  bool batchConcatenatable(Group *x0, Group *y0,
                           Group *x1, Group *y1,
                           Group **x, Group **y, Group **middle);
  bool applyAnd(Group *x, Group *y, Group *middle,
                const Carrier &c0, const Carrier &c1,
                DualBatch *&db);
  void applyOr(DualBatch &db,
               vector<Carrier>::const_iterator semiCur,
               vector<Carrier>::const_iterator semiEnd,
               vector<Carrier>::const_iterator tailIntersectionCur,
               const Carrier &un, const Carrier &in,
               unsigned int depth);
  vector<Carrier> semisWithSamePath(const Batch &b, const Batch::Iterator &i);
  void processConnBatch(DualBatch &db);
  void processSemiBatch(DualBatch &db);
  void processDualBatch(DualBatch &db);
  GroupPair winningGroupPair() const;
  void setConnWinner(const GroupPair &k);

  SlicedTask *_task;
  bool _stop;

  Poi<DualBatchLimiter> _limiter;
  unsigned _maxInOrRule;
  bool _useEdgePivot;
  bool _includePivotInCarrier;
  Grouping _groups;
  GroupPairQueue _queue;
  DualBatchMap _map;
  FanMap _fanMap;
  HexMark _winner;
  HexMark _connWinner;
};

#endif
